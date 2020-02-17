#include "MultOperation.h"

#include <algorithm>
#include <cassert>

#include "Constant.h"
#include "INodeHelper.h"
#include "Imaginary.h"
#include "OpInfo.h"
#include "UnMinusOperation.h"

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
  return false;
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

void MultOperation::SimplifyChain(std::unique_ptr<INode>* new_node) {
  UnfoldChain();
  bool is_positve = true;
  size_t i = 0;
  ProcessImaginary(&operands_);
  for (auto& node : operands_) {
    if (i++ == 0)
      continue;
    if (auto* un_minus = INodeHelper::AsUnMinus(node.get())) {
      is_positve = !is_positve;
      node = INodeHelper::Negate(std::move(node));
    }
  }
  if (!is_positve) {
    operands_[0] = INodeHelper::Negate(std::move(operands_[0]));
  }
  Operation::SimplifyChain(new_node);
}

void MultOperation::UnfoldChain() {
  std::vector<std::unique_ptr<INode>> new_nodes;
  INodeHelper::ExctractNodesWithOp(Op::Mult, &operands_, &new_nodes);
  operands_.swap(new_nodes);
  CheckIntegrity();
}
