
#include <iostream>

#include "INode.h"
#include "ValueHelpers.h"

int main() {
  auto a = Var("a");
  auto b = Var("b");
  auto c = Var("c");
  auto y = Var("y");
  auto x = Var("x");
  Variable x2 = x * -x;
  x2 = x * 2;
  y = x + x2;
  std::cout << y.Print() << "\n";
  y.Simplify();
}
