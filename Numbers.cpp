
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
  x2 = x * x;
  y = a * x2 + b * x + c;
  std::cout << y.Print() << "\n";
  y.Simplify();
  std::cout << y.Print() << "\n";

  y = (a * (-x) * x) + (b * x) - c;
  std::cout << y.Print() << "\n";
  y.Simplify();
  std::cout << y.Print() << "\n";

  Variable z = -(-(-a));
  std::cout << z.Print() << "\n";
  z.Simplify();
  std::cout << z.Print() << "\n";
}
