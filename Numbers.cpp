
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

  // Variable x2 = (c/-a) / (-b / -x);
  //Variable x2 = (-(-(-(a))))* ((a * (1/pi)*b) )/(-( c* (y /x))) +y;
  Variable x2 =   (-(-(-(a))))* ((-a * (1/pi)*b) )/(-( -c* -(y /-x))) +y;
  std::cout << x2.Print() << "\n";
  x2.Simplify();
  std::cout << x2.Print() << "\n";
  /*
  Variable x2 = (a + 2 * b + 3 * c + 4);
  // Variable x22 = x / 2;
  // y = (x + 1) - (x + 1) + (1 + x) + x2 + 3 * x2;
  // y = 4 * x2 - 3 * x2 + (3 + x);
  // y =  x2 + (x + 1);
  // y = pi / Const(3);
  y = x2 * 2 / 3 * x2 + 10 * a + 4 * b + 6 * c + 6 * pi;

  std::cout << y.Print() << "\n";
  y.Simplify();
  std::cout << y.Print() << "\n";
  */
}
