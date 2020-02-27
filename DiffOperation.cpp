#include "DiffOperation.h"

#include <cassert>

#include "Brackets.h"
#include "Constant.h"
#include "DivOperation.h"
#include "INodeHelper.h"
#include "LogOperation.h"
#include "MultOperation.h"
#include "PlusOperation.h"
#include "UnMinusOperation.h"
#include "ValueHelpers.h"
#include "Variable.h"
#include "VariableRef.h"

namespace {
std::unique_ptr<INode> DoDiffNode(const INode* node, const Variable& by_var);
std::unique_ptr<INode> DoDiffVariable(const Variable* var,
                                      const Variable& by_var);
std::unique_ptr<INode> DoDiffOperation(const Operation* operation,
                                       const Variable& by_var);
std::unique_ptr<INode> DoDiffMultOperation(const Operation* operation,
                                           const Variable& by_var);
std::unique_ptr<INode> DoDiffDivOperation(const Operation* operation,
                                          const Variable& by_var);
std::unique_ptr<INode> DoDiffPlusOperation(const Operation* operation,
                                           const Variable& by_var);
std::unique_ptr<INode> DoDiffPowOperation(const Operation* operation,
                                          const Variable& by_var);
std::unique_ptr<INode> DoDiffLogOperation(const Operation* operation,
                                          const Variable& by_var);

std::unique_ptr<INode> DoDiffVariable(const Variable* var,
                                      const Variable& by_var) {
  return INodeHelper::MakeConst(var->GetName() == by_var.GetName() ? 1.0 : 0.0);
}

std::unique_ptr<INode> DoDiffOperation(const Operation* operation,
                                       const Variable& by_var) {
  switch (operation->op()) {
    case Op::UnMinus: {
      return INodeHelper::MakeUnMinus(
          DoDiffNode(operation->Operand(0), by_var));
    } break;
    case Op::Minus: {
      assert(false);
    } break;
    case Op::Plus: {
      return DoDiffPlusOperation(operation, by_var);
    } break;
    case Op::Mult: {
      return DoDiffMultOperation(operation, by_var);
    } break;
    case Op::Pow: {
      return DoDiffPowOperation(operation, by_var);
    } break;
    case Op::VectorMult: {
      assert(false);
    } break;
    case Op::Div: {
      return DoDiffDivOperation(operation, by_var);
    } break;
    case Op::Sin: {
      auto f = operation->Operand(0);
      return DoDiffNode(f, by_var) * Cos(f->Clone());
    }
    case Op::Cos: {
      auto f = operation->Operand(0);
      return -DoDiffNode(f, by_var) * Sin(f->Clone());
    } break;
    case Op::Log: {
      return DoDiffLogOperation(operation, by_var);
    } break;
    case Op::Equal: {
      assert(false);
    } break;
    case Op::Diff: {
      assert(false);
    } break;
    case Op::Sqrt: {
      assert(false);
    } break;
  }

  return nullptr;
}

std::unique_ptr<INode> DoDiffMultOperation(const Operation* operation,
                                           const Variable& by_var) {
  std::vector<std::unique_ptr<INode>> new_plus_operands;
  new_plus_operands.reserve(operation->OperandsCount());

  for (size_t i = 0; i < operation->OperandsCount(); ++i) {
    std::vector<std::unique_ptr<INode>> new_mult_operands;
    new_mult_operands.reserve(operation->OperandsCount());
    for (size_t j = 0; j < operation->OperandsCount(); ++j) {
      if (i == j) {
        auto diff = DoDiffNode(operation->Operand(j), by_var);
        if (auto* as_const = INodeHelper::AsConstant(diff.get())) {
          if (as_const->Value() == 0.0) {
            new_mult_operands.clear();
            break;
          }
        }
        new_mult_operands.push_back(std::move(diff));
      } else {
        new_mult_operands.push_back(operation->Operand(j)->Clone());
      }
    }
    if (new_mult_operands.empty())
      continue;
    new_plus_operands.push_back(
        INodeHelper::MakeMultIfNeeded(std::move(new_mult_operands)));
  }
  return INodeHelper::MakePlusIfNeeded(std::move(new_plus_operands));
}

std::unique_ptr<INode> DoDiffDivOperation(const Operation* operation,
                                          const Variable& by_var) {
  auto f = operation->Operand(0);
  auto g = operation->Operand(1);

  return (INodeHelper::MakeMultIfNeeded(DoDiffNode(f, by_var), g->Clone()) -
          INodeHelper::MakeMultIfNeeded(f->Clone(), DoDiffNode(g, by_var))) /
         Pow(g->Clone(), 2);
}

std::unique_ptr<INode> DoDiffPlusOperation(const Operation* operation,
                                           const Variable& by_var) {
  std::vector<std::unique_ptr<INode>> new_plus_operands;
  new_plus_operands.reserve(operation->OperandsCount());
  for (size_t i = 0; i < operation->OperandsCount(); ++i) {
    new_plus_operands.push_back(DoDiffNode(operation->Operand(i), by_var));
  }
  return INodeHelper::MakePlusIfNeeded(std::move(new_plus_operands));
}

std::unique_ptr<INode> DoDiffPowOperation(const Operation* operation,
                                          const Variable& by_var) {
  auto f = operation->Operand(0);
  auto g = operation->Operand(1);
  auto derivative_f = DoDiffNode(f, by_var);
  auto derivative_g = DoDiffNode(g, by_var);
  if (derivative_f->IsEqual(Constants::Zero()) &&
      derivative_g->IsEqual(Constants::Zero())) {
    return Const(0.0);
  }

  if (derivative_f->IsEqual(Constants::Zero())) {
    // 10^x = 10^x * log(10);
    return std::move(derivative_g) * operation->Clone() * Log(f->Clone());
  }
  if (derivative_g->IsEqual(Constants::Zero())) {
    // x ^ a = a *( x ^ (a-1))
    return std::move(derivative_f) * g->Clone() *
           Pow(f->Clone(), g->Clone() - 1.0);
  }

  // (f(x)^g(x))' = f(x) ^ (g(x)-1) * (g(x)*f'(x) + f(x)*log(f(x)*g'(x)))
  auto a =
      Pow(f->Clone(),
          (g->Clone() - 1.0)->SymCalc(SymCalcSettings::KeepNamedConstants));

  std::vector<std::unique_ptr<INode>> b;
  b.push_back(
      INodeHelper::MakeMultIfNeeded(g->Clone(), std::move(derivative_f)));

  if (!derivative_g->IsEqual(Constants::Zero())) {
    b.push_back(
        f->Clone() *
        INodeHelper::MakeLogIfNeeded(Constants::E()->Clone(), f->Clone()) *
        std::move(derivative_g));
  }
  return INodeHelper::MakeMultIfNeeded(
      std::move(a), (INodeHelper::MakePlusIfNeeded(std::move(b))));
}

std::unique_ptr<INode> DoDiffLogOperation(const Operation* operation,
                                          const Variable& by_var) {
  auto base = operation->Operand(0);
  auto f = operation->Operand(1);
  auto derivative_base = DoDiffNode(base, by_var);
  if (!INodeHelper::AsConstant(derivative_base.get())) {
    return INodeHelper::MakeError(L"base is not constant");
  }

  return DoDiffNode(f, by_var) /
         (INodeHelper::MakeMultIfNeeded(
             f->Clone(), INodeHelper::MakeLogIfNeeded(Constants::E()->Clone(),
                                                      base->Clone())));
}

std::unique_ptr<INode> DoDiffNode(const INode* node, const Variable& by_var) {
  if (auto* as_const = node->AsNodeImpl()->AsConstant())
    return INodeHelper::MakeConst(0.0);
  if (auto* as_imag = node->AsNodeImpl()->AsImaginary())
    return INodeHelper::MakeConst(0.0);
  if (auto* as_operation = node->AsNodeImpl()->AsOperation())
    return DoDiffOperation(as_operation, by_var);
  if (auto* as_var = node->AsNodeImpl()->AsVariable())
    return DoDiffVariable(as_var, by_var);
  assert(false);
  return nullptr;
}
}  // namespace

DiffOperation::DiffOperation(std::unique_ptr<INode> lh,
                             std::unique_ptr<VariableRef> rh)
    : Operation(GetOpInfo(Op::Diff), std::move(lh), std::move(rh)) {}

std::unique_ptr<INode> DiffOperation::Clone() const {
  auto lh = Operand(0)->Clone();
  auto rh = std::make_unique<VariableRef>(Operand(1)->AsVariable());
  return INodeHelper::MakeDiff(std::move(lh), std::move(rh));
}

PrintSize DiffOperation::Render(Canvas* canvas,
                                PrintBox print_box,
                                bool dry_run,
                                RenderBehaviour render_behaviour) const {
  auto prefix_size = RenderPrefix(canvas, print_box, dry_run, render_behaviour);
  assert(first_non_diff_operand_ != nullptr);

  auto operand_size =
      dry_run ? first_non_diff_operand_->Render(canvas, PrintBox::Infinite(),
                                                dry_run, render_behaviour)
              : first_non_diff_operand_->LastPrintSize();
  print_box = print_box.ShrinkLeft(prefix_size.width);
  PrintBox inner_print_box;
  auto brackets_size = canvas->RenderBrackets(
      print_box, BracketType::Round, operand_size, dry_run, &inner_print_box);
  if (!dry_run) {
    first_non_diff_operand_->Render(canvas, inner_print_box, dry_run,
                                    render_behaviour);
  }

  return print_size_ = prefix_size.GrowWidth(brackets_size, true);
}

const INode* DiffOperation::Value() const {
  return Operand(0);
}

const Variable* DiffOperation::ByVar() const {
  return Operand(1)->AsNodeImpl()->AsVariable();
}

PrintSize DiffOperation::RenderPrefix(Canvas* canvas,
                                      PrintBox print_box,
                                      bool dry_run,
                                      RenderBehaviour render_behaviour) const {
  if (dry_run) {
    std::vector<const DiffOperation*> diffs = GetDiffChain();
    std::unique_ptr<INode> top = INodeHelper::MakePowIfNeeded(
        INodeHelper::MakeConst(0.0, L"δ"), static_cast<double>(diffs.size()));
    std::vector<std::unique_ptr<INode>> bottom_nodes;
    for (auto diff : diffs) {
      bottom_nodes.push_back(
          INodeHelper::MakeConst(0.0, L"δ" + diff->ByVar()->GetName()));
    }
    std::unique_ptr<INode> bottom =
        INodeHelper::MakeMultIfNeeded(std::move(bottom_nodes));
    if (auto* as_op = INodeHelper::AsOperation(bottom.get())) {
      std::unique_ptr<INode> new_bottom;
      as_op->SimplifyTheSame({}, &new_bottom);
      if (new_bottom)
        bottom = std::move(new_bottom);
    }
    prefix_ = INodeHelper::MakeDiv(std::move(top), std::move(bottom));
    first_non_diff_operand_ = diffs.back()->Operand(0);
  }
  return prefix_->Render(canvas, print_box, dry_run, render_behaviour);
}

std::vector<const DiffOperation*> DiffOperation::GetDiffChain() const {
  if (auto* as_diff = INodeHelper::AsDiff(Value())) {
    auto result = as_diff->GetDiffChain();
    result.insert(result.begin(), this);
    return result;
  }
  return {this};
}

std::unique_ptr<INode> Differential(
    const OpInfo* op,
    std::vector<std::unique_ptr<INode>>* operands) {
  assert(op->op == Op::Diff);
  auto by_var = (*operands)[1]->AsNodeImpl()->AsVariable();
  assert(by_var);

  return DoDiffNode((*operands)[0].get(), *by_var);
}
