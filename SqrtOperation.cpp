#include "SqrtOperation.h"

#include <algorithm>
#include <cassert>
#include <cmath>

#include "Brackets.h"
#include "Constant.h"
#include "INodeHelper.h"

double TrivialSqrt(double lh, double rh) {
  if (rh == 2.0)
    return std::sqrt(lh);
  if (rh == 3.0)
    return std::cbrt(lh);
  return std::pow(lh, 1.0 / rh);
}

std::unique_ptr<INode> NonTrivialSqrt(
    const OpInfo* op,
    std::vector<std::unique_ptr<INode>>* operands) {
  return std::unique_ptr<INode>();
}

SqrtOperation::SqrtOperation(std::unique_ptr<INode> value,
                             std::unique_ptr<INode> exp)
    : Operation(GetOpInfo(Op::Sqrt), std::move(value), std::move(exp)) {}

std::unique_ptr<INode> SqrtOperation::Clone() const {
  return INodeHelper::MakeSqrt(Value()->Clone(), Exp()->Clone());
}

PrintSize SqrtOperation::Render(Canvas* canvas,
                                PrintBox print_box,
                                bool dry_run,
                                RenderBehaviour render_behaviour) const {
  PrintSize exp_size;
  bool print_exp = true;
  if (auto* exp_const = Exp()->AsConstant()) {
    if (exp_const->Value() == 2 && !exp_const->IsNamed())
      print_exp = false;
  }

  if (print_exp) {
    auto exp_render_behaviour = render_behaviour;
    exp_render_behaviour.SetBrackets(BracketsBehaviour::Ommit);
    exp_render_behaviour.SetMunus(MinusBehaviour::Ommit);
    exp_size = dry_run ? Exp()->Render(canvas, print_box, dry_run,
                                       exp_render_behaviour)
                       : Exp()->LastPrintSize();
    if (!dry_run) {
      auto exp_print_box = print_box;
      exp_print_box.base_line =
          print_box.y + print_box.height - 2 - exp_size.base_line;
      auto exp_size2 =
          Exp()->Render(canvas, exp_print_box, dry_run, exp_render_behaviour);
      assert(exp_size == exp_size2);
      print_box = print_box.ShrinkLeft(exp_size.width - 1);
    }
  }

  auto value_render_behaviour = render_behaviour;
  value_render_behaviour.SetBrackets(BracketsBehaviour::Ommit);
  value_render_behaviour.SetMunus(MinusBehaviour::Ommit);
  auto value_size = dry_run ? Value()->Render(canvas, print_box, dry_run,
                                              value_render_behaviour)
                            : Value()->LastPrintSize();
  value_size.height = std::max(value_size.height, exp_size.height + 1);

  PrintBox value_print_box;
  auto sqrt_size = canvas->RenderBrackets(
      print_box, BracketType::Sqrt, value_size, dry_run, &value_print_box);

  if (!dry_run) {
    Value()->Render(canvas, value_print_box, dry_run, value_render_behaviour);
  }

  return print_size_ = exp_size.GrowWidth(sqrt_size, true);
}

std::optional<CanonicPow> SqrtOperation::GetCanonicPow() {
  auto* exp_const = Exp()->AsConstant();
  if (!exp_const || exp_const->IsNamed())
    return std::nullopt;

  CanonicPow result;
  result.Add(1.0 / exp_const->Value(), &operands_[ValueIndex]);
  return result;
}

void SqrtOperation::SimplifyChains(HotToken token,
                                   std::unique_ptr<INode>* new_node) {
  Operation::SimplifyChains({&token}, nullptr);
}

void SqrtOperation::SimplifyExp(std::unique_ptr<INode>* new_node) {}
