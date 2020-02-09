#include "Constant.h"

Constant::Constant(double val) : value_(val) {}

std::string Constant::PrintImpl(bool ommit_front_minus) const {
  if (ommit_front_minus && HasFrontMinus())
    return std::to_string(-value_);
  return std::to_string(value_);
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

bool Constant::IsUnMinus() const {
  return false;
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