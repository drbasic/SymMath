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

  static Operation* AsMult(INode* lh) {
    auto result = lh->AsOperation();
    if (result && result->op_info_->op == Op::Mult) {
      return result;
    }
    return nullptr;
  }

  static std::vector<std::unique_ptr<INode>>& GetOperands(Operation* op) {
    return op->operands_;
  }

  static bool IsUnMinus(const INode* lh) {
    if (auto op = AsOperation(lh))
      return op->IsUnMinus();
    return false;
  }

  static Operation* AsUnMinus(INode* lh) {
    if (auto op = AsOperation(lh))
      return op->IsUnMinus() ? op : nullptr;
    return nullptr;
  }

  static const Operation* AsDiv(const INode* lh) {
    if (auto op = AsOperation(lh))
      return op->op_info_->op == Op::Div ? op : nullptr;
    return nullptr;
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
      if (!INodeAcessor::IsNodesEqual(lhs[i], rhs[i]))
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
      if (!INodeAcessor::IsNodesEqual(lhs[i]->get(), rhs[i]->get()))
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
    CanonicMultDiv canonic_lh = Operation::GetCanonic(lhs[i]);

    for (size_t j = 0; j < rhs.size(); ++j) {
      if (used[j])
        continue;
      CanonicMultDiv canonic_rh = Operation::GetCanonic(rhs[j]);
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
      CanonicMultDiv canonic_rh = Operation::GetCanonic(rhs[i]);
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

double TryExctractSum(const CanonicMultDiv& canonic,
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

PrintSize Operation::Render(Canvas* canvas,
                            const Position& pos,
                            bool dry_run,
                            MinusBehavior minus_behavior) const {
  // if (op_info_->print_f)
  //  return op_info_->print_f(op_info_, operands_);
  switch (op_info_->op) {
    case Op::UnMinus:
      return print_size_ = RenderUnMinus(canvas, pos, dry_run, minus_behavior);
      break;
    case Op::Minus:
    case Op::Plus:
    case Op::Mult:
      return print_size_ =
                 RenderMinusPlus(canvas, pos, dry_run, minus_behavior);
      break;
    case Op::Div:
      return print_size_ = RenderDiv(canvas, pos, dry_run, minus_behavior);
      break;
  }
  assert(false);
  return {};
}

PrintSize Operation::LastPrintSize() const {
  return print_size_;
}

bool Operation::HasFrontMinus() const {
  if (IsUnMinus()) {
    return !operands_[0]->HasFrontMinus();
  }
  if (const auto* div = INodeAcessor::AsDiv(this)) {
    bool lh_minus = div->operands_[0]->HasFrontMinus();
    bool rh_minus = div->operands_[1]->HasFrontMinus();
    return lh_minus ^ rh_minus;
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
  if (!IsUnMinus())
    return false;
  Operation* sub_un_minus = INodeAcessor::AsUnMinus(operands_[0].get());
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
    auto* operation = node->AsOperation();
    if (!operation) {
      new_nodes.push_back(std::move(node));
      continue;
    }
    auto sub_nodes = operation->TakeOperands(op_info_->op);
    if (sub_nodes.empty()) {
      new_nodes.push_back(std::move(node));
    } else {
      is_optimized = true;
      new_nodes.reserve(new_nodes.size() + sub_nodes.size());
      for (auto& sub_node : sub_nodes)
        new_nodes.push_back(std::move(sub_node));
    }
  }
  operands_.clear();

  if (op_info_->op == Op::Mult) {
    for (size_t i = 1; i < new_nodes.size(); ++i) {
      auto* un_minus = INodeAcessor::AsUnMinus(new_nodes[i].get());
      if (!un_minus)
        continue;
      auto un_minus_sub_node = un_minus->TakeOperands(Op::UnMinus);
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
    CanonicMultDiv conanic_1 = GetCanonic(&operands_[i]);
    if (conanic_1.nodes.empty())
      continue;
    bool is_operand_optimized = false;
    for (size_t j = i + 1; j < operands_.size(); ++j) {
      if (!operands_[j])
        continue;
      CanonicMultDiv conanic_2 = GetCanonic(&operands_[j]);
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
    // x * 34 / 17
    if (i == 1 && op_info_->op == Op::Div) {
      if (auto* mult = INodeAcessor::AsMult(operands_[0].get())) {
        if (mult->ReduceFor(constant->Value())) {
          *new_node = std::move(operands_[0]);
          return true;
        }
      }
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
    if (op_info_->op == Op::Mult)
      operands_.insert(operands_.begin(), Const(accumulator));
    else
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

CanonicMultDiv Operation::GetCanonic(std::unique_ptr<INode>* node) {
  CanonicMultDiv result{};
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
    return oper->GetCanonicMult();
  }
  if (oper->op_info_->op == Op::Div) {
    return oper->GetCanonicDiv();
  }
  if (oper->op_info_->op == Op::UnMinus) {
    return oper->GetCanonicUnMinus();
  }
  assert(false);
  return result;
}

CanonicMultDiv Operation::GetCanonicMult() {
  assert(op_info_->op == Op::Mult);
  CanonicMultDiv result;
  for (auto& op : operands_) {
    Constant* constant = op->AsConstant();
    if (constant)
      result.a = op_info_->trivial_f(result.a, constant->Value());
    else
      result.nodes.push_back(&op);
  }
  return result;
}

CanonicMultDiv Operation::GetCanonicDiv() {
  assert(op_info_->op == Op::Div);
  CanonicMultDiv result;
  Constant* rh = operands_[1]->AsConstant();
  if (rh) {
    result.b = rh->Value();
    result.nodes.push_back(&operands_[0]);
  }
  return result;
}

CanonicMultDiv Operation::GetCanonicUnMinus() {
  assert(op_info_->op == Op::UnMinus);
  CanonicMultDiv result = GetCanonic(&operands_[0]);
  result.a *= -1;
  return result;
}

bool Operation::ReduceFor(double val) {
  assert(op_info_->op == Op::Mult);
  assert(val != 0.0);

  for (size_t i = 0; i < operands_.size(); ++i) {
    Constant* constant = operands_[i]->AsConstant();
    if (!constant)
      continue;
    double d = constant->Value() / val;
    double intpart;
    if (modf(d, &intpart) != 0.0)
      continue;
    operands_[i] = Const(d);
    return true;
  }
  return false;
}

void Operation::RemoveEmptyOperands() {
  operands_.erase(
      std::remove_if(std::begin(operands_), std::end(operands_),
                     [](const std::unique_ptr<INode>& node) { return !node; }),
      std::end(operands_));
}

PrintSize Operation::RenderUnMinus(Canvas* canvas,
                                   const Position& pos,
                                   bool dry_run,
                                   MinusBehavior minus_behavior) const {
  if (minus_behavior == MinusBehavior::Force) {
    assert(!HasFrontMinus());
    return operands_[0]->Render(canvas, pos, dry_run, MinusBehavior::Force);
  }

  // - -a => a
  if (operands_[0]->HasFrontMinus()) {
    return operands_[0]->Render(canvas, pos, dry_run, MinusBehavior::Ommit);
  }

  // don't render this minus. a + (-b) -> a - b. Minus render from (+)
  // operation.
  if (minus_behavior == MinusBehavior::Ommit) {
    assert(HasFrontMinus());
    assert(!operands_[0]->HasFrontMinus());
    return RenderOperand(operands_[0].get(), canvas, pos, dry_run, false);
  }

  if (minus_behavior == MinusBehavior::Relax) {
    return RenderOperand(operands_[0].get(), canvas, pos, dry_run, true);
  }

  assert(false);
  return {};
}

PrintSize Operation::RenderMinusPlus(Canvas* canvas,
                                     const Position& pos,
                                     bool dry_run,
                                     MinusBehavior minus_behavior) const {
  PrintSize new_print_size = {};
  Position operand_pos{pos};
  for (size_t i = 0; i < operands_.size(); ++i) {
    operand_pos.y =
        dry_run
            ? (pos.y)
            : (pos.y +
               (print_size_.height - operands_[i]->LastPrintSize().height) / 2);
    auto op_size =
        RenderOperand(operands_[i].get(), canvas, operand_pos, dry_run, i != 0);
    operand_pos.x += op_size.width;
    new_print_size.width += op_size.width;
    new_print_size.height = std::max(new_print_size.height, op_size.height);
  }
  if (!dry_run) {
    assert(print_size_ == new_print_size);
  }
  return print_size_ = new_print_size;
}

PrintSize Operation::RenderDiv(Canvas* canvas,
                               const Position& pos,
                               bool dry_run,
                               MinusBehavior minus_behavior) const {
  assert(op_info_->op == Op::Div);
  bool has_front_minus =
      (minus_behavior == MinusBehavior::Force) ||
      (minus_behavior == MinusBehavior::Relax && HasFrontMinus());

  auto lh_size =
      dry_run ? operands_[0]->Render(canvas, pos, dry_run, MinusBehavior::Ommit)
              : operands_[0]->LastPrintSize();
  auto rh_size =
      dry_run ? operands_[1]->Render(canvas, pos, dry_run, MinusBehavior::Ommit)
              : operands_[1]->LastPrintSize();
  PrintSize operands_size{std::max(lh_size.width, rh_size.width),
                          1 + lh_size.height + rh_size.height};

  auto* un_minus_op_info = GetOpInfo(Op::UnMinus);
  size_t left_offset = has_front_minus ? 1 + un_minus_op_info->name.size() : 0;
  Position divider_pos = {pos.x + left_offset, pos.y + lh_size.height};
  if (has_front_minus) {
    if (!dry_run) {
      canvas->PrintAt({pos.x, divider_pos.y}, std::string(un_minus_op_info->name));
    }
  }

  if (!dry_run) {
    Position lh_pos = {
        divider_pos.x + (operands_size.width - lh_size.width) / 2, pos.y};
    operands_[0]->Render(canvas, lh_pos, dry_run, MinusBehavior::Ommit);

    canvas->PrintAt(divider_pos, std::string(operands_size.width, '~'));

    Position rh_pos = {
        divider_pos.x + (operands_size.width - rh_size.width) / 2,
        pos.y + lh_size.height + 1};
    operands_[1]->Render(canvas, rh_pos, dry_run, MinusBehavior::Ommit);
  }

  return {operands_size.width + left_offset, operands_size.height};
}

PrintSize Operation::RenderOperand(const INode* node,
                                   Canvas* canvas,
                                   const Position& pos,
                                   bool dry_run,
                                   bool with_op) const {
  bool need_br = (node->Priority() < Priority());
  if (!with_op && INodeAcessor::IsUnMinus(node) && op_info_->op == Op::Mult)
    need_br = false;
  if (need_br || !with_op) {
    PrintSize result;
    if (with_op) {
      // + - *
      if (!dry_run) {
        canvas->PrintAt(pos, std::string(op_info_->name));
      }
      result.width += op_info_->name.size();
    }

    // xyz
    Position operand_position{pos};
    operand_position.x += result.width;
    size_t left_br_x = operand_position.x;
    operand_position.x += need_br ? 1 : 0;
    auto operand_size =
        node->Render(canvas, operand_position, dry_run, MinusBehavior::Relax);
    result.width += operand_size.width;
    result.height = std::max(result.height, operand_size.height);

    if (need_br) {
      // { [ ( ) ] }
      if (!dry_run) {
        canvas->RenderBracket({left_br_x, pos.y}, Canvas::Bracket::Left,
                              operand_size.height);
        canvas->RenderBracket({left_br_x + operand_size.width + 1, pos.y},
                              Canvas::Bracket::Right, operand_size.height);
      }
      result.width += 2;
    }
    return result;
  }

  PrintSize operand_size;
  auto op_to_print = op_info_;
  if (with_op && op_info_->op == Op::Plus && node->HasFrontMinus()) {
    // +-1 -> -1 // minus (-) print here, so ommit (-) in operand
    op_to_print = GetOpInfo(Op::Minus);
    operand_size =
        node->Render(canvas, {pos.x + op_to_print->name.size(), pos.y}, dry_run,
                     MinusBehavior::Ommit);
  } else if (with_op && op_info_->op == Op::Minus && node->HasFrontMinus()) {
    // --1 -> +1 // minus operation and front minus(--) -> print plus(+) here,
    // so
    // ommit (-) in operand
    op_to_print = GetOpInfo(Op::Plus);
    operand_size =
        node->Render(canvas, {pos.x + op_to_print->name.size(), pos.y}, dry_run,
                     MinusBehavior::Ommit);
  } else {
    // operand print minus if necessary by self
    operand_size =
        node->Render(canvas, {pos.x + op_to_print->name.size(), pos.y}, dry_run,
                     MinusBehavior::Relax);
  }
  if (!dry_run) {
    canvas->PrintAt({pos.x, pos.y + operand_size.height / 2},
                    std::string(op_to_print->name));
  }
  return {op_info_->name.size() + operand_size.width, operand_size.height};
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
