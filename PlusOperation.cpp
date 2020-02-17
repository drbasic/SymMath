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

void PlusOperation::SimplifyChain(std::unique_ptr<INode>* new_node) {
  UnfoldChain();
  Operation::SimplifyChain(new_node);
}

void PlusOperation::UnfoldChain() {
  std::vector<std::unique_ptr<INode>> new_nodes;
  INodeHelper::ExctractNodesWithOp(Op::Plus, &operands_, &new_nodes);
  operands_.swap(new_nodes);
  CheckIntegrity();
}
