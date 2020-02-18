#include "MultOperation.h"

#include <algorithm>
#include <cassert>

#include "Constant.h"
#include "DivOperation.h"
#include "INodeHelper.h"
#include "Imaginary.h"
#include "OpInfo.h"
#include "PlusOperation.h"
#include "UnMinusOperation.h"

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
  return std::make_unique<MultOperation>(std::move(new_nodes));
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

std::optional<CanonicMult> MultOperation::GetCanonic() {
  CanonicMult result;
  for (auto& op : operands_) {
    Constant* constant = op->AsNodeImpl()->AsConstant();
    if (constant)
      result.a = op_info_->trivial_f(result.a, constant->Value());
    else
      INodeHelper::MergeCanonic(&op, &result);
  }
  return result;
}

void MultOperation::ProcessImaginary(
    std::vector<std::unique_ptr<INode>>* nodes) const {
  bool has_imaginary = false;
  for (auto& node : *nodes) {
    if (!node->AsNodeImpl()->AsImaginary())
      continue;
    if (has_imaginary) {
      node = INodeHelper::MakeConst(-1);
      has_imaginary = false;
    } else {
      node.reset();
      has_imaginary = true;
    }
  }
  INodeHelper::RemoveEmptyOperands(nodes);
  if (has_imaginary)
    nodes->push_back(INodeHelper::MakeImaginary());
}

void MultOperation::UnfoldChains() {
  Operation::UnfoldChains();

  std::vector<std::unique_ptr<INode>> new_nodes;
  INodeHelper::ExctractNodesWithOp(Op::Mult, &operands_, &new_nodes);
  operands_.swap(new_nodes);
  CheckIntegrity();
}

void MultOperation::SimplifyUnMinus(std::unique_ptr<INode>* new_node) {
  Operation::SimplifyUnMinus(nullptr);
  bool is_positve = true;
  for (auto& node : operands_) {
    if (auto* un_minus = INodeHelper::AsUnMinus(node.get())) {
      is_positve = !is_positve;
      node = INodeHelper::Negate(std::move(node));
    }
  }
  if (!is_positve) {
    *new_node =
        INodeHelper::MakeUnMinus(INodeHelper::MakeMult(std::move(operands_)));
  }
}

void MultOperation::SimplifyChains(std::unique_ptr<INode>* new_node) {
  Operation::SimplifyChains(nullptr);
  ProcessImaginary(&operands_);
}

void MultOperation::SimplifyDivMul(std::unique_ptr<INode>* new_node) {
  Operation::SimplifyConsts(new_node);

  std::vector<std::unique_ptr<INode>> new_bottom;
  for (size_t i = 0; i < operands_.size(); ++i) {
    if (auto* div = INodeHelper::AsDiv(operands_[i].get())) {
      operands_.push_back(std::move(div->operands_[0]));
      new_bottom.push_back(std::move(div->operands_[1]));
      operands_[i].reset();
    }
  }
  if (new_bottom.empty())
    return;
  INodeHelper::RemoveEmptyOperands(&operands_);
  *new_node = INodeHelper::MakeDiv(
      INodeHelper::MakeMultIfNeeded(std::move(operands_)),
      INodeHelper::MakeMultIfNeeded(std::move(new_bottom)));
}

void MultOperation::SimplifyConsts(std::unique_ptr<INode>* new_node) {
  Operation::SimplifyConsts(new_node);
  if (*new_node)
    return;

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
  RemoveEmptyOperands();
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

void MultOperation::OpenBrackets(std::unique_ptr<INode>* new_node) {
  Operation::OpenBrackets(nullptr);
  if (!INodeHelper::HasAnyPlusOperation(operands_))
    return;

  std::vector<std::unique_ptr<INode>> ordinal_nodes;
  std::vector<std::vector<std::unique_ptr<INode>>> plus_nodes;
  std::vector<std::pair<size_t, size_t>> permutation_indexes;
  for (auto& node : operands_) {
    if (auto* plus = INodeHelper::AsPlus(node.get())) {
      permutation_indexes.emplace_back(0, plus->operands_.size());
      plus_nodes.push_back(std::move(plus->operands_));
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
  *new_node = INodeHelper::MakePlus(std::move(new_plus_nodes));
}