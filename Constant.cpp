#include "Constant.h"

Constant::Constant(double val) : value_(val) {}

std::string Constant::PrintImpl(bool ommit_front_minus) const {
  if (ommit_front_minus && HasFrontMinus())
    return std::to_string(-value_);
  return std::to_string(value_);
}

int Constant::Priority() const
{
  return 100;
}

bool Constant::NeedBrackets() const {
  return false;
}

bool Constant::HasFrontMinus() const {
  return value_ < 0;
}

bool Constant::CheckCircular(const INode* other) const {
  return false;
}

bool Constant::IsUnMinus() const
{
  return false;
}

Constant* Constant::AsConstant()
{
  return this;
}

std::unique_ptr<INode> Constant::SymCalc() const {
  return std::make_unique<Constant>(value_);
}