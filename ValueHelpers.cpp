#include "ValueHelpers.h"

#include "Constant.h"
#include "INode.h"
#include "OpInfo.h"
#include "Operation.h"

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
  return std::make_unique<Operation>(GetOpInfo(Op::Div), std::move(lh),
                                     std::move(rh));
}
std::unique_ptr<INode> operator/(double lh, std::unique_ptr<INode> rh) {
  return std::make_unique<Operation>(
      GetOpInfo(Op::Div), std::make_unique<Constant>(lh), std::move(rh));
}
std::unique_ptr<INode> operator/(std::unique_ptr<INode> lh, double rh) {
  return std::make_unique<Operation>(GetOpInfo(Op::Div), std::move(lh),
                                     std::make_unique<Constant>(rh));
}
//=============================================================================
std::ostream& operator<<(std::ostream& out, const Variable& v) {
  out << v.Print();
  return out;
}
//=============================================================================