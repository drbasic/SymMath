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

VariablePtr Var() {
  return INodeHelper::MakeVariable();
}

VariablePtr Var(std::string name) {
  return INodeHelper::MakeVariable(std::move(name));
}

VariablePtr Var(std::string name, double val) {
  return INodeHelper::MakeVariable(std::move(name), Const(val));
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
  return INodeHelper::MakeMult(std::move(lh), std::move(rh));
}

std::unique_ptr<INode> operator*(double lh, std::unique_ptr<INode> rh) {
  return INodeHelper::MakeMult(std::make_unique<Constant>(lh), std::move(rh));
}

std::unique_ptr<INode> operator*(std::unique_ptr<INode> lh, double rh) {
  return INodeHelper::MakeMult(std::move(lh), std::make_unique<Constant>(rh));
}
//=============================================================================
std::unique_ptr<INode> operator/(std::unique_ptr<INode> lh,
                                 std::unique_ptr<INode> rh) {
  return INodeHelper::MakeDiv(std::move(lh), std::move(rh));
}

std::unique_ptr<INode> operator/(double lh, std::unique_ptr<INode> rh) {
  return INodeHelper::MakeDiv(std::make_unique<Constant>(lh), std::move(rh));
}

std::unique_ptr<INode> operator/(std::unique_ptr<INode> lh, double rh) {
  return INodeHelper::MakeDiv(std::move(lh), std::make_unique<Constant>(rh));
}

std::unique_ptr<INode> AddBrackets(std::unique_ptr<INode> value) {
  return std::make_unique<Brackets>(BracketType::Round, std::move(value));
}

std::unique_ptr<INode> AddBrackets(BracketType bracket_type,
                                   std::unique_ptr<INode> value) {
  return std::make_unique<Brackets>(bracket_type, std::move(value));
}

std::unique_ptr<INode> Sin(std::unique_ptr<INode> value) {
  return INodeHelper::MakeTrigonometric(Op::Sin, std::move(value));
}

std::unique_ptr<INode> Cos(std::unique_ptr<INode> value) {
  return INodeHelper::MakeTrigonometric(Op::Cos, std::move(value));
}

//=============================================================================
std::wostream& operator<<(std::wostream& out, const VariablePtr& v) {
  out << v.Print();
  return out;
}
//=============================================================================