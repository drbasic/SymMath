#include "SqrtOperation.h"

#include <algorithm>
#include <cassert>
#include <cmath>

#include "Brackets.h"
#include "Constant.h"
#include "DivOperation.h"
#include "INodeHelper.h"
#include "MultOperation.h"
#include "PowOperation.h"
#include "SimplifyHelpers.h"

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
    if (exp_const->Value() == 2.0 && !exp_const->IsNamed())
      print_exp = false;
  }

  if (print_exp) {
    auto exp_render_behaviour = render_behaviour;
    exp_render_behaviour.SetBrackets(BracketsBehaviour::Ommit);
    exp_render_behaviour.SetMunus(MinusBehaviour::Ommit);
    exp_render_behaviour.SetSubSuper(SubSuperBehaviour::Superscript);
    exp_size = dry_run ? Exp()->Render(canvas, print_box, dry_run,
                                       exp_render_behaviour)
                       : Exp()->LastPrintSize();
    if (!dry_run) {
      PrintBox exp_print_box(exp_size, print_box.x,
                             print_box.base_line - exp_size.base_line - 1);
      auto exp_size2 =
          Exp()->Render(canvas, exp_print_box, dry_run, exp_render_behaviour);
      assert(exp_size == exp_size2);
      print_box = print_box.ShrinkLeft(exp_size.width - 1);
    }
    exp_size.width -= 1;
  }

  auto value_render_behaviour = render_behaviour;
  value_render_behaviour.SetBrackets(BracketsBehaviour::Ommit);
  value_render_behaviour.SetMunus(MinusBehaviour::Ommit);
  auto value_size = dry_run ? Value()->Render(canvas, print_box, dry_run,
                                              value_render_behaviour)
                            : Value()->LastPrintSize();
  value_size.height = std::max(value_size.height, exp_size.height);

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

  CanonicPow result = INodeHelper::GetCanonicPow(operands_[0]);
  for (auto& node_info : result.base_nodes)
    node_info.exp_down *= exp_const->Value();
  return result;
}

void SqrtOperation::SimplifyConsts(HotToken token,
                                   std::unique_ptr<INode>* new_node) {
  Operation::SimplifyConsts({&token}, new_node);
  if (*new_node)
    return;
  SimplifyExp(token, new_node);
}

void SqrtOperation::SimplifyChains(HotToken token,
                                   std::unique_ptr<INode>* new_node) {
  Operation::SimplifyChains({&token}, nullptr);

  if (auto* as_sqrt = INodeHelper::AsSqrt(Value())) {
    SetOperand(
        OperandIndex::ExpIndex,
        INodeHelper::MakeMult(TakeOperand(OperandIndex::ExpIndex),
                              as_sqrt->TakeOperand(OperandIndex::ExpIndex)));
    SetOperand(OperandIndex::ValueIndex,
               as_sqrt->TakeOperand(OperandIndex::ValueIndex));
  }

  if (auto* as_pow = INodeHelper::AsPow(Value())) {
    auto down = ExtractMultipliers(Exp()->Clone().get());
    auto up = ExtractMultipliers(as_pow->Exp()->Clone().get());
    auto eq_nodes = TakeEqualNodes(&up, &down);
    if (!eq_nodes.empty()) {
      INodeHelper::RemoveEmptyOperands(&down);
      INodeHelper::RemoveEmptyOperands(&up);
      if (!up.empty()) {
        as_pow->SetOperand(PowOperation::OperandIndex::PowIndex,
                           INodeHelper::MakeMultIfNeeded(std::move(up)));
      } else {
        SetOperand(OperandIndex::ValueIndex,
                   as_pow->TakeOperand(PowOperation::OperandIndex::BaseIndex));
      }
      if (!down.empty()) {
        SetOperand(OperandIndex::ExpIndex,
                   INodeHelper::MakeMultIfNeeded(std::move(down)));
      } else {
        SetOperand(OperandIndex::ExpIndex, INodeHelper::MakeConst(1.0));
      }
    }
  }
}

void SqrtOperation::SimplifyExp(HotToken& token,
                                std::unique_ptr<INode>* new_node) {
  if (auto* as_const = INodeHelper::AsConstant(Exp())) {
    if (!as_const->IsNamed()) {
      double exp = as_const->Value();
      if (exp == 0.0) {
        token.SetChanged();
        *new_node = INodeHelper::MakeConst(1.0);
        return;
      }
      if (exp == 1.0) {
        token.SetChanged();
        *new_node = TakeOperand(OperandIndex::ValueIndex);
        return;
      }
    }
  }
}
