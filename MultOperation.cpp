#include "MultOperation.h"

#include <algorithm>
#include <cassert>

#include "Constant.h"
#include "INodeHelper.h"
#include "UnMinusOperation.h"
#include "OpInfo.h"

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
    Constant* constant = op->AsConstant();
    if (constant)
      result.a = op_info_->trivial_f(result.a, constant->Value());
    else
      INodeHelper::MergeCanonic(&op, &result);
  }
  return result;
}

void MultOperation::SimplifyChain() {
  UnfoldChain();
  bool is_positve = true;
  size_t i = 0;
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
  Operation::SimplifyChain();
}

void MultOperation::UnfoldChain() {
  std::vector<std::unique_ptr<INode>> positive_nodes;
  std::vector<std::unique_ptr<INode>> negative_nodes;
  for (auto& node : operands_) {
    INodeHelper::ExctractNodesWithOp(Op::Mult, std::move(node), &positive_nodes,
                                     &negative_nodes);
  }
  operands_.swap(positive_nodes);
  operands_.reserve(operands_.size() + negative_nodes.size());
  for (auto& node : negative_nodes) {
    operands_.push_back(INodeHelper::Negate(std::move(node)));
  }
  CheckIntegrity();
}
