#include "Constant.h"

#include <sstream>

Constant::Constant(double val) : value_(val) {}

std::string Constant::PrintImpl(bool ommit_front_minus) const {
  std::stringstream ss;
  double for_print = (ommit_front_minus && HasFrontMinus()) ? -value_ : value_;
  ss << for_print;
  return ss.str();
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
