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

  PrintSize total_size;
  {
    // Left bracket
    auto lh_size = canvas->RenderBracket(print_box, Bracket::Left, bracket_type,
                                         inner_size.height, dry_run);
    total_size = total_size.GrowWidth(lh_size);
    print_box = print_box.ShrinkLeft(lh_size.width);
  }
  {
    // Inner value
    if (!dry_run) {
      auto value_size =
          node->Render(canvas, print_box, dry_run, minus_behavior);
      assert(inner_size == value_size);
    }
    total_size = total_size.GrowWidth(inner_size);
    print_box = print_box.ShrinkLeft(inner_size.width);
  }
  {
    // Right bracket
    auto rh_size = canvas->RenderBracket(
        print_box, Bracket::Right, bracket_type, inner_size.height, dry_run);
    total_size = total_size.GrowWidth(rh_size);
    print_box = print_box.ShrinkLeft(rh_size.width);
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
  return 0;
}

bool Brackets::HasFrontMinus() const {
  return false;
}

bool Brackets::CheckCircular(const INode* other) const {
  return value_->CheckCircular(other);
}
