#include "Constant.h"

#include <sstream>

Constant::Constant(double val) : value_(val) {}

PrintSize Constant::Render(Canvas* canvas,
                           const PrintBox& print_box,
                           bool dry_run,
                           MinusBehavior minus_behavior) const {
  bool has_front_minus = HasFrontMinus();
  double for_print =
      (has_front_minus &&
       (minus_behavior == MinusBehavior::Ommit ||
        minus_behavior == MinusBehavior::OmmitMinusAndBrackets)) ||
              (!has_front_minus && minus_behavior == MinusBehavior::Force)
          ? -value_
          : value_;
  std::stringstream ss;
  ss << for_print;
  return print_size_ = canvas->PrintAt(print_box, ss.str(), dry_run);
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
