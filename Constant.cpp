#include "Constant.h"

#include <sstream>

Constant::Constant(double val) : value_(val) {}

PrintSize Constant::Render(Canvas* canvas,
                           const Position& pos,
                           bool dry_run,
                           MinusBehavior minus_behavior) const {
  double for_print =
      (minus_behavior == MinusBehavior::Ommit && HasFrontMinus()) ||
              (minus_behavior == MinusBehavior::Force && !HasFrontMinus())
          ? -value_
          : value_;
  std::stringstream ss;
  ss << for_print;
  auto str = ss.str();
  if (!dry_run) {
    canvas->PrintAt(pos, str);
  }
  return print_size_ = {str.size(), 1};
}

PrintSize Constant::LastPrintSize() const {
  return print_size_;
}

int Constant::Priority() const {
  return 100;
}

bool Constant::HasFrontMinus() const {
  return value_ < 0;
}

bool Constant::CheckCircular(const INode* other) const {
  return false;
}

bool Constant::IsEqual(const INode* rh) const {
  const Constant* rh_const = rh->AsConstant();
  return rh_const && (Value() == rh_const->Value());
}

Constant* Constant::AsConstant() {
  return this;
}
const Constant* Constant::AsConstant() const {
  return this;
}

std::unique_ptr<INode> Constant::SymCalc() const {
  return std::make_unique<Constant>(value_);
}

std::unique_ptr<INode> Constant::Clone() const {
  return std::make_unique<Constant>(value_);
}
