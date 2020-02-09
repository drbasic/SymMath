#include "Operation.h"

#include <algorithm>
#include <cassert>
#include <map>
#include <numeric>
#include <optional>
#include <sstream>

#include "Constant.h"
#include "Exception.h"
#include "OpInfo.h"
#include "Operation.h"
#include "ValueHelpers.h"

class INodeAcessor {
 public:
  static bool IsNodesEqual(const INode* lh, const INode* rh) {
    return lh->IsEqual(rh);
  }

  static const Constant* AsConstant(const INode* lh) {
    return lh->AsConstant();
  }

  static const Operation* AsOperation(const INode* lh) {
    return lh->AsOperation();
  }
  static Operation* AsOperation(INode* lh) { return lh->AsOperation(); }

  static std::vector<std::unique_ptr<INode>>& GetOperands(Operation* op) {
    return op->operands_;
  }
};

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

bool IsNodesTransitiveEqual(const std::vector<const INode*>& lhs,
                            const std::vector<const INode*>& rhs) {
  if (lhs.size() != rhs.size())
    return false;

  std::vector<bool> used;
  used.resize(rhs.size());
  for (size_t i = 0; i < lhs.size(); ++i) {
    bool equal_found = false;
    for (size_t j = 0; j < rhs.size(); ++j) {
      if (used[j])
        continue;
      if (!lhs[i]->IsEqual(rhs[i]))
        continue;
      used[j] = true;
      equal_found = true;
      break;
    }
    if (!equal_found)
      return false;
  }
  return true;
}

bool IsNodesTransitiveEqual(const std::vector<std::unique_ptr<INode>*>& lhs,
                            const std::vector<std::unique_ptr<INode>*>& rhs) {
  auto transform = [](const std::vector<std::unique_ptr<INode>*>& nodes)
      -> std::vector<const INode*> {
    std::vector<const INode*> result;
    result.resize(nodes.size());
    for (size_t i = 0; i < nodes.size(); ++i) {
      result[i] = nodes[i]->get();
    }
    return result;
  };
  return IsNodesTransitiveEqual(transform(lhs), transform(rhs));
}

bool IsNodesTransitiveEqual(const std::vector<std::unique_ptr<INode>>& lhs,
                            const std::vector<std::unique_ptr<INode>>& rhs) {
  auto transform = [](const std::vector<std::unique_ptr<INode>>& nodes)
      -> std::vector<const INode*> {
    std::vector<const INode*> result;
    result.resize(nodes.size());
    for (size_t i = 0; i < nodes.size(); ++i) {
      result[i] = nodes[i].get();
    }
    return result;
  };
  return IsNodesTransitiveEqual(transform(lhs), transform(rhs));
}

std::vector<std::unique_ptr<INode>*> GetNodesPointers(
    std::vector<std::unique_ptr<INode>>& nodes,
    size_t skip) {
  std::vector<std::unique_ptr<INode>*> result;
  for (size_t i = 0; i < nodes.size(); ++i) {
    if (i == skip)
      continue;
    if (nodes[i].get())
      result.push_back(&nodes[i]);
  }
  return result;
}

std::vector<std::unique_ptr<INode>*> GetNodesPointersWithoutConstant(
    const std::vector<std::unique_ptr<INode>*>& src,
    double* constant) {
  std::vector<std::unique_ptr<INode>*> result;
  for (auto node : src) {
    if (auto c = INodeAcessor::AsConstant(node->get()))
      *constant += c->Value();
    else
      result.push_back(node);
  }
  return result;
}

bool TakeTransitiveEqualNodesTheSame(
    const std::vector<std::unique_ptr<INode>*>& lhs,
    const std::vector<std::unique_ptr<INode>*>& rhs) {
  std::vector<bool> used;
  used.resize(rhs.size());
  for (size_t i = 0; i < lhs.size(); ++i) {
    bool equal_found = false;
    for (size_t j = 0; j < rhs.size(); ++j) {
      if (used[j])
        continue;
      if (!lhs[i]->get()->IsEqual(rhs[i]->get()))
        continue;
      used[j] = true;
      equal_found = true;
      break;
    }
    if (!equal_found)
      return false;
  }
  for (size_t i = 0; i < used.size(); ++i) {
    if (used[i])
      rhs[i]->reset();
  }
  return true;
}
std::unique_ptr<INode> MakeMult(double dividend,
                                double divider,
                                std::vector<std::unique_ptr<INode>*> nodes) {
  if (dividend == divider && nodes.size() == 1) {
    return std::move(*nodes[0]);
  }
  if (dividend == divider) {
    std::vector<std::unique_ptr<INode>> operands;
    for (auto& node : nodes) {
      operands.push_back(std::move(*node));
    }
    return std::make_unique<Operation>(GetOpInfo(Op::Mult),
                                       std::move(operands));
  }

  std::unique_ptr<INode> result;
  if (dividend != 1.0 || nodes.size() > 1) {
    std::vector<std::unique_ptr<INode>> operands;
    operands.reserve(nodes.size() + (dividend != 1.0 ? 1 : 0));
    if (dividend != 1.0)
      operands.push_back(Const(dividend));
    for (auto& node : nodes) {
      operands.push_back(std::move(*node));
    }
    result =
        std::make_unique<Operation>(GetOpInfo(Op::Mult), std::move(operands));
  } else {
    result = std::move(*nodes[0]);
  }
  if (divider == 1.0)
    return result;
  result = std::make_unique<Operation>(GetOpInfo(Op::Div), std::move(result),
                                       Const(divider));
  return result;
}

double TakeTransitiveEqualNodesCount(
    const std::vector<std::unique_ptr<INode>*>& lhs,
    const std::vector<std::unique_ptr<INode>*>& rhs) {
  std::vector<double> used;
  used.resize(rhs.size());
  for (size_t i = 0; i < lhs.size(); ++i) {
    bool equal_found = false;
    ConanicMultDiv canonic_lh = Operation::GetConanic(lhs[i]);

    for (size_t j = 0; j < rhs.size(); ++j) {
      if (used[j])
        continue;
      ConanicMultDiv canonic_rh = Operation::GetConanic(rhs[j]);
      bool eq = IsNodesTransitiveEqual(canonic_lh.nodes, canonic_rh.nodes);
      if (!eq)
        continue;
      used[j] = (canonic_rh.a * canonic_lh.b) / (canonic_lh.a * canonic_rh.b);
      equal_found = true;
      break;
    }
    if (!equal_found)
      return 0;
  }

  std::map<double, size_t> popular;
  for (size_t i = 0; i < used.size(); ++i) {
    if (used[i] != 0.0)
      popular[used[i]]++;
  }

  auto it =
      std::max_element(std::begin(popular), std::end(popular),
                       [](auto lh, auto rh) { return lh.second < rh.second; });
  double counter = it->first;
  for (size_t i = 0; i < used.size(); ++i) {
    if (used[i] == 0.0)
      continue;
    if (used[i] == counter) {
      rhs[i]->reset();
    } else {
      ConanicMultDiv canonic_rh = Operation::GetConanic(rhs[i]);
      double dividend = (canonic_rh.a - counter) * canonic_rh.b;
      double divider = canonic_rh.a;
      *rhs[i] = MakeMult(dividend, divider, {rhs[i]});
    }
  }
  return counter;
}

double TakeTransitiveEqualNodes(const std::vector<std::unique_ptr<INode>*>& lhs,
                                const std::vector<std::unique_ptr<INode>*>& rhs,
                                double* remains) {
  if (TakeTransitiveEqualNodesTheSame(lhs, rhs)) {
    remains = 0;
    return 1;
  }
  std::vector<std::unique_ptr<INode>*> without_consts =
      GetNodesPointersWithoutConstant(lhs, remains);
  double count = TakeTransitiveEqualNodesCount(without_consts, rhs);
  *remains *= -count;
  return count;
}

double TryExctractSum(const ConanicMultDiv& canonic,
                      std::vector<std::unique_ptr<INode>*> free_operands,
                      double* remains) {
  if (canonic.nodes.size() != 1)
    return 0.0;
  Operation* canonic_op = INodeAcessor::AsOperation(canonic.nodes[0]->get());
  if (!canonic_op)
    return 0.0;

  if (INodeAcessor::GetOperands(canonic_op).size() > free_operands.size())
    return 0.0;

  double count = TakeTransitiveEqualNodes(
      GetNodesPointers(INodeAcessor::GetOperands(canonic_op),
                       std::numeric_limits<size_t>::max()),
      free_operands, remains);
  return count;
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

std::unique_ptr<INode> Operation::Clone() const {
  std::vector<std::unique_ptr<INode>> new_nodes;
  new_nodes.reserve(operands_.size());
  for (const auto& op : operands_)
    new_nodes.push_back(op->Clone());
  return std::make_unique<Operation>(op_info_, std::move(new_nodes));
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

bool Operation::IsEqual(const INode* rh) const {
  const Operation* rh_op = rh->AsOperation();
  if (!rh_op)
    return false;
  if (op_info_ != rh_op->op_info_)
    return false;
  if (operands_.size() != rh_op->operands_.size())
    return false;
  if (!op_info_->is_transitive) {
    for (size_t i = 0; i < operands_.size(); ++i) {
      if (!operands_[i]->IsEqual(rh_op->operands_[i].get()))
        return false;
    }
    return true;
  }

  return IsNodesTransitiveEqual(operands_, rh_op->operands_);
}

Operation* Operation::AsUnMinus() {
  return op_info_->op == Op::UnMinus ? this : nullptr;
}

Operation* Operation::AsOperation() {
  return this;
}

const Operation* Operation::AsOperation() const {
  return this;
}

std::vector<std::unique_ptr<INode>> Operation::TakeOperands(Op op) {
  if (op_info_->op != op)
    return {};

  return std::move(operands_);
}

bool Operation::Combine(Op op,
                        const INode* node1,
                        const INode* node2,
                        std::unique_ptr<INode>* new_node1,
                        std::unique_ptr<INode>* new_node2) const {
  return false;
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
      CheckIntegrity();
    }
  }
  if (SimplifyUnMinus(new_node)) {
    if (new_node->get())
      return true;
    simplified = true;
  }
  while (SimplifyChain()) {
    CheckIntegrity();
    simplified = true;
  }

  while (SimplifySame(new_node)) {
    if (new_node->get())
      return true;
    CheckIntegrity();
    simplified = true;
  }

  if (SimplifyConsts(new_node)) {
    if (new_node->get())
      return true;
    CheckIntegrity();
    simplified = true;
  }
  return simplified;
}

void Operation::CheckIntegrity() const {
  assert(op_info_);
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
  if (op_info_->op != Op::Plus && op_info_->op != Op::Mult &&
      op_info_->op != Op::Minus)
    return false;

  if (NeedConvertToChain()) {
    ConvertToPlus();
  }

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

bool Operation::SimplifySame(std::unique_ptr<INode>* new_node) {
  if (op_info_->op != Op::Plus && op_info_->op != Op::Minus)
    return false;
  bool is_optimized = false;
  for (size_t i = 0; i < operands_.size(); ++i) {
    if (!operands_[i])
      continue;
    ConanicMultDiv conanic_1 = GetConanic(&operands_[i]);
    if (conanic_1.nodes.empty())
      continue;
    bool is_operand_optimized = false;
    for (size_t j = i + 1; j < operands_.size(); ++j) {
      if (!operands_[j])
        continue;
      ConanicMultDiv conanic_2 = GetConanic(&operands_[j]);
      if (conanic_2.nodes.empty())
        continue;
      if (!IsNodesTransitiveEqual(conanic_1.nodes, conanic_2.nodes))
        continue;
      double dividend = op_info_->trivial_f(conanic_1.a * conanic_2.b,
                                            conanic_2.a * conanic_1.b);
      double divider = (conanic_1.b * conanic_2.b);
      operands_[i] = MakeMult(dividend, divider, conanic_1.nodes);
      operands_[j].reset();
      is_optimized = true;
      is_operand_optimized = true;
      break;
    }
    if (is_operand_optimized)
      continue;
    double remains = 0;
    double count =
        TryExctractSum(conanic_1, GetNodesPointers(operands_, i), &remains);
    if (count != 0.0) {
      double k =
          op_info_->trivial_f(conanic_1.a, count * conanic_1.b) / conanic_1.b;
      std::vector<std::unique_ptr<INode>> operands;
      operands.push_back(Const(k));
      for (auto& node : conanic_1.nodes) {
        operands.push_back(std::move(*node));
      }
      operands_[i] =
          std::make_unique<Operation>(GetOpInfo(Op::Mult), std::move(operands));
      if (remains)
        operands_.push_back(Const(remains));
      is_optimized = true;
    }
  }
  if (is_optimized)
    RemoveEmptyOperands();
  if (operands_.size() == 1) {
    *new_node = std::move(operands_[0]);
  }
  return is_optimized;
}

bool Operation::IsAllOperandsConst() const {
  for (const auto& operand : operands_) {
    Constant* constant = operand->AsConstant();
    if (!constant)
      return false;
  }
  return true;
}

bool Operation::SimplifyConsts(std::unique_ptr<INode>* new_node) {
  if (IsAllOperandsConst()) {
    *new_node = SymCalc();
    return true;
  }
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
  RemoveEmptyOperands();
  if (operands_.empty()) {
    *new_node = Const(accumulator);
    return true;
  }
  if (op_info_->op == Op::Mult && accumulator == -1.0) {
    operands_[0] = std::make_unique<Operation>(GetOpInfo(Op::UnMinus),
                                               std::move(operands_[0]));
    const_count = 0;
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

bool Operation::NeedConvertToChain() const {
  if (op_info_->op == Op::Minus)
    return true;

  if (op_info_->op != Op::Plus)
    return false;

  for (const auto& operand : operands_) {
    if (operand->AsConstant())
      return true;
    const Operation* as_operation = operand->AsOperation();
    if (as_operation && (as_operation->op_info_->op == Op::Minus ||
                         as_operation->op_info_->op == Op::Plus))
      return true;
  }
  return false;
}

void Operation::ConvertToPlus() {
  assert(op_info_->op == Op::Minus || op_info_->op == Op::Plus);

  std::vector<std::unique_ptr<INode>> add_nodes;
  std::vector<std::unique_ptr<INode>> sub_nodes;

  ConvertToPlus(&add_nodes, &sub_nodes);
  operands_.swap(add_nodes);
  operands_.reserve(operands_.size() + sub_nodes.size());
  for (auto& sub_node : sub_nodes) {
    operands_.push_back(std::make_unique<Operation>(GetOpInfo(Op::UnMinus),
                                                    std::move(sub_node)));
  }
  op_info_ = GetOpInfo(Op::Plus);
  CheckIntegrity();
}

void Operation::ConvertToPlus(std::vector<std::unique_ptr<INode>>* add_nodes,
                              std::vector<std::unique_ptr<INode>>* sub_nodes) {
  assert(op_info_->op == Op::Minus || op_info_->op == Op::Plus);

  size_t i = 0;
  for (auto& operand : operands_) {
    bool revert_nodes = (op_info_->op == Op::Minus) && (i++ != 0);

    Operation* as_operation = operand->AsOperation();
    if (as_operation && (as_operation->op_info_->op == Op::Minus ||
                         as_operation->op_info_->op == Op::Plus)) {
      as_operation->ConvertToPlus(revert_nodes ? sub_nodes : add_nodes,
                                  revert_nodes ? add_nodes : sub_nodes);
      operand.reset();
    } else {
      (revert_nodes ? sub_nodes : add_nodes)->push_back(std::move(operand));
    }
  }
}

ConanicMultDiv Operation::GetConanic(std::unique_ptr<INode>* node) {
  ConanicMultDiv result{};
  if (!node || !node->get())
    return result;

  Operation* oper = node->get()->AsOperation();
  if (!oper ||
      (oper->op_info_->op != Op::Mult && oper->op_info_->op != Op::Div &&
       oper->op_info_->op != Op::UnMinus)) {
    result.nodes.push_back(node);
    return result;
  }

  if (oper->op_info_->op == Op::Mult) {
    return oper->GetConanicMult();
  }
  if (oper->op_info_->op == Op::Div) {
    return oper->GetConanicDiv();
  }
  if (oper->op_info_->op == Op::UnMinus) {
    return oper->GetConanicUnMinus();
  }
  assert(false);
  return result;
}

ConanicMultDiv Operation::GetConanicMult() {
  assert(op_info_->op == Op::Mult);
  ConanicMultDiv result;
  for (auto& op : operands_) {
    Constant* constant = op->AsConstant();
    if (constant)
      result.a = op_info_->trivial_f(result.a, constant->Value());
    else
      result.nodes.push_back(&op);
  }
  return result;
}

ConanicMultDiv Operation::GetConanicDiv() {
  assert(op_info_->op == Op::Div);
  ConanicMultDiv result;
  Constant* rh = operands_[1]->AsConstant();
  if (rh) {
    result.b = rh->Value();
    result.nodes.push_back(&operands_[0]);
  }
  return result;
}

ConanicMultDiv Operation::GetConanicUnMinus() {
  assert(op_info_->op == Op::UnMinus);
  ConanicMultDiv result = GetConanic(&operands_[0]);
  result.a *= -1;
  return result;
}

void Operation::RemoveEmptyOperands() {
  operands_.erase(
      std::remove_if(std::begin(operands_), std::end(operands_),
                     [](const std::unique_ptr<INode>& node) { return !node; }),
      std::end(operands_));
}

std::string Operation::PrintUnMinus(bool ommit_front_minus) const {
  return PrintOperand(operands_[0].get(), !ommit_front_minus);
}

std::string Operation::PrintMinusPlusMultDiv() const {
  int i = 0;
  std::stringstream ss;
  // if (operands_.size() > 2)
  ss << "[";
  for (const auto& operand : operands_) {
    ss << PrintOperand(operand.get(), i++ != 0);
  }
  // if (operands_.size() > 2)
  ss << "]";
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
