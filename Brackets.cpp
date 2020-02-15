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
  // Render brackets
  PrintBox inner_print_box;
  auto result = canvas->RenderBrackets(print_box, bracket_type, inner_size,
                                       dry_run, &inner_print_box);
  // Render inner value
  if (!dry_run) {
    auto value_size =
        node->Render(canvas, inner_print_box, dry_run, minus_behavior);
    assert(inner_size == value_size);
  }

  return result;
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
