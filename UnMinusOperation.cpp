#include "UnMinusOperation.h"

#include <cassert>

#include "Constant.h"
#include "INodeHelper.h"
#include "MultOperation.h"
#include "OpInfo.h"
#include "PlusOperation.h"

UnMinusOperation::UnMinusOperation(std::unique_ptr<INode> value)
    : Operation(GetOpInfo(Op::UnMinus), std::move(value)) {}

std::unique_ptr<INode> UnMinusOperation::Clone() const {
  return INodeHelper::MakeUnMinus(operands_[0]->Clone());
}

PrintSize UnMinusOperation::Render(Canvas* canvas,
                                   PrintBox print_box,
                                   bool dry_run,
                                   RenderBehaviour render_behaviour) const {
  auto minus_behaviour = render_behaviour.TakeMinus();

  if (minus_behaviour == MinusBehaviour::Force) {
    assert(!HasFrontMinus());
    return print_size_ =
               Operand()->Render(canvas, print_box, dry_run, render_behaviour);
  }

  // - -a => a
  if (Operand()->HasFrontMinus()) {
    render_behaviour.SetMunus(MinusBehaviour::Ommit);
    return print_size_ =
               Operand()->Render(canvas, print_box, dry_run, render_behaviour);
  }

  // don't render this minus. a + (-b) -> a - b. Minus render from (+)
  // operation.
  if (minus_behaviour == MinusBehaviour::Ommit) {
    assert(HasFrontMinus());
    assert(!Operand()->HasFrontMinus());
    return print_size_ = RenderOperand(Operand(), canvas, print_box, dry_run,
                                       render_behaviour, false);
  }

  if (minus_behaviour == MinusBehaviour::Relax) {
    return print_size_ = RenderOperand(Operand(), canvas, print_box, dry_run,
                                       render_behaviour, true);
  }

  assert(false);
  return {};
}

bool UnMinusOperation::HasFrontMinus() const {
  return !Operand()->HasFrontMinus();
}

ValueType UnMinusOperation::GetValueType() const {
  return Operand()->GetValueType();
}

void UnMinusOperation::OpenBracketsImpl(HotToken token,
                                        std::unique_ptr<INode>* new_node) {
  Operation::OpenBracketsImpl({&token}, nullptr);

  auto* as_plus = INodeHelper::AsPlus(Operand());
  if (!as_plus)
    return;

  token.SetChanged();
  auto new_operands = as_plus->TakeAllOperands();
  for (auto& node : new_operands) {
    node = INodeHelper::Negate(std::move(node));
  }

  auto temp_node = INodeHelper::MakePlus(std::move(new_operands));
  temp_node->OpenBracketsImpl({&token}, new_node);
  if (!*new_node)
    *new_node = std::move(temp_node);
}

std::optional<CanonicMult> UnMinusOperation::GetCanonicMult() {
  CanonicMult result = INodeHelper::GetCanonicMult(operands_[0]);
  result.a *= -1.0;
  return result;
}

void UnMinusOperation::SimplifyUnMinus(HotToken token,
                                       std::unique_ptr<INode>* new_node) {
  Operation::SimplifyUnMinus({&token}, nullptr);

  if (auto sub_un_minus = INodeHelper::AsUnMinus(Operand())) {
    token.SetChanged();
    *new_node = sub_un_minus->TakeOperand();
    return;
  }

  if (auto* sub_mult = INodeHelper::AsMult(Operand())) {
    for (size_t i = 0; i < sub_mult->OperandsCount(); ++i) {
      if (auto* as_const = INodeHelper::AsConstant(sub_mult->Operand(i))) {
        if (!as_const->Name().empty())
          continue;
        sub_mult->SetOperand(i, INodeHelper::MakeConst(-as_const->Value()));
        token.SetChanged();
        *new_node = TakeOperand();
        return;
      }
    }
  }
}