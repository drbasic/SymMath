#include "Vector.h"

#include <cassert>

#include "INodeHelper.h"
#include "PlusOperation.h"

Vector::Vector() = default;

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
    : AbstractSequence(std::move(values)) {}

bool Vector::IsEqual(const INode* rh) const {
  const Vector* rh_vector = rh->AsNodeImpl()->AsVector();
  if (!rh_vector)
    return false;
  return AbstractSequence::IsEqualSequence(rh_vector);
}

std::unique_ptr<INode> Vector::Clone() const {
  return AbstractSequence::Clone(std::make_unique<Vector>());
}

std::unique_ptr<INode> Vector::SymCalc(SymCalcSettings settings) const {
  return AbstractSequence::SymCalc(std::make_unique<Vector>(), settings);
}

PrintSize Vector::Render(Canvas* canvas,
                         PrintBox print_box,
                         bool dry_run,
                         RenderBehaviour render_behaviour) const {
  render_behaviour.TakeMinus();
  render_behaviour.TakeBrackets();
  return AbstractSequence::Render(PrintDirection::Vertical, canvas, print_box,
                                  dry_run, render_behaviour);
}

void Vector::Add(std::unique_ptr<Vector> rh) {
  for (size_t i = 0; i < rh->Size(); ++i) {
    if (Size() < i) {
      values_.push_back(rh->TakeValue(i));
    } else {
      values_[i] =
          INodeHelper::MakePlus(std::move(values_[i]), rh->TakeValue(i));
    }
  }
}
