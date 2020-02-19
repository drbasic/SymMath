#include <Windows.h>
#include <fcntl.h>
#include <io.h>
#include <locale.h>
#include <wincon.h>

#include <iostream>

#include "Brackets.h"
#include "INode.h"
#include "Tests.h"
#include "ValueHelpers.h"

int main() {
  _setmode(_fileno(stdout), _O_U16TEXT);
  Tests::Run();

  auto a = Var("a");
  auto b = Var("b");
  auto c = Var("c");
  auto d = Var("d");
  auto e = Var("e");
  auto y = Var("y");
  auto x = Var("x");
  auto pi = Var("PI");
  pi = 3.1415926;
  Variable t = b*b*a * Pow(b * a, 10);
  y = t;
  Variable calc = a.SymCalc();
  std::wcout << y.Print() << calc.Print() << "\n";
  y.OpenBrackets();
  // calc = y.SymCalc();
  std::wcout << y.Print() << calc.Print() << "\n";
  y.Simplify();
  // calc = y.SymCalc();
  std::wcout << y.Print() << calc.Print() << "\n";

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
