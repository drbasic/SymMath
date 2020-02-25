#pragma once
#include <memory>
#include <ostream>
#include <string>

#include "Constant.h"
#include "Variable.h"

class INode;

namespace Constants {
  const Constant* Zero();
  const Constant* E();
  std::unique_ptr<Constant> MakeE();
  const Constant* PI();
  std::unique_ptr<Constant> MakePI();
  const Imaginary* Imag();
}

Variable Var(std::string name);
Variable Var(std::string name, double val);
std::unique_ptr<INode> Const(double val);
std::unique_ptr<INode> Const(double val, std::string name);
std::unique_ptr<INode> iConst(double val);
std::unique_ptr<INode> Imag();
std::unique_ptr<INode> Vector2(std::unique_ptr<INode> a,
                               std::unique_ptr<INode> b);
std::unique_ptr<INode> Vector3(std::unique_ptr<INode> a,
                               std::unique_ptr<INode> b,
                               std::unique_ptr<INode> c);

std::unique_ptr<INode> operator==(std::unique_ptr<INode> lh,
                                  std::unique_ptr<INode> rh);

std::unique_ptr<INode> operator-(std::unique_ptr<INode> lh);
std::unique_ptr<INode> operator-(std::unique_ptr<INode> lh,
                                 std::unique_ptr<INode> rh);
std::unique_ptr<INode> operator-(double lh, std::unique_ptr<INode> rh);
std::unique_ptr<INode> operator-(std::unique_ptr<INode> lh, double rh);

std::unique_ptr<INode> operator+(std::unique_ptr<INode> lh,
                                 std::unique_ptr<INode> rh);
std::unique_ptr<INode> operator+(double lh, std::unique_ptr<INode> rh);
std::unique_ptr<INode> operator+(std::unique_ptr<INode> lh, double rh);

std::unique_ptr<INode> operator*(std::unique_ptr<INode> lh,
                                 std::unique_ptr<INode> rh);
std::unique_ptr<INode> operator*(double lh, std::unique_ptr<INode> rh);
std::unique_ptr<INode> operator*(std::unique_ptr<INode> lh, double rh);

std::unique_ptr<INode> operator/(std::unique_ptr<INode> lh,
                                 std::unique_ptr<INode> rh);
std::unique_ptr<INode> operator/(double lh, std::unique_ptr<INode> rh);
std::unique_ptr<INode> operator/(std::unique_ptr<INode> lh, double rh);

std::unique_ptr<INode> operator^(std::unique_ptr<INode> lh,
                                 std::unique_ptr<INode> rh);
std::unique_ptr<INode> operator^(std::unique_ptr<INode> lh,
                                 double rh);
std::unique_ptr<INode> Pow(std::unique_ptr<INode> lh,
                           std::unique_ptr<INode> rh);
std::unique_ptr<INode> Pow(std::unique_ptr<INode> lh, double exp);

std::unique_ptr<INode> AddBrackets(std::unique_ptr<INode> value);
std::unique_ptr<INode> AddBrackets(BracketType bracket_type,
                                   std::unique_ptr<INode> value);

std::unique_ptr<INode> Sin(std::unique_ptr<INode> value);
std::unique_ptr<INode> Cos(std::unique_ptr<INode> value);
std::unique_ptr<INode> Log(std::unique_ptr<INode> value);
std::unique_ptr<INode> Log2(std::unique_ptr<INode> value);
std::unique_ptr<INode> Log10(std::unique_ptr<INode> value);

std::unique_ptr<INode> VectorMult(std::unique_ptr<INode> lh,
                                  std::unique_ptr<INode> rh);

std::unique_ptr<INode> Diff(std::unique_ptr<INode> lh, const Variable& var);

std::ostream& operator<<(std::ostream& out, const Variable& v);
