#pragma once
#include <memory>
#include <ostream>
#include <string>

#include "Variable.h"

class INode;
Variable Var(std::string name);
Variable Var(std::string name, double val);
std::unique_ptr<INode> Const(double val);
std::unique_ptr<INode> iConst(double val);
std::unique_ptr<INode> Imag();

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

std::unique_ptr<INode> AddBrackets(std::unique_ptr<INode> value);
std::unique_ptr<INode> AddBrackets(BracketType bracket_type,
                                   std::unique_ptr<INode> value);

std::unique_ptr<INode> Sin(std::unique_ptr<INode> value);
std::unique_ptr<INode> Cos(std::unique_ptr<INode> value);

std::ostream& operator<<(std::ostream& out, const Variable& v);
