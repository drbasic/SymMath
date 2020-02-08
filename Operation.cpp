#include "Operation.h"

#include <algorithm>
#include <cassert>
#include <numeric>
#include <optional>
#include <sstream>

#include "Constant.h"
#include "Exception.h"
#include "OpInfo.h"
#include "Operation.h"
#include "ValueHelpers.h"

namespace {

std::vector<std::unique_ptr<INode>> CalcOperands(
    const std::vector<std::unique_ptr<INode>>& operands) {
  std::vector<std::unique_ptr<INode>> result;
  result.reserve(operands.size());
  for (const auto& operand : operands) {
    result.push_back(operand->SymCalc());
  }
  return result;
}

std::optional<std::vector<double>> AsTrivial(
    const std::vector<std::unique_ptr<INode>>& operands) {
  std::vector<double> result;
  result.reserve(operands.size());
  for (const auto& operand : operands) {
    if (Constant* as_const = dynamic_cast<Constant*>(operand.get())) {
      result.push_back(as_const->Value());
    } else {
      return std::nullopt;
    }
  }
  return result;
}

}  // namespace

Operation::Operation(const OpInfo* op_info, std::unique_ptr<INode> lh)
    : op_info_(op_info) {
  operands_.push_back(std::move(lh));
  assert(operands_[0]);
}

Operation::Operation(const OpInfo* op_info,
                     std::unique_ptr<INode> lh,
                     std::unique_ptr<INode> rh)
    : op_info_(op_info) {
  operands_.reserve(2);
  operands_.push_back(std::move(lh));
  operands_.push_back(std::move(rh));
  assert(operands_[0] && operands_[1]);
}

Operation::Operation(const OpInfo* op_info,
                     std::vector<std::unique_ptr<INode>> operands)
    : op_info_(op_info), operands_(std::move(operands)) {
  CheckIntegrity();
}

std::string Operation::PrintImpl(bool ommit_front_minus) const {
  if (op_info_->print_f)
    return op_info_->print_f(op_info_, operands_);
  if (IsUnMinus())
    return PrintUnMinus(ommit_front_minus);
  return PrintMinusPlusMultDiv();
}

int Operation::Priority() const {
  return op_info_->priority;
}

std::unique_ptr<INode> Operation::SymCalc() const {
  if (op_info_->calc_f)
    return op_info_->calc_f(op_info_, operands_);
  if (IsUnMinus())
    return CalcUnMinus();
  return CalcMinusPlusMultDiv();
}

bool Operation::HasFrontMinus() const {
  if (IsUnMinus()) {
    return !operands_[0]->HasFrontMinus();
  }
  return false;
}

bool Operation::CheckCircular(const INode* other) const {
  for (const auto& operand : operands_) {
    if (operand->CheckCircular(other))
      return true;
  }
  return false;
}

Operation* Operation::AsUnMinus() {
  return op_info_->op == Op::UnMinus ? this : nullptr;
}

std::vector<std::unique_ptr<INode>> Operation::TakeOperands(Op op) {
  if (op_info_->op != op)
    return {};

  return std::move(operands_);
}

bool Operation::SimplifyImpl(std::unique_ptr<INode>* new_node) {
  CheckIntegrity();

  bool simplified = false;
  for (auto& node : operands_) {
    std::unique_ptr<INode> new_sub_node;
    while (node->SimplifyImpl(&new_sub_node)) {
      simplified = true;
      if (new_sub_node)
        node = std::move(new_sub_node);
    }
  }
  if (SimplifyConsts(new_node)) {
    CheckIntegrity();
    return true;
  }
  if (SimplifyUnMinus(new_node)) {
    CheckIntegrity();
    return true;
  }
  while (SimplifyChain()) {
    CheckIntegrity();
    simplified = true;
  }

  return simplified;
}

void Operation::CheckIntegrity() const {
  switch (op_info_->op) {
    case Op::UnMinus:
      assert(operands_.size() == 1);
      break;
    case Op::Minus:
      assert(operands_.size() > 1);
      break;
    case Op::Plus:
      assert(operands_.size() > 1);
      break;
    case Op::Mult:
      assert(operands_.size() > 1);
      break;
    case Op::Div:
      assert(operands_.size() == 2);
      break;
  }
  for (const auto& operand : operands_) {
    assert(operand);
  }
}

bool Operation::IsUnMinus() const {
  return op_info_->op == Op::UnMinus;
}

bool Operation::SimplifyUnMinus(std::unique_ptr<INode>* new_node) {
  if (!AsUnMinus())
    return false;
  Operation* sub_un_minus = operands_[0]->AsUnMinus();
  if (!sub_un_minus)
    return false;

  *new_node = std::move(sub_un_minus->operands_[0]);
  return true;
}

bool Operation::SimplifyChain() {
  ConvertToPlus();

  if (op_info_->op != Op::Plus && op_info_->op != Op::Mult)
    return false;

  bool is_optimized = false;
  std::vector<std::unique_ptr<INode>> new_nodes;

  for (auto& node : operands_) {
    auto sub_nodes = node->TakeOperands(op_info_->op);
    if (sub_nodes.empty()) {
      new_nodes.push_back(std::move(node));
    } else {
      is_optimized = true;
      for (auto& sub_node : sub_nodes)
        new_nodes.push_back(std::move(sub_node));
    }
  }
  operands_.clear();

  if (op_info_->op == Op::Mult) {
    for (size_t i = 1; i < new_nodes.size(); ++i) {
      auto un_minus_sub_node = new_nodes[i]->TakeOperands(Op::UnMinus);
      assert(un_minus_sub_node.empty() || un_minus_sub_node.size() == 1);
      if (!un_minus_sub_node.empty()) {
        is_optimized = true;
        new_nodes.push_back(Const(-1.0));
        new_nodes[i] = std::move(un_minus_sub_node[0]);
      }
    }
  }

  operands_.swap(new_nodes);

  return is_optimized;
}

bool Operation::SimplifyConsts(std::unique_ptr<INode>* new_node) {
  if (op_info_->op != Op::Minus && op_info_->op != Op::Plus &&
      op_info_->op != Op::Mult && op_info_->op != Op::Div)
    return false;

  size_t const_count = 0;
  bool is_optimized = false;
  for (size_t i = 0; i < operands_.size(); ++i) {
    if (!operands_[i])
      continue;
    Constant* constant = operands_[i]->AsConstant();
    if (!constant)
      continue;
    ++const_count;
    // x * 0.0
    if (constant->Value() == 0.0 && op_info_->op == Op::Mult) {
      *new_node = Const(0.0);
      return true;
    }
    // x + 0
    if (constant->Value() == 0.0 && op_info_->op == Op::Plus) {
      is_optimized = true;
      operands_[i].reset();
    }
    // x - 0
    if (constant->Value() == 0.0 && i != 0 && op_info_->op == Op::Minus) {
      is_optimized = true;
      operands_[i].reset();
    }
    // 0 -x
    if (constant->Value() == 0.0 && i == 0 && op_info_->op == Op::Minus) {
      if (i + 1 < operands_.size()) {
      } else {
        *new_node = Const(0.0);
        return true;
      }
    }
    // 0 / x
    if (constant->Value() == 0.0 && i == 0 && op_info_->op == Op::Div) {
      *new_node = Const(0.0);
      return true;
    }
    // x * 1
    if (constant->Value() == 1.0 && (op_info_->op == Op::Mult)) {
      is_optimized = true;
      operands_[i].reset();
    }
    // x / 1
    if (constant->Value() == 1.0 && i == 1 && op_info_->op == Op::Div) {
      *new_node = std::move(operands_[0]);
      return true;
    }
    // x / 0
    if (constant->Value() == 1.0 && i == 1 && op_info_->op == Op::Div) {
      *new_node = Const(std::numeric_limits<double>::infinity());
      return true;
    }
  }
  if (!is_optimized && const_count == 0)
    return false;
  if (const_count == 2 && op_info_->op == Op::Div) {
    *new_node = SymCalc();
    return true;
  }
  const_count = 0;
  double accumulator = op_info_->op == Op::Mult ? 1 : 0;
  for (size_t i = 0; i < operands_.size(); ++i) {
    if (!operands_[i])
      continue;
    Constant* constant = operands_[i]->AsConstant();
    if (!constant)
      continue;
    if (op_info_->op != Op::Div) {
      ++const_count;
      accumulator = op_info_->trivial_f(accumulator, constant->Value());
      operands_[i].reset();
    }
  }
  operands_.erase(
      std::remove_if(std::begin(operands_), std::end(operands_),
                     [](const std::unique_ptr<INode>& node) { return !node; }),
      std::end(operands_));
  if (operands_.empty()) {
    *new_node = Const(accumulator);
    return true;
  }
  if (op_info_->op == Op::Mult && accumulator == -1.0) {
    operands_[0] = std::make_unique<Operation>(GetOpInfo(Op::UnMinus),
                                               std::move(operands_[0]));
    is_optimized = true;
  }

  if (const_count) {
    operands_.reserve(operands_.size() + 1);
    operands_.push_back(Const(accumulator));
    is_optimized = const_count > 1;
  }
  if (operands_.size() == 1) {
    *new_node = std::move(operands_[0]);
    return true;
  }
  return is_optimized;
}

void Operation::ConvertToPlus() {
  if (op_info_->op != Op::Minus)
    return;
  op_info_ = GetOpInfo(Op::Plus);
  for (size_t i = 1; i < operands_.size(); ++i) {
    operands_[i] = std::make_unique<Operation>(GetOpInfo(Op::UnMinus),
                                               std::move(operands_[i]));
  }
}

std::string Operation::PrintUnMinus(bool ommit_front_minus) const {
  return PrintOperand(operands_[0].get(), !ommit_front_minus);
}

std::string Operation::PrintMinusPlusMultDiv() const {
  int i = 0;
  std::stringstream ss;
  // if (operands_.size() > 2)
  //  ss << "[";
  for (const auto& operand : operands_) {
    ss << PrintOperand(operand.get(), i++ != 0);
  }
  // if (operands_.size() > 2)
  //  ss << "]";
  return ss.str();
}

std::string Operation::PrintOperand(const INode* node, bool with_op) const {
  if (IsUnMinus() && node->HasFrontMinus()) {
    return node->PrintImpl(true);
  }

  bool need_br = (node->Priority() < Priority());
  if (!with_op && node->IsUnMinus() && op_info_->op == Op::Mult)
    need_br = false;
  if (need_br || !with_op) {
    std::stringstream ss;
    if (with_op)
      ss << op_info_->name;
    if (need_br)
      ss << "(";
    ss << node->PrintImpl(false);
    if (need_br)
      ss << ")";
    return ss.str();
  }

  if ((op_info_->op == Op::Minus || op_info_->op == Op::Plus) &&
      node->HasFrontMinus()) {
    if (op_info_->op == Op::Minus)
      return "+" + node->PrintImpl(true);
    else
      return node->PrintImpl(false);
  }

  return std::string(op_info_->name) + node->PrintImpl(false);
}

std::unique_ptr<INode> Operation::CalcUnMinus() const {
  std::unique_ptr<INode> val = operands_[0]->SymCalc();
  if (Constant* as_const = val->AsConstant()) {
    return Const(op_info_->trivial_f(as_const->Value(), 0.0));
  }
  return std::make_unique<Operation>(op_info_, std::move(val));
}

std::unique_ptr<INode> Operation::CalcMinusPlusMultDiv() const {
  std::vector<std::unique_ptr<INode>> calculated_operands =
      CalcOperands(operands_);
  auto trivials = AsTrivial(calculated_operands);
  if (!trivials) {
    return std::make_unique<Operation>(op_info_,
                                       std::move(calculated_operands));
  }

  double result =
      std::accumulate(std::begin(*trivials) + 1, std::end(*trivials),
                      (*trivials).front(), op_info_->trivial_f);
  return std::make_unique<Constant>(result);
}
//=============================================================================
