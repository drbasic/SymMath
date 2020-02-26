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

void BacMinusCab() {
  auto a1 = Var("a1");
  auto a2 = Var("a2");
  auto a3 = Var("a3");
  auto b1 = Var("b1");
  auto b2 = Var("b2");
  auto b3 = Var("b3");
  auto c1 = Var("c1");
  auto c2 = Var("c2");
  auto c3 = Var("c3");

  auto a = Var("a");
  a = Vector3(a1, a2, a3);
  std::wcout << a.Print() << "\n";

  auto b = Var("b");
  b = Vector3(b1, b2, b3);
  std::wcout << b.Print() << "\n";

  auto c = Var("c");
  c = Vector3(c1, c2, c3);
  std::wcout << c.Print() << "\n";

  Variable s_ab = a * b;
  std::wcout << s_ab.Print(true) << "\n";

  Variable v_aa = VectorMult(b, c);
  std::wcout << v_aa.Print(true) << "\n";

  Variable v_ab = VectorMult(a, b);
  std::wcout << v_ab.Print(true) << "\n";

  Variable v_ba = VectorMult(b, a);
  std::wcout << v_ba.Print(true) << "\n";

  Variable vv = v_ab + v_ba;
  std::wcout << vv.Print(true) << "\n";

  Variable sv_abc = a * VectorMult(b, c);
  std::wcout << sv_abc.Print(true) << "\n";
  Variable sv_bca = b * VectorMult(c, a);
  std::wcout << sv_bca.Print(true) << "\n";
  Variable sv_cab = c * VectorMult(a, b);
  std::wcout << sv_cab.Print(true) << "\n";

  Variable eq_sv =
      Vector3(sv_abc == sv_bca, sv_bca == sv_cab, sv_cab == sv_abc);
  std::wcout << eq_sv.Print(true) << "\n";

  Variable abc = VectorMult(a, VectorMult(b, c));
  std::wcout << abc.Print(true) << "\n";

  Variable bac = (b * (a * c));
  std::wcout << bac.Print(true) << "\n";

  Variable cab = (c * (a * b));
  std::wcout << cab.Print(true) << "\n";

  Variable bac_cab = bac - cab;
  std::wcout << bac_cab.Print(true) << "\n";

  Variable abc_bac_cab = abc == (bac - cab);
  std::wcout << abc_bac_cab.Print(true) << "\n";
}

void EulerEquation() {
  auto x = Var("x");
  {
    Variable t = (Sin(x) ^ 2) + (Cos(x) ^ 2);
    std::wcout << t.Print(true) << "\n";
    t.ConvertToComplex();
    t.OpenBrackets();
    std::wcout << t.Print(true) << "\n";
  }
  {
    Variable t1 = 2 * Sin(x) * Cos(x);
    t1.ConvertToComplex();
    t1.OpenBrackets();
    t1.Simplify();

    Variable t2 = Sin(2 * x);
    t2.ConvertToComplex();
    t2.OpenBrackets();
    t2.Simplify();

    Variable t12 = t1 == t2;
    std::wcout << t12.Print(true) << "\n";
  }
  {
    Variable t3 = Pow(Cos(x), 2) - Pow(Sin(x), 2);
    t3.ConvertToComplex();
    t3.OpenBrackets();
    std::wcout << t3.Print(true) << "\n";

    Variable t4 = Cos(2 * x);
    t4.ConvertToComplex();
    t4.OpenBrackets();
    std::wcout << t4.Print(true) << "\n";

    Variable t34 = t3 == t4;
    std::wcout << t34.Print(true) << "\n";
  }
  {
    Variable t = Diff(Sin(x), x);
    t.ConvertToComplex();
    Variable dt = Cos(x);
    dt.ConvertToComplex();
    Variable cmp = t == dt;
    std::wcout << cmp.Print(true) << "\n";
  }
}

int main() {
  _setmode(_fileno(stdout), _O_U16TEXT);
  // Tests::Run();

  // BacMinusCab();
  // EulerEquation();
  // return 0;
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

  Variable t1 = Diff(Diff(Sin(x), x), x);
  t1.ConvertToComplex();
  std::wcout << t1.Print(true) << "\n";
}
