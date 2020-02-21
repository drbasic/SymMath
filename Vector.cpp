#include "Vector.h"

#include <cassert>

#include "Brackets.h"

Vector::Vector() {}

Vector::Vector(std::unique_ptr<INode> a, std::unique_ptr<INode> b) {
  values_.reserve(2);
  values_.push_back(std::move(a));
  values_.push_back(std::move(b));
}

Vector::Vector(std::unique_ptr<INode> a,
               std::unique_ptr<INode> b,
               std::unique_ptr<INode> c) {
  values_.reserve(3);
  values_.push_back(std::move(a));
  values_.push_back(std::move(b));
  values_.push_back(std::move(c));
}

Vector::Vector(std::vector<std::unique_ptr<INode>> values)
    : values_(std::move(values)) {}

bool Vector::IsEqual(const INode* rh) const {
  const Vector* rh_vector = rh->AsNodeImpl()->AsVector();
  if (!rh_vector)
    return false;
  if (values_.size() != rh_vector->values_.size())
    return false;
  for (size_t i = 0; i < values_.size(); ++i) {
    if (!values_[i]->IsEqual(rh_vector->values_[i].get()))
      return false;
  }
  return true;
}

std::unique_ptr<INode> Vector::Clone() const {
  auto result = std::make_unique<Vector>();
  result->values_.reserve(values_.size());
  for (const auto& val : values_)
    result->values_.push_back(val->Clone());
  return result;
}

std::unique_ptr<INode> Vector::SymCalc() const {
  auto result = std::make_unique<Vector>();
  result->values_.reserve(values_.size());
  for (const auto& val : values_)
    result->values_.push_back(val->SymCalc());
  return result;
}

PrintSize Vector::Render(Canvas* canvas,
                         PrintBox print_box,
                         bool dry_run,
                         RenderBehaviour render_behaviour) const {
  render_behaviour.TakeMinus();
  render_behaviour.TakeBrackets();

  if (dry_run) {
    values_print_size_ =
        RenderAllValues(canvas, print_box, dry_run, render_behaviour);
  }

  PrintBox values_box;
  auto print_size = canvas->RenderBrackets(
      print_box, BracketType::Square, values_print_size_, dry_run, &values_box);

  if (!dry_run) {
    auto values_print_size =
        RenderAllValues(canvas, values_box, dry_run, render_behaviour);
    assert(values_print_size == values_print_size_);
    assert(print_size == print_size_);
  }

  return print_size_ = print_size;
}

bool Vector::CheckCircular(const INodeImpl* other) const {
  for (const auto& val : values_) {
    if (val->AsNodeImpl()->CheckCircular(other))
      return true;
  }
  return false;
}

std::unique_ptr<INode> Vector::TakeValue(size_t indx) {
  return std::move(values_[indx]);
}

PrintSize Vector::RenderAllValues(Canvas* canvas,
                                  PrintBox print_box,
                                  bool dry_run,
                                  RenderBehaviour render_behaviour) const {
  PrintSize values_print_size;
  for (size_t i = 0; i < values_.size(); ++i) {
    bool with_comma = i != 0;
    auto operand_size = RenderValue(values_[i].get(), canvas, print_box,
                                    dry_run, render_behaviour, with_comma);

    print_box = print_box.ShrinkLeft(operand_size.width);
    values_print_size = values_print_size.GrowWidth(operand_size, true);
  }
  return values_print_size;
}

PrintSize Vector::RenderValue(const INode* value,
                              Canvas* canvas,
                              PrintBox print_box,
                              bool dry_run,
                              RenderBehaviour render_behaviour,
                              bool with_comma) const {
  PrintSize total_value_size;
  if (with_comma) {
    auto comma_size = canvas->PrintAt(print_box, ", ",
                                      render_behaviour.GetSubSuper(), dry_run);
    total_value_size = total_value_size.GrowWidth(comma_size, true);
    print_box = print_box.ShrinkLeft(comma_size.width);
  }

  // Render valaue
  auto valaue_size =
      value->AsNodeImpl()->Render(canvas, print_box, dry_run, render_behaviour);
  return total_value_size.GrowWidth(valaue_size, true);
}
