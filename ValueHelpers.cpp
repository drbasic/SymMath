#include "ValueHelpers.h"

#include "Brackets.h"
#include "Constant.h"
#include "DivOperation.h"
#include "INode.h"
#include "INodeHelper.h"
#include "MultOperation.h"
#include "OpInfo.h"
#include "Operation.h"
#include "PlusOperation.h"
#include "TrigonometricOperation.h"
#include "UnMinusOperation.h"

Variable Var(std::string name) {
  return Variable(std::move(name));
}

std::unique_ptr<INode> Const(double val) {
  return std::make_unique<Constant>(val);
}

//=============================================================================
std::unique_ptr<INode> operator-(std::unique_ptr<INode> lh) {
  return INodeHelper::MakeUnMinus(std::move(lh));
}

std::unique_ptr<INode> operator-(std::unique_ptr<INode> lh,
                                 std::unique_ptr<INode> rh) {
  return INodeHelper::MakeMinus(std::move(lh), std::move(rh));
}

std::unique_ptr<INode> operator-(double lh, std::unique_ptr<INode> rh) {
  return INodeHelper::MakeMinus(std::make_unique<Constant>(lh), std::move(rh));
}

std::unique_ptr<INode> operator-(std::unique_ptr<INode> lh, double rh) {
  return INodeHelper::MakeMinus(std::move(lh), std::make_unique<Constant>(rh));
}
//=============================================================================
std::unique_ptr<INode> operator+(std::unique_ptr<INode> lh,
                                 std::unique_ptr<INode> rh) {
  return INodeHelper::MakePlus(std::move(lh), std::move(rh));
}

std::unique_ptr<INode> operator+(double lh, std::unique_ptr<INode> rh) {
  return INodeHelper::MakePlus(std::make_unique<Constant>(lh), std::move(rh));
}

std::unique_ptr<INode> operator+(std::unique_ptr<INode> lh, double rh) {
  return INodeHelper::MakePlus(std::move(lh), std::make_unique<Constant>(rh));
}
//=============================================================================
std::unique_ptr<INode> operator*(std::unique_ptr<INode> lh,
                                 std::unique_ptr<INode> rh) {
  return std::make_unique<MultOperation>(std::move(lh), std::move(rh));
}

std::unique_ptr<INode> operator*(double lh, std::unique_ptr<INode> rh) {
  return std::make_unique<MultOperation>(std::make_unique<Constant>(lh),
                                         std::move(rh));
}

std::unique_ptr<INode> operator*(std::unique_ptr<INode> lh, double rh) {
  return std::make_unique<MultOperation>(std::move(lh),
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
  return std::make_unique<TrigonometricOperation>(GetOpInfo(Op::Sin),
                                                 std::move(value));
}

std::unique_ptr<INode> Cos(std::unique_ptr<INode> value) {
  return std::make_unique<TrigonometricOperation>(GetOpInfo(Op::Cos),
                                                 std::move(value));
}

//=============================================================================
std::wostream& operator<<(std::wostream& out, const Variable& v) {
  out << v.Print();
  return out;
}
//=============================================================================