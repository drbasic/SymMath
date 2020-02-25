#include "Brackets.h"

#include <cassert>

Brackets::Brackets(BracketType bracket_type, std::unique_ptr<INode> value)
    : bracket_type_(bracket_type), value_(std::move(value)) {}

PrintSize Brackets::RenderBrackets(const INodeImpl* node,
                                   BracketType bracket_type,
                                   Canvas* canvas,
                                   PrintBox print_box,
                                   bool dry_run,
                                   RenderBehaviour render_behaviour) {
  PrintSize inner_size =
      !dry_run ? node->LastPrintSize()
               : node->Render(canvas, print_box, dry_run, render_behaviour);
  // Render brackets
  PrintBox inner_print_box;
  auto result = canvas->RenderBrackets(print_box, bracket_type, inner_size,
                                       dry_run, &inner_print_box);
  // Render inner value
  if (!dry_run) {
    auto value_size =
        node->Render(canvas, inner_print_box, dry_run, render_behaviour);
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
                           RenderBehaviour render_behaviour) const {
  return print_size_ =
             RenderBrackets(value_->AsNodeImpl(), bracket_type_, canvas,
                            std::move(print_box), dry_run, render_behaviour);
}

PrintSize Brackets::LastPrintSize() const {
  return print_size_;
}

bool Brackets::CheckCircular(const INodeImpl* other) const {
  return value_->AsNodeImpl()->CheckCircular(other);
}

Constant* Brackets::AsConstant() {
  return transparent_ ? Value()->AsConstant() : nullptr;
}

const Constant* Brackets::AsConstant() const {
  return transparent_ ? Value()->AsConstant() : nullptr;
}

const ErrorNode* Brackets::AsError() const {
  return transparent_ ? Value()->AsError() : nullptr;
}

const Variable* Brackets::AsVariable() const {
  return transparent_ ? Value()->AsVariable() : nullptr;
}

Operation* Brackets::AsOperation() {
  return transparent_ ? Value()->AsOperation() : nullptr;
}

const Operation* Brackets::AsOperation() const {
  return transparent_ ? Value()->AsOperation() : nullptr;
}

void Brackets::SimplifyImpl(HotToken token, std::unique_ptr<INode>* new_node) {
  Value()->SimplifyImpl({&token}, new_node);
}

void Brackets::OpenBracketsImpl(HotToken token,
                                std::unique_ptr<INode>* new_node) {
  Value()->OpenBracketsImpl({&token}, new_node);
}

void Brackets::ConvertToComplexImpl(HotToken token,
                                    std::unique_ptr<INode>* new_node) {
  Value()->ConvertToComplexImpl({&token}, new_node);
}

INodeImpl* Brackets::Value() {
  return value_ ? value_->AsNodeImpl() : nullptr;
}

const INodeImpl* Brackets::Value() const {
  return value_ ? value_->AsNodeImpl() : nullptr;
}
