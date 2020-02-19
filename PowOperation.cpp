#include "PowOperation.h"

#include <algorithm>
#include <cassert>

#include "Constant.h"
#include "INodeHelper.h"
#include "MultOperation.h"
#include "OpInfo.h"

PowOperation::PowOperation(std::unique_ptr<INode> lh, std::unique_ptr<INode> rh)
    : Operation(GetOpInfo(Op::Pow), std::move(lh), std::move(rh)) {}

std::unique_ptr<INode> PowOperation::Clone() const {
  return std::make_unique<PowOperation>(operands_[0]->Clone(),
                                        operands_[1]->Clone());
}

PrintSize PowOperation::Render(Canvas* canvas,
                               PrintBox print_box,
                               bool dry_run,
                               RenderBehaviour render_behaviour) const {
  auto base_render_behaviour = render_behaviour;
  auto exp_render_behaviour = render_behaviour;
  if (Base()->Priority() < Priority())
    base_render_behaviour.SetBrackets(BracketsBehaviour::Force);
  base_print_size_ = dry_run
                         ? RenderOperand(Base(), canvas, PrintBox::Infinite(),
                                         dry_run, base_render_behaviour, false)
                         : base_print_size_;

  pow_print_size_ = dry_run
                        ? RenderOperand(Exp(), canvas, PrintBox::Infinite(),
                                        dry_run, exp_render_behaviour, false)
                        : pow_print_size_;

  if (!dry_run) {
    {
      // Render base
      PrintBox base_box = print_box.ShrinkTop(pow_print_size_.height);
      base_box.base_line = base_box.y + base_print_size_.base_line;
      auto base_size2 = RenderOperand(Base(), canvas, base_box, dry_run,
                                      base_render_behaviour, false);
      assert(base_size2 == base_print_size_);
    }
    {
      // Render exp
      PrintBox exp_box = print_box.ShrinkLeft(base_print_size_.width);
      exp_box.height = pow_print_size_.height;
      exp_box.base_line = exp_box.y + pow_print_size_.base_line;
      auto exp_size2 = RenderOperand(Exp(), canvas, exp_box, dry_run,
                                     exp_render_behaviour, false);
      assert(exp_size2 == pow_print_size_);
    }
  }
  auto total_size = pow_print_size_.GrowDown(base_print_size_, true);
  total_size.width = pow_print_size_.width + base_print_size_.width;
  return print_size_ = total_size;
}

std::optional<CanonicPow> PowOperation::GetCanonicPow() {
  auto* exp_const = INodeHelper::AsConstant(operands_[1].get());
  if (!exp_const)
    return std::nullopt;
  CanonicPow result = INodeHelper::GetCanonicPow(operands_[0]);
  result.exp *= exp_const->Value();
  return result;
}

INodeImpl* PowOperation::Base() {
  return Operand(0);
}

const INodeImpl* PowOperation::Base() const {
  return Operand(0);
}

INodeImpl* PowOperation::Exp() {
  return Operand(1);
}

const INodeImpl* PowOperation::Exp() const {
  return Operand(1);
}
