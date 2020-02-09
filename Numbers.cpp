
#include <iostream>

#include "INode.h"
#include "ValueHelpers.h"

int main() {
  auto a = Var("a");
  auto b = Var("b");
  auto c = Var("c");
  auto y = Var("y");
  auto x = Var("x");
  auto pi = Var("PI");
  pi = 3.1415926;
  Variable x2 = x * -x;
  x2 = 2 * (x + 1);
  Variable x22 = x / 2;
  //y = (x + 1) - (x + 1) + (1 + x) + x2 + 3 * x2;
  y =  (x * -x) +  x2 * 3;
  // y = pi / Const(3);
  std::cout << y.Print() << "\n";
  y.Simplify();
  std::cout << y.Print() << "\n";
}
