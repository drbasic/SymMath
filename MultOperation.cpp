#include "MultOperation.h"

#include <algorithm>
#include <cassert>

#include "Constant.h"
#include "DivOperation.h"
#include "INodeHelper.h"
#include "Imaginary.h"
#include "OpInfo.h"
#include "PlusOperation.h"
#include "SimplifyHelpers.h"
#include "UnMinusOperation.h"
#include "Vector.h"
#include "VectorScalarProduct.h"

namespace {

bool NextPermutation(
    std::vector<std::pair<size_t, size_t>>* permutation_indexes) {
  for (auto ii = std::rbegin(*permutation_indexes);
       ii != std::rend(*permutation_indexes); ++ii) {
    ++ii->first;
    if (ii->first >= ii->second)
      ii->first = 0;
    else
      return true;
  }
  return false;
}

}  // namespace

MultOperation::MultOperation(std::unique_ptr<INode> lh,
                             std::unique_ptr<INode> rh)
    : Operation(GetOpInfo(Op::Mult), std::move(lh), std::move(rh)) {}

MultOperation::MultOperation(std::vector<std::unique_ptr<INode>> operands)
    : Operation(GetOpInfo(Op::Mult), std::move(operands)) {}

std::unique_ptr<INode> MultOperation::Clone() const {
  std::vector<std::unique_ptr<INode>> new_nodes;
  new_nodes.reserve(operands_.size());
  for (const auto& op : operands_)
    new_nodes.push_back(op->Clone());
  return INodeHelper::MakeMult(std::move(new_nodes));
}

PrintSize MultOperation::Render(Canvas* canvas,
                                PrintBox print_box,
                                bool dry_run,
                                RenderBehaviour render_behaviour) const {
  return print_size_ =
             RenderOperandChain(canvas, print_box, dry_run, render_behaviour);
}

bool MultOperation::HasFrontMinus() const {
  return operands_[0]->AsNodeImpl()->HasFrontMinus();
}

ValueType MultOperation::GetValueType() const {
  if (OperandsCount() < 1)
    return ValueType::Scalar;
  ValueType result = operands_[0]->AsNodeImpl()->GetValueType();
  for (size_t i = 1; i < OperandsCount(); ++i) {
    result =
        GetMultResultType(result, operands_[i]->AsNodeImpl()->GetValueType());
  }
  return result;
}

void MultOperation::OpenBracketsImpl(HotToken token,
                                     std::unique_ptr<INode>* new_node) {
  Operation::OpenBracketsImpl({&token}, nullptr);
  std::unique_ptr<INode> temp_node;
  SimplifyUnMinus({&token}, &temp_node);
  if (temp_node) {
    temp_node->AsNodeImpl()->OpenBracketsImpl({&token}, new_node);
    if (!*new_node)
      *new_node = std::move(temp_node);
    return;
  }

  SimplifyDivMul({&token}, &temp_node);
  if (temp_node) {
    temp_node->AsNodeImpl()->OpenBracketsImpl({&token}, new_node);
    if (!*new_node)
      *new_node = std::move(temp_node);
    return;
  }

  OpenPlusBrackets(token, new_node);
}

std::optional<CanonicMult> MultOperation::GetCanonicMult() {
  CanonicMult result;
  for (auto& op : operands_) {
    Constant* constant = op->AsNodeImpl()->AsConstant();
    if (constant)
      result.a = op_info_->trivial_f(result.a, constant->Value());
    else
      result.nodes.push_back(&op);
  }
  return result;
}

std::optional<CanonicPow> MultOperation::GetCanonicPow() {
  CanonicPow result;
  result.base_nodes.reserve(operands_.size());
  for (auto& node : operands_) {
    result.Merge(INodeHelper::GetCanonicPow(node));
  }
  return result;
}

// static
std::unique_ptr<INode> MultOperation::ProcessImaginary(
    std::vector<std::unique_ptr<INode>>* nodes) {
  size_t count_i = 0;
  for (auto& node : *nodes) {
    if (!node->AsNodeImpl()->AsImaginary())
      continue;
    ++count_i;
    if (count_i == 2)
      break;
  }
  if (count_i < 2)
    return nullptr;

  count_i = 0;
  for (auto& node : *nodes) {
    if (!node->AsNodeImpl()->AsImaginary())
      continue;
    ++count_i;
    node.reset();
  }

  INodeHelper::RemoveEmptyOperands(nodes);
  if (count_i >= 4) {
    count_i = count_i % 4;
  }
  bool negate = false;
  if (count_i >= 2) {
    count_i -= 2;
    negate = true;
  }
  assert(count_i == 0 || count_i == 1);
  std::unique_ptr<INode> node;
  if (count_i)
    node = INodeHelper::MakeImaginary();
  else
    node = INodeHelper::MakeConst(1.0);
  if (negate)
    node = INodeHelper::Negate(std::move(node));
  if (!nodes->empty()) {
    nodes->push_back(std::move(node));
  }
  return node;
}

void MultOperation::UnfoldChains(HotToken token) {
  Operation::UnfoldChains({&token});
  auto params_change_counter = token.CountParamsChanged(this);

  std::vector<std::unique_ptr<INode>> new_nodes;
  ExctractNodesWithOp(Op::Mult, &operands_, &new_nodes);
  operands_.swap(new_nodes);
}

void MultOperation::SimplifyUnMinus(HotToken token,
                                    std::unique_ptr<INode>* new_node) {
  Operation::SimplifyUnMinus({&token}, nullptr);

  auto params_change_counter = token.CountParamsChanged(this);

  bool is_positive = true;
  for (auto& node : operands_) {
    if (auto* un_minus = INodeHelper::AsUnMinus(node.get())) {
      is_positive = !is_positive;
      node = INodeHelper::Negate(std::move(node));
    }
  }
  if (!is_positive) {
    *new_node =
        INodeHelper::MakeUnMinus(INodeHelper::MakeMult(std::move(operands_)));
    return;
  }
}

void MultOperation::SimplifyChains(HotToken token,
                                   std::unique_ptr<INode>* new_node) {
  Operation::SimplifyChains({&token}, nullptr);
  auto params_change_counter = token.CountParamsChanged(this);

  auto i_node = ProcessImaginary(&operands_);
  if (i_node) {
    *new_node = std::move(i_node);
    return;
  }
}

void MultOperation::SimplifyDivMul(HotToken token,
                                   std::unique_ptr<INode>* new_node) {
  Operation::SimplifyDivMul({&token}, nullptr);
  auto params_change_counter = token.CountParamsChanged(this);

  std::vector<std::unique_ptr<INode>> new_bottom;
  for (auto& node : operands_) {
    if (auto* div = INodeHelper::AsDiv(node.get())) {
      new_bottom.push_back(div->TakeOperand(1));
      node = div->TakeOperand(0);
    }
  }
  if (new_bottom.empty())
    return;
  *new_node = INodeHelper::MakeDiv(
      INodeHelper::MakeMultIfNeeded(std::move(operands_)),
      INodeHelper::MakeMultIfNeeded(std::move(new_bottom)));
}

void MultOperation::SimplifyConsts(HotToken token,
                                   std::unique_ptr<INode>* new_node) {
  Operation::SimplifyConsts({&token}, new_node);
  if (*new_node)
    return;
  auto params_change_counter = token.CountParamsChanged(this);

  size_t const_count = 0;
  double mult_total = 0;
  for (auto& node : operands_) {
    Constant* constant = INodeHelper::AsConstant(node.get());
    if (!constant)
      continue;
    // x * 0.0
    if (constant->Value() == 0.0) {
      *new_node = INodeHelper::MakeConst(0.0);
      return;
    }
    // x * 1
    if (constant->Value() == 1.0) {
      node.reset();
      continue;
    }
    ++const_count;
    if (const_count == 1) {
      mult_total = constant->Value();
    } else {
      mult_total = op_info_->trivial_f(mult_total, constant->Value());
    }
    node.reset();
  }
  INodeHelper::RemoveEmptyOperands(&operands_);
  if (operands_.empty()) {
    *new_node = INodeHelper::MakeConst(mult_total);
    return;
  }

  if (const_count && mult_total == -1.0)
    operands_[0] = INodeHelper::MakeUnMinus(std::move(operands_[0]));
  else if (const_count && mult_total != 1.0)
    operands_.insert(operands_.begin(), INodeHelper::MakeConst(mult_total));

  if (operands_.size() == 1) {
    *new_node = std::move(operands_[0]);
    return;
  }
  CheckIntegrity();
}

void MultOperation::SimplifyTheSame(HotToken token,
                                    std::unique_ptr<INode>* new_node) {
  Operation::SimplifyTheSame({&token}, nullptr);

  SimplifyTheSamePow(token, new_node);
  if (*new_node)
    return;
  SimplifyTheSameMult(token, new_node);
}

void MultOperation::OrderOperands(HotToken token) {
  Operation::OrderOperands({&token});

  ReorderOperands(&operands_, true);
}

void MultOperation::OpenPlusBrackets(HotToken& token,
                                     std::unique_ptr<INode>* new_node) {
  if (!INodeHelper::HasAnyOperation(Op::Plus, operands_))
    return;
  auto params_change_counter = token.CountParamsChanged(this);

  std::vector<std::unique_ptr<INode>> ordinal_nodes;
  std::vector<std::vector<std::unique_ptr<INode>>> plus_nodes;
  std::vector<std::pair<size_t, size_t>> permutation_indexes;
  for (auto& node : operands_) {
    if (auto* plus = INodeHelper::AsPlus(node.get())) {
      permutation_indexes.emplace_back(0, plus->OperandsCount());
      plus_nodes.push_back(plus->TakeAllOperands());
    } else {
      ordinal_nodes.push_back(std::move(node));
    }
  }

  std::vector<std::unique_ptr<INode>> new_plus_nodes;
  do {
    std::vector<std::unique_ptr<INode>> mult_nodes;
    for (const auto& node : ordinal_nodes) {
      mult_nodes.push_back(node->Clone());
    }
    for (size_t i = 0; i < plus_nodes.size(); ++i) {
      mult_nodes.push_back(
          plus_nodes[i][permutation_indexes[i].first]->Clone());
    }
    auto mult = INodeHelper::MakeMult(std::move(mult_nodes));
    new_plus_nodes.push_back(std::move(mult));
  } while (NextPermutation(&permutation_indexes));

  auto temp_node = INodeHelper::MakePlus(std::move(new_plus_nodes));
  temp_node->OpenBracketsImpl({&token}, new_node);
  if (!*new_node)
    *new_node = std::move(temp_node);
}

void MultOperation::SimplifyTheSameMult(HotToken& token,
                                        std::unique_ptr<INode>* new_node) {
  auto params_change_counter = token.CountParamsChanged(this);

  bool need_try = true;
  while (need_try) {
    need_try = false;
    for (size_t i = 0; i < operands_.size(); ++i) {
      if (!operands_[i])
        continue;
      CanonicMult canonic_1 = INodeHelper::GetCanonicMult(operands_[i]);
      if (canonic_1.nodes.empty()) {
        // skip constants.
        continue;
      }

      for (size_t j = i + 1; j < operands_.size(); ++j) {
        if (!operands_[j])
          continue;
        CanonicMult canonic_2 = INodeHelper::GetCanonicMult(operands_[j]);
        if (canonic_2.nodes.empty())
          continue;

        bool is_combined = MergeCanonicToMult(token, canonic_1, canonic_2,
                                              &operands_[i], &operands_[j]);
        if (!operands_[i])
          break;
        if (is_combined) {
          need_try = true;
          canonic_1 = INodeHelper::GetCanonicMult(operands_[i]);
        }
      }
    }
    INodeHelper::RemoveEmptyOperands(&operands_);
    SimplifyConsts({&token}, new_node);
    if (*new_node)
      return;
  }
  if (operands_.size() == 1) {
    *new_node = std::move(operands_[0]);
  }
  if (operands_.size() == 0) {
    *new_node = INodeHelper::MakeConst(0.0);
  }
}

void MultOperation::SimplifyTheSamePow(HotToken& token,
                                       std::unique_ptr<INode>* new_node) {
  auto params_change_counter = token.CountParamsChanged(this);

  for (size_t i = 0; i < operands_.size(); ++i) {
    if (!operands_[i])
      continue;
    CanonicPow canonic_1 = INodeHelper::GetCanonicPow(operands_[i]);
    if (canonic_1.base_nodes.empty()) {
      // skip constants.
      continue;
    }

    for (size_t j = i + 1; j < operands_.size(); ++j) {
      if (!operands_[j])
        continue;
      CanonicPow canonic_2 = INodeHelper::GetCanonicPow(operands_[j]);
      if (canonic_2.base_nodes.empty())
        continue;

      std::vector<std::unique_ptr<INode>> new_sub_nodes;
      bool is_combined = MergeCanonicToPow(
          token, canonic_1, std::move(canonic_2), &new_sub_nodes, nullptr);
      if (is_combined) {
        operands_[i].reset();
        operands_[j].reset();
        for (size_t k = 0; k < new_sub_nodes.size(); ++k) {
          std::unique_ptr<INode> new_sub_node;
          new_sub_nodes[k]->AsNodeImpl()->SimplifyImpl({&token}, &new_sub_node);
          if (!new_sub_node)
            new_sub_node = std::move(new_sub_nodes[k]);
          if (k == 0)
            operands_[i] = std::move(new_sub_node);
          else if (k == 1)
            operands_[j] = std::move(new_sub_node);
          else
            operands_.push_back(std::move(new_sub_node));
        }
        canonic_1 = INodeHelper::GetCanonicPow(operands_[i]);
      }
    }
  }
  INodeHelper::RemoveEmptyOperands(&operands_);
  if (operands_.size() == 1) {
    *new_node = std::move(operands_[0]);
    return;
  }
  if (operands_.size() == 0) {
    *new_node = INodeHelper::MakeConst(0.0);
    return;
  }
}