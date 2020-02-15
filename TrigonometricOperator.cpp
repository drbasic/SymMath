#include "TrigonometricOperator.h"

#include <cassert>

#include "OpInfo.h"

TrigonometricOperator::TrigonometricOperator(const OpInfo* op_info,
                                             std::unique_ptr<INode> lh)
    : Operation(op_info, std::move(lh)) {
  assert(op_info->op == Op::Sin || op_info->op == Op::Cos);
}

std::unique_ptr<INode> TrigonometricOperator::Clone() const {
  return std::make_unique<TrigonometricOperator>(op_info_,
                                                 operands_[0]->Clone());
}

PrintSize TrigonometricOperator::Render(
    Canvas* canvas,
    PrintBox print_box,
    bool dry_run,
    RenderBehaviour render_behaviour) const {
  render_behaviour.SetBrackets(BracketsBehaviour::Force);
  return RenderOperand(operands_[0].get(), canvas, print_box, dry_run,
                       render_behaviour, true);
}
