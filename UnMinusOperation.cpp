#include "UnMinusOperation.h"

#include <cassert>

#include "Constant.h"
#include "INodeHelper.h"
#include "OpInfo.h"

UnMinusOperation::UnMinusOperation(std::unique_ptr<INode> value)
    : Operation(GetOpInfo(Op::UnMinus), std::move(value)) {}

std::unique_ptr<INode> UnMinusOperation::Clone() const {
  return std::make_unique<UnMinusOperation>(operands_[0]->Clone());
}

std::unique_ptr<INode> UnMinusOperation::SymCalc() const {
  std::unique_ptr<INode> val = operands_[0]->SymCalc();
  if (Constant* as_const = val->AsNodeImpl()->AsConstant()) {
    return INodeHelper::MakeConst(op_info_->trivial_f(as_const->Value(), 0.0));
  }
  return std::make_unique<UnMinusOperation>(std::move(val));
}

PrintSize UnMinusOperation::Render(Canvas* canvas,
                                   PrintBox print_box,
                                   bool dry_run,
                                   RenderBehaviour render_behaviour) const {
  auto minus_behaviour = render_behaviour.TakeMinus();

  if (minus_behaviour == MinusBehaviour::Force) {
    assert(!HasFrontMinus());
    return Operand(0)->Render(canvas, print_box, dry_run, render_behaviour);
  }

  // - -a => a
  if (Operand(0)->HasFrontMinus()) {
    render_behaviour.SetMunus(MinusBehaviour::Ommit);
    return Operand(0)->Render(canvas, print_box, dry_run, render_behaviour);
  }

  // don't render this minus. a + (-b) -> a - b. Minus render from (+)
  // operation.
  if (minus_behaviour == MinusBehaviour::Ommit) {
    assert(HasFrontMinus());
    assert(!Operand(0)->HasFrontMinus());
    return RenderOperand(Operand(0), canvas, print_box, dry_run,
                         render_behaviour, false);
  }

  if (minus_behaviour == MinusBehaviour::Relax) {
    return RenderOperand(Operand(0), canvas, print_box, dry_run,
                         render_behaviour, true);
  }

  assert(false);
  return {};
}

bool UnMinusOperation::HasFrontMinus() const {
  return !Operand(0)->HasFrontMinus();
}

std::optional<CanonicMult> UnMinusOperation::GetCanonic() {
  CanonicMult result;
  result.a = -1.0;
  result.nodes.push_back(&operands_[0]);
  return result;
}
