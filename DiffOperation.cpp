#include "DiffOperation.h"

#include <cassert>

#include "Brackets.h"
#include "Constant.h"
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
    case Op::Ln: {
      auto base = operation->Operand(0);
      auto f = operation->Operand(1);
      assert(INodeHelper::AsConstant(base));
      return DoDiffNode(f, by_var) /
             (f->Clone() * INodeHelper::MakeLn(base->Clone()));
    } break;
    case Op::Equal: {
      assert(false);
    } break;
    case Op::Diff: {
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

  return (DoDiffNode(f, by_var) * g->Clone() -
          f->Clone() * DoDiffNode(g, by_var)) /
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
  return Pow(f->Clone(), g->Clone() - 1.0) *
         (g->Clone() * DoDiffNode(f, by_var) +
          f->Clone() * INodeHelper::MakeLn(f->Clone()) * DoDiffNode(g, by_var));
}

std::unique_ptr<INode> DoDiffNode(const INode* node, const Variable& by_var) {
  if (auto* as_const = node->AsNodeImpl()->AsConstant())
    return INodeHelper::MakeConst(0.0);
  if (auto* as_operation = node->AsNodeImpl()->AsOperation())
    return DoDiffOperation(as_operation, by_var);
  if (auto* as_var = node->AsNodeImpl()->AsVariable())
    return DoDiffVariable(as_var, by_var);

  return nullptr;
}
}  // namespace

DiffOperation::DiffOperation(std::unique_ptr<INode> lh,
                             std::unique_ptr<VariableRef> rh)
    : Operation(GetOpInfo(Op::Diff), std::move(lh), std::move(rh)) {}

std::unique_ptr<INode> DiffOperation::Clone() const {
  return INodeHelper::MakeDiff(Operand(0)->Clone(), Operand(1)->Clone());
}

PrintSize DiffOperation::Render(Canvas* canvas,
                                PrintBox print_box,
                                bool dry_run,
                                RenderBehaviour render_behaviour) const {
  auto prefix_size = RenderPrefix(canvas, print_box, dry_run, render_behaviour);

  auto operand_size = dry_run ? Operand(0)->Render(canvas, PrintBox::Infinite(),
                                                   dry_run, render_behaviour)
                              : Operand(0)->LastPrintSize();
  print_box = print_box.ShrinkLeft(prefix_size.width);
  PrintBox inner_print_box;
  auto brackets_size = canvas->RenderBrackets(
      print_box, BracketType::Round, operand_size, dry_run, &inner_print_box);
  if (!dry_run) {
    Operand(0)->Render(canvas, inner_print_box, dry_run, render_behaviour);
  }

  return prefix_size.GrowWidth(brackets_size, true);
}

PrintSize DiffOperation::RenderPrefix(Canvas* canvas,
                                      PrintBox print_box,
                                      bool dry_run,
                                      RenderBehaviour render_behaviour) const {
  std::wstring top_text = L"δ";
  auto temp = Operand(1)->AsVariable()->GetName();
  std::wstring bottom_text = L"δ" + std::wstring(temp.begin(), temp.end());
  PrintSize print_size{bottom_text.size(), 3, 1};
  if (dry_run)
    return print_size;

  // Render divider
  auto div_size = canvas->RenderDivider(print_box, print_size.width, dry_run);

  {
    // Render top
    PrintBox lh_box(print_box);
    lh_box.x = lh_box.x + (print_size.width - top_text.size()) / 2;
    lh_box.height = print_box.base_line - div_size.base_line;
    lh_box.base_line = lh_box.height - 1;
    canvas->PrintAt(lh_box, top_text, render_behaviour.GetSubSuper(), dry_run);
  }
  {
    // Render bottom
    PrintBox rh_box(print_box);
    rh_box.x = rh_box.x + (div_size.width - bottom_text.size()) / 2;
    rh_box.y = print_box.base_line + (div_size.height - div_size.base_line);
    rh_box.base_line = rh_box.y;
    canvas->PrintAt(rh_box, bottom_text, render_behaviour.GetSubSuper(),
                    dry_run);
  }

  return print_size;
}

std::unique_ptr<INode> Differential(
    const OpInfo* op,
    std::vector<std::unique_ptr<INode>>* operands) {
  assert(op->op == Op::Diff);
  auto by_var = (*operands)[1]->AsNodeImpl()->AsVariable();
  assert(by_var);

  return DoDiffNode((*operands)[0].get(), *by_var);
}
