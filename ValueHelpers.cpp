#include "ValueHelpers.h"

#include "Brackets.h"
#include "Constant.h"
#include "DivOperation.h"
#include "INode.h"
#include "INodeHelper.h"
#include "Imaginary.h"
#include "MultOperation.h"
#include "OpInfo.h"
#include "Operation.h"
#include "PlusOperation.h"
#include "PowOperation.h"
#include "TrigonometricOperation.h"
#include "UnMinusOperation.h"
#include "Vector.h"

Variable Var(std::string name) {
  return Variable(std::move(name));
}

Variable Var(std::string name, double val) {
  return Variable(std::move(name), Const(val));
}

std::unique_ptr<INode> Const(double val) {
  return INodeHelper::MakeConst(val);
}

std::unique_ptr<INode> iConst(double val) {
  if (val == 1.0)
    return INodeHelper::MakeImaginary();
  return INodeHelper::MakeMult(INodeHelper::MakeConst(val),
                               INodeHelper::MakeImaginary());
}

std::unique_ptr<INode> Imag() {
  return INodeHelper::MakeImaginary();
}

std::unique_ptr<INode> Vector2(std::unique_ptr<INode> a,
                               std::unique_ptr<INode> b) {
  return INodeHelper::MakeVector(std::move(a), std::move(b));
}

std::unique_ptr<INode> Vector3(std::unique_ptr<INode> a,
                               std::unique_ptr<INode> b,
                               std::unique_ptr<INode> c) {
  return INodeHelper::MakeVector(std::move(a), std::move(b), std::move(c));
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
  return INodeHelper::MakeMinus(INodeHelper::MakeConst(lh), std::move(rh));
}

std::unique_ptr<INode> operator-(std::unique_ptr<INode> lh, double rh) {
  return INodeHelper::MakeMinus(std::move(lh), INodeHelper::MakeConst(rh));
}
//=============================================================================
std::unique_ptr<INode> operator+(std::unique_ptr<INode> lh,
                                 std::unique_ptr<INode> rh) {
  return INodeHelper::MakePlus(std::move(lh), std::move(rh));
}

std::unique_ptr<INode> operator+(double lh, std::unique_ptr<INode> rh) {
  return INodeHelper::MakePlus(INodeHelper::MakeConst(lh), std::move(rh));
}

std::unique_ptr<INode> operator+(std::unique_ptr<INode> lh, double rh) {
  return INodeHelper::MakePlus(std::move(lh), INodeHelper::MakeConst(rh));
}
//=============================================================================
std::unique_ptr<INode> operator*(std::unique_ptr<INode> lh,
                                 std::unique_ptr<INode> rh) {
  return INodeHelper::MakeMult(std::move(lh), std::move(rh));
}

std::unique_ptr<INode> operator*(double lh, std::unique_ptr<INode> rh) {
  return INodeHelper::MakeMult(INodeHelper::MakeConst(lh), std::move(rh));
}

std::unique_ptr<INode> operator*(std::unique_ptr<INode> lh, double rh) {
  return INodeHelper::MakeMult(std::move(lh), INodeHelper::MakeConst(rh));
}
//=============================================================================
std::unique_ptr<INode> operator/(std::unique_ptr<INode> lh,
                                 std::unique_ptr<INode> rh) {
  return INodeHelper::MakeDiv(std::move(lh), std::move(rh));
}

std::unique_ptr<INode> operator/(double lh, std::unique_ptr<INode> rh) {
  return INodeHelper::MakeDiv(INodeHelper::MakeConst(lh), std::move(rh));
}

std::unique_ptr<INode> operator/(std::unique_ptr<INode> lh, double rh) {
  return INodeHelper::MakeDiv(std::move(lh), INodeHelper::MakeConst(rh));
}
//=============================================================================
std::unique_ptr<INode> operator^(std::unique_ptr<INode> lh,
                                 std::unique_ptr<INode> rh) {
  return INodeHelper::MakePow(std::move(lh), std::move(rh));
}

std::unique_ptr<INode> Pow(std::unique_ptr<INode> lh,
                           std::unique_ptr<INode> rh) {
  return INodeHelper::MakePow(std::move(lh), std::move(rh));
}

std::unique_ptr<INode> Pow(std::unique_ptr<INode> lh, double exp) {
  return INodeHelper::MakePow(std::move(lh), INodeHelper::MakeConst(exp));
}
//=============================================================================
std::unique_ptr<INode> AddBrackets(std::unique_ptr<INode> value) {
  return INodeHelper::MakeBrackets(BracketType::Round, std::move(value));
}

std::unique_ptr<INode> AddBrackets(BracketType bracket_type,
                                   std::unique_ptr<INode> value) {
  return INodeHelper::MakeBrackets(bracket_type, std::move(value));
}
//=============================================================================
std::unique_ptr<INode> Sin(std::unique_ptr<INode> value) {
  return INodeHelper::MakeTrigonometric(Op::Sin, std::move(value));
}

std::unique_ptr<INode> Cos(std::unique_ptr<INode> value) {
  return INodeHelper::MakeTrigonometric(Op::Cos, std::move(value));
}

//=============================================================================
std::wostream& operator<<(std::wostream& out, const Variable& v) {
  out << v.Print();
  return out;
}
//=============================================================================
