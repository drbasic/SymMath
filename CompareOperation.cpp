#include "CompareOperation.h"

#include <cassert>

#include "Constant.h"
#include "INodeHelper.h"

std::unique_ptr<INode> CompareEqual(
    const OpInfo* op,
    std::vector<std::unique_ptr<INode>>* operands) {
  assert(op->op == Op::Equal);
  assert(operands->size() == 2);
  for (auto& operand : *operands) {
    std::unique_ptr<INode> new_sub_node;
    operand->AsNodeImpl()->OpenBracketsImpl(&new_sub_node);
    if (new_sub_node)
      operand = std::move(new_sub_node);
    operand->AsNodeImpl()->SimplifyImpl(&new_sub_node);
    if (new_sub_node)
      operand = std::move(new_sub_node);
  }
  bool is_equal = (*operands)[0]->IsEqual((*operands)[1].get());
  return INodeHelper::MakeConst(is_equal);
}

CompareOperation::CompareOperation(Op op,
                                   std::unique_ptr<INode> lh,
                                   std::unique_ptr<INode> rh)
    : Operation(GetOpInfo(op), std::move(lh), std::move(rh)) {}

std::unique_ptr<INode> CompareOperation::Clone() const {
  return INodeHelper::MakeCompare(op(), Operand(0)->Clone(),
                                  Operand(1)->Clone());
}

PrintSize CompareOperation::Render(Canvas* canvas,
                                   PrintBox print_box,
                                   bool dry_run,
                                   RenderBehaviour render_behaviour) const {
  return print_size_ =
             RenderOperandChain(canvas, print_box, dry_run, render_behaviour);
}
