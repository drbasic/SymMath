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
  // Tests::Run();

  auto a = Var("a");
  auto b = Var("b");
  auto c = Var("c");
  auto d = Var("d");
  auto e = Var("e");
  auto y = Var("y");
  auto x = Var("x");
  auto v = Var("v");
  auto pi = Var("PI");
  pi = 3.1415926;
  Variable t = Vector2(a, b);/// * Vector2(a, b);
  v = t;
  y = v * t* a;
  // a = 1;
  // b = 2;
  // c = 3;
  // x = 5;
  Variable calc = y.SymCalc();
  calc.Simplify();
  std::wcout << y.Print() << calc.Print() << "\n";
}
