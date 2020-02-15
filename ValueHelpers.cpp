#include "ValueHelpers.h"

#include "Brackets.h"
#include "Constant.h"
#include "DivOperation.h"
#include "INode.h"
#include "OpInfo.h"
#include "Operation.h"
#include "TrigonometricOperator.h"

Variable Var(std::string name) {
  return Variable(std::move(name));
}

std::unique_ptr<INode> Const(double val) {
  return std::make_unique<Constant>(val);
}

//=============================================================================
std::unique_ptr<INode> operator-(std::unique_ptr<INode> lh) {
  return std::make_unique<Operation>(GetOpInfo(Op::UnMinus), std::move(lh));
}

std::unique_ptr<INode> operator-(std::unique_ptr<INode> lh,
                                 std::unique_ptr<INode> rh) {
  return std::make_unique<Operation>(GetOpInfo(Op::Minus), std::move(lh),
                                     std::move(rh));
}

std::unique_ptr<INode> operator-(double lh, std::unique_ptr<INode> rh) {
  return std::make_unique<Operation>(
      GetOpInfo(Op::Minus), std::make_unique<Constant>(lh), std::move(rh));
}

std::unique_ptr<INode> operator-(std::unique_ptr<INode> lh, double rh) {
  return std::make_unique<Operation>(GetOpInfo(Op::Minus), std::move(lh),
                                     std::make_unique<Constant>(rh));
}
//=============================================================================
std::unique_ptr<INode> operator+(std::unique_ptr<INode> lh,
                                 std::unique_ptr<INode> rh) {
  return std::make_unique<Operation>(GetOpInfo(Op::Plus), std::move(lh),
                                     std::move(rh));
}

std::unique_ptr<INode> operator+(double lh, std::unique_ptr<INode> rh) {
  return std::make_unique<Operation>(
      GetOpInfo(Op::Plus), std::make_unique<Constant>(lh), std::move(rh));
}

std::unique_ptr<INode> operator+(std::unique_ptr<INode> lh, double rh) {
  return std::make_unique<Operation>(GetOpInfo(Op::Plus), std::move(lh),
                                     std::make_unique<Constant>(rh));
}
//=============================================================================
std::unique_ptr<INode> operator*(std::unique_ptr<INode> lh,
                                 std::unique_ptr<INode> rh) {
  return std::make_unique<Operation>(GetOpInfo(Op::Mult), std::move(lh),
                                     std::move(rh));
}

std::unique_ptr<INode> operator*(double lh, std::unique_ptr<INode> rh) {
  return std::make_unique<Operation>(
      GetOpInfo(Op::Mult), std::make_unique<Constant>(lh), std::move(rh));
}

std::unique_ptr<INode> operator*(std::unique_ptr<INode> lh, double rh) {
  return std::make_unique<Operation>(GetOpInfo(Op::Mult), std::move(lh),
                                     std::make_unique<Constant>(rh));
}
//=============================================================================
std::unique_ptr<INode> operator/(std::unique_ptr<INode> lh,
                                 std::unique_ptr<INode> rh) {
  return std::make_unique<DivOperation>(std::move(lh), std::move(rh));
}

std::unique_ptr<INode> operator/(double lh, std::unique_ptr<INode> rh) {
  return std::make_unique<DivOperation>(std::make_unique<Constant>(lh),
                                        std::move(rh));
}

std::unique_ptr<INode> operator/(std::unique_ptr<INode> lh, double rh) {
  return std::make_unique<DivOperation>(std::move(lh),
                                        std::make_unique<Constant>(rh));
}

std::unique_ptr<INode> AddBrackets(std::unique_ptr<INode> value) {
  return std::make_unique<Brackets>(BracketType::Round, std::move(value));
}

std::unique_ptr<INode> AddBrackets(BracketType bracket_type,
                                   std::unique_ptr<INode> value) {
  return std::make_unique<Brackets>(bracket_type, std::move(value));
}

std::unique_ptr<INode> Sin(std::unique_ptr<INode> value) {
  return std::make_unique<TrigonometricOperator>(GetOpInfo(Op::Sin),
                                                 std::move(value));
}

std::unique_ptr<INode> Cos(std::unique_ptr<INode> value) {
  return std::make_unique<TrigonometricOperator>(GetOpInfo(Op::Cos),
                                                 std::move(value));
}

//=============================================================================
std::wostream& operator<<(std::wostream& out, const Variable& v) {
  out << v.Print();
  return out;
}
//=============================================================================