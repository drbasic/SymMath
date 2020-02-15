#include "UnMinusOperation.h"

#include <cassert>

#include "OpInfo.h"
#include "Constant.h"
#include "INodeHelper.h"

UnMinusOperation::UnMinusOperation(std::unique_ptr<INode> value)
    : Operation(GetOpInfo(Op::UnMinus), std::move(value)) {}

std::unique_ptr<INode> UnMinusOperation::SymCalc() const {
  std::unique_ptr<INode> val = operands_[0]->SymCalc();
  if (Constant* as_const = val->AsConstant()) {
    return INodeHelper::MakeConst(op_info_->trivial_f(as_const->Value(), 0.0));
  }
  return std::make_unique<UnMinusOperation>(std::move(val));
}

std::unique_ptr<INode> UnMinusOperation::Clone() const {
  return std::make_unique<UnMinusOperation>(operands_[0]->Clone());
}

PrintSize UnMinusOperation::Render(Canvas* canvas,
                                   PrintBox print_box,
                                   bool dry_run,
                                   RenderBehaviour render_behaviour) const {
  auto minus_behaviour = render_behaviour.TakeMinus();

  if (minus_behaviour == MinusBehaviour::Force) {
    assert(!HasFrontMinus());
    return operands_[0]->Render(canvas, print_box, dry_run, render_behaviour);
  }

  // - -a => a
  if (operands_[0]->HasFrontMinus()) {
    render_behaviour.SetMunus(MinusBehaviour::Ommit);
    return operands_[0]->Render(canvas, print_box, dry_run, render_behaviour);
  }

  // don't render this minus. a + (-b) -> a - b. Minus render from (+)
  // operation.
  if (minus_behaviour == MinusBehaviour::Ommit) {
    assert(HasFrontMinus());
    assert(!operands_[0]->HasFrontMinus());
    return RenderOperand(operands_[0].get(), canvas, print_box, dry_run,
                         render_behaviour, false);
  }

  if (minus_behaviour == MinusBehaviour::Relax) {
    return RenderOperand(operands_[0].get(), canvas, print_box, dry_run,
                         render_behaviour, true);
  }

  assert(false);
  return {};
}

bool UnMinusOperation::HasFrontMinus() const {
  return !operands_[0]->HasFrontMinus();
}
