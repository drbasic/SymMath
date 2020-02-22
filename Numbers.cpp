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
  auto v1 = Var("v1");
  auto v2 = Var("v2");
  auto v3 = Var("v3");
  auto pi = Var("PI");
  pi = 3.1415926;
  Variable t = Vector3(a, b, c);  /// * Vector2(a, b);

  auto a1 = Var("a1");
  auto a2 = Var("a2");
  auto a3 = Var("a3");
  auto b1 = Var("b1");
  auto b2 = Var("b2");
  auto b3 = Var("b3");
  auto c1 = Var("c1");
  auto c2 = Var("c2");
  auto c3 = Var("c3");

  a = Vector3(a1, a2, a3);
  b = Vector3(b1, b2, b3);
  c = Vector3(c1, c2, c3);
  Variable abc("abc");
  abc = VectorMult(VectorMult(a, b), c)->SymCalc();
  abc.OpenBrackets();

  Variable bac("bac");
  bac = (b * (a * c))->SymCalc();
  bac.OpenBrackets();

  Variable cab("cab");
  cab = (c * (a * b))->SymCalc();
  cab .OpenBrackets();

  Variable delta = (abc - (bac - cab))->SymCalc();
  delta.OpenBrackets();
  delta.Simplify();
  std::wcout << abc.Print() << "\n";
  std::wcout << bac.Print() << "\n";
  std::wcout << cab.Print() << "\n";
  std::wcout << delta.Print() << "\n";
}
