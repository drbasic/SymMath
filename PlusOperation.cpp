#include "PlusOperation.h"

#include <algorithm>
#include <cassert>

#include "INodeHelper.h"
#include "OpInfo.h"
#include "UnMinusOperation.h"

PlusOperation::PlusOperation(std::unique_ptr<INode> lh,
                             std::unique_ptr<INode> rh)
    : Operation(GetOpInfo(Op::Plus), std::move(lh), std::move(rh)) {}

PlusOperation::PlusOperation(std::vector<std::unique_ptr<INode>> operands)
    : Operation(GetOpInfo(Op::Plus), std::move(operands)) {}

std::unique_ptr<INode> PlusOperation::Clone() const {
  std::vector<std::unique_ptr<INode>> new_nodes;
  new_nodes.reserve(operands_.size());
  for (const auto& op : operands_)
    new_nodes.push_back(op->Clone());
  return std::make_unique<PlusOperation>(std::move(new_nodes));
}

PrintSize PlusOperation::Render(Canvas* canvas,
                                PrintBox print_box,
                                bool dry_run,
                                RenderBehaviour render_behaviour) const {
  return print_size_ =
             RenderOperandChain(canvas, print_box, dry_run, render_behaviour);
}

bool PlusOperation::HasFrontMinus() const {
  return false;
}

void PlusOperation::SimplifyChain() {
  UnfoldChain();
  Operation::SimplifyChain();
}

void PlusOperation::UnfoldChain() {
  std::vector<std::unique_ptr<INode>> add_nodes;
  std::vector<std::unique_ptr<INode>> sub_nodes;
  for (auto& node : operands_) {
    INodeHelper::ExctractNodesWithOp(Op::Plus, std::move(node), &add_nodes,
                                     &sub_nodes);
  }
  operands_.swap(add_nodes);
  operands_.reserve(operands_.size() + sub_nodes.size());
  for (auto& node : sub_nodes) {
    operands_.push_back(INodeHelper::Negate(std::move(node)));
  }
  CheckIntegrity();
}
