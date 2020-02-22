#include "VectorMultOperation.h"

#include "INodeHelper.h"

VectorMultOperation::VectorMultOperation(std::unique_ptr<INode> lh,
                                         std::unique_ptr<INode> rh)
    : Operation(GetOpInfo(Op::VectorMult), std::move(lh), std::move(rh)) {}

std::unique_ptr<INode> VectorMultOperation::Clone() const {
  return INodeHelper::MakeVectorMult(Operand(0)->Clone(), Operand(1)->Clone());
}

PrintSize VectorMultOperation::Render(Canvas* canvas,
                                      PrintBox print_box,
                                      bool dry_run,
                                      RenderBehaviour render_behaviour) const {
  return print_size_ =
             RenderOperandChain(canvas, print_box, dry_run, render_behaviour);
}
