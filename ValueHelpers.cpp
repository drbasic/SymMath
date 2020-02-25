#include "ValueHelpers.h"
#define _USE_MATH_DEFINES
#include <math.h>

#include "Brackets.h"
#include "CompareOperation.h"
#include "Constant.h"
#include "DiffOperation.h"
#include "DivOperation.h"
#include "INode.h"
#include "INodeHelper.h"
#include "Imaginary.h"
#include "LogOperation.h"
#include "MultOperation.h"
#include "OpInfo.h"
#include "Operation.h"
#include "PlusOperation.h"
#include "PowOperation.h"
#include "TrigonometricOperation.h"
#include "UnMinusOperation.h"
#include "VariableRef.h"
#include "Vector.h"
#include "VectorMultOperation.h"

namespace Constants {

const Constant* Zero() {
  static const auto kZero = INodeHelper::MakeConst(0.0);
  return kZero.get();
}

const Constant* E() {
  static const auto kE = INodeHelper::MakeConst(M_E, "e");
  return kE.get();
}

std::unique_ptr<Constant> MakeE() {
  return INodeHelper::MakeConst(M_E, "e");
}

const Constant* PI() {
  static const auto kPi = INodeHelper::MakeConst(M_PI, "Pi");
  return kPi.get();
}

const Imaginary* Imag() {
  static const auto kImag = INodeHelper::MakeImaginary();
  return kImag.get();
}

}  // namespace Constants

Variable Var(std::string name) {
  return Variable(std::move(name));
}

Variable Var(std::string name, double val) {
  return Variable(std::move(name), Const(val));
}

std::unique_ptr<INode> Const(double val) {
  return INodeHelper::MakeConst(val);
}

std::unique_ptr<INode> Const(double val, std::string name) {
  return INodeHelper::MakeConst(val, std::move(name));
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
std::unique_ptr<INode> operator==(std::unique_ptr<INode> lh,
                                  std::unique_ptr<INode> rh) {
  return INodeHelper::MakeCompare(Op::Equal, std::move(lh), std::move(rh));
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

std::unique_ptr<INode> operator^(std::unique_ptr<INode> lh, double rh) {
  return INodeHelper::MakePow(std::move(lh), INodeHelper::MakeConst(rh));
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

std::unique_ptr<INode> Log(std::unique_ptr<INode> value) {
  return INodeHelper::MakeLogIfNeeded(Constants::E()->Clone(),
                                      std::move(value));
}

std::unique_ptr<INode> Log2(std::unique_ptr<INode> value) {
  return INodeHelper::MakeLogIfNeeded(Const(2.0), std::move(value));
}

std::unique_ptr<INode> Log10(std::unique_ptr<INode> value) {
  return INodeHelper::MakeLogIfNeeded(Const(10.0), std::move(value));
}

std::unique_ptr<INode> VectorMult(std::unique_ptr<INode> lh,
                                  std::unique_ptr<INode> rh) {
  return INodeHelper::MakeVectorMult(std::move(lh), std::move(rh));
}

std::unique_ptr<INode> Diff(std::unique_ptr<INode> lh, const Variable& var) {
  return INodeHelper::MakeDiff(std::move(lh),
                               std::make_unique<VariableRef>(&var));
}

//=============================================================================
std::wostream& operator<<(std::wostream& out, const Variable& v) {
  out << v.Print();
  return out;
}
//=============================================================================
