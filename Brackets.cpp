#include "Brackets.h"

#include <cassert>

Brackets::Brackets(BracketType bracket_type, std::unique_ptr<INode> value)
    : bracket_type_(bracket_type), value_(std::move(value)) {}

PrintSize Brackets::RenderBrackets(const INode* node,
                                   BracketType bracket_type,
                                   Canvas* canvas,
                                   PrintBox print_box,
                                   bool dry_run,
                                   MinusBehavior minus_behavior) {
  PrintSize inner_size =
      !dry_run ? node->LastPrintSize()
               : node->Render(canvas, print_box, dry_run, minus_behavior);

  PrintBox inner_print_box;
  PrintSize total_size;
  {
    // Left bracket
    auto lh_size = canvas->RenderBracket(print_box, Bracket::Left, bracket_type,
                                         inner_size.height, dry_run);
    total_size = total_size.GrowWidth(lh_size, false);
    print_box = print_box.ShrinkLeft(lh_size.width);
    // Center inner by height
    inner_print_box = print_box;
    inner_print_box.y += (lh_size.height - inner_size.height) / 2;
    inner_print_box.base_line = inner_print_box.y + inner_size.base_line;
  }
  {
    // Inner value
    if (!dry_run) {
      auto value_size =
          node->Render(canvas, inner_print_box, dry_run, minus_behavior);
      assert(inner_size == value_size);
    }
    total_size = total_size.GrowWidth(inner_size, false);
    print_box = print_box.ShrinkLeft(inner_size.width);
  }
  {
    // Right bracket
    auto rh_size = canvas->RenderBracket(
        print_box, Bracket::Right, bracket_type, inner_size.height, dry_run);
    total_size.base_line = 0;
    total_size = total_size.GrowWidth(rh_size, false);
    total_size.base_line = rh_size.base_line;
  }
  return total_size;
}

std::unique_ptr<INode> Brackets::SymCalc() const {
  return value_->SymCalc();
}

bool Brackets::IsEqual(const INode* rh) const {
  return value_->IsEqual(rh);
}

std::unique_ptr<INode> Brackets::Clone() const {
  return std::make_unique<Brackets>(bracket_type_, value_->Clone());
}

PrintSize Brackets::Render(Canvas* canvas,
                           PrintBox print_box,
                           bool dry_run,
                           MinusBehavior minus_behavior) const {
  return print_size_ =
             RenderBrackets(value_.get(), bracket_type_, canvas,
                            std::move(print_box), dry_run, minus_behavior);
}

PrintSize Brackets::LastPrintSize() const {
  return print_size_;
}

int Brackets::Priority() const {
  return 1000;
}

bool Brackets::HasFrontMinus() const {
  return false;
}

bool Brackets::CheckCircular(const INode* other) const {
  return value_->CheckCircular(other);
}

Constant* Brackets::AsConstant() {
  return transparent_ ? value_->AsConstant() : nullptr;
}

const Constant* Brackets::AsConstant() const {
  return transparent_ ? value_->AsConstant() : nullptr;
}

const ErrorNode* Brackets::AsError() const {
  return transparent_ ? value_->AsError() : nullptr;
}

const Variable* Brackets::AsVariable() const {
  return transparent_ ? value_->AsVariable() : nullptr;
}

Operation* Brackets::AsOperation() {
  return transparent_ ? value_->AsOperation() : nullptr;
}

const Operation* Brackets::AsOperation() const {
  return transparent_ ? value_->AsOperation() : nullptr;
}
