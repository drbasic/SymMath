#include "TrigonometricOperation.h"

#include <cassert>

#include "Constant.h"
#include "OpInfo.h"
#include "ValueHelpers.h"

TrigonometricOperation::TrigonometricOperation(const OpInfo* op_info,
                                               std::unique_ptr<INode> lh)
    : Operation(op_info, std::move(lh)) {
  assert(op_info->op == Op::Sin || op_info->op == Op::Cos);
}

std::unique_ptr<INode> TrigonometricOperation::Clone() const {
  return std::make_unique<TrigonometricOperation>(op_info_,
                                                  operands_[0]->Clone());
}

PrintSize TrigonometricOperation::Render(
    Canvas* canvas,
    PrintBox print_box,
    bool dry_run,
    RenderBehaviour render_behaviour) const {
  render_behaviour.SetBrackets(BracketsBehaviour::Force);
  return print_size_ = RenderOperand(Operand(0), canvas, print_box, dry_run,
                                     render_behaviour, true);
}

void TrigonometricOperation::ConvertToComplexImpl(
    std::unique_ptr<INode>* new_node) {
  Operation::ConvertToComplexImpl(nullptr);

  auto x = Operand(0);
  if (op_info_->op == Op::Sin) {
    *new_node = (Pow(Constants::MakeE(), Imag() * x->Clone()) -
                 Pow(Constants::MakeE(), -Imag() * x->Clone())) /
                (2 * Imag());
    return;
  }
  if (op_info_->op == Op::Cos) {
    *new_node = (Pow(Constants::MakeE(), Imag() * x->Clone()->Clone()) +
                 Pow(Constants::MakeE(), -Imag() * x->Clone())) /
                2;
    return;
  }
}
