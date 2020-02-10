#pragma once
#include <memory>
#include <ostream>
#include <string>

#include "Variable.h"

class INode;
Variable Var(std::string name);
std::unique_ptr<INode> Const(double val);

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

std::ostream& operator<<(std::ostream& out, const Variable& v);
