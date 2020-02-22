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
  std::wcout << a.Print() << "\n";
  b = Vector3(b1, b2, b3);
  std::wcout << b.Print() << "\n";
  c = Vector3(c1, c2, c3);
  std::wcout << c.Print() << "\n";

  Variable v_aa = VectorMult(a, a);
  std::wcout << v_aa.Print(true) << "\n";

  Variable v_ab = VectorMult(a, b);
  std::wcout << v_ab.Print(true) << "\n";

  Variable v_ba = VectorMult(b, a);
  std::wcout << v_ba.Print(true) << "\n";

  Variable vv = v_ab + v_ba;
  std::wcout << vv.Print(true) << "\n";

  Variable s_ab = a * b;
  std::wcout << s_ab.Print(true) << "\n";

  Variable sv_abc = a * VectorMult(b, c);
  std::wcout << sv_abc.Print(true) << "\n";
  Variable sv_bca = b * VectorMult(c, a);
  std::wcout << sv_bca.Print(true) << "\n";
  Variable sv_cab = c * VectorMult(a, b);
  std::wcout << sv_cab.Print(true) << "\n";
  Variable cmp = Vector3(sv_abc == sv_bca, sv_bca == sv_cab, sv_cab == sv_abc);
  std::wcout << cmp.Print(true) << "\n";

  Variable abc("abc");
  abc = VectorMult(a, VectorMult(b, c));
  std::wcout << abc.Print(true) << "\n";

  Variable bac("bac");
  bac = (b * (a * c));
  std::wcout << bac.Print(true) << "\n";

  Variable cab("cab");
  cab = (c * (a * b));
  std::wcout << cab.Print(true) << "\n";

  Variable bac_cab("bac-cab");
  bac_cab = bac - cab;
  std::wcout << bac_cab.Print(true) << "\n";

  //Variable abc_bac_cab("abc-bac-cab");
  Variable abc_bac_cab = abc == (bac - cab);
  std::wcout << abc_bac_cab.Print(true) << "\n";
}
