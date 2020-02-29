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
  auto a1 = Var(L"a1");
  auto a2 = Var(L"a2");
  auto a3 = Var(L"a3");
  auto b1 = Var(L"b1");
  auto b2 = Var(L"b2");
  auto b3 = Var(L"b3");
  auto c1 = Var(L"c1");
  auto c2 = Var(L"c2");
  auto c3 = Var(L"c3");

  auto a = Var(L"a");
  a = Vector3(a1, a2, a3);
  std::wcout << a.Print() << "\n";

  auto b = Var(L"b");
  b = Vector3(b1, b2, b3);
  std::wcout << b.Print() << "\n";

  auto c = Var(L"c");
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
  auto x = Var(L"x");
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
  auto a = Var(L"a");
  auto b = Var(L"b");
  auto c = Var(L"c");
  auto d = Var(L"d");
  auto e = Var(L"e");
  auto y = Var(L"y");
  auto x = Var(L"x");
  auto v1 = Var(L"v1");
  auto v2 = Var(L"v2");
  auto v3 = Var(L"v3");
  auto pi = Var(L"PI");
  pi = 3.1415926;

  Variable t1 = (Sqrt(Const(64)) * Sqrt(100 + Sqrt(Const(81))) )/ (1 + Sqrt(Const(1)));
  t1 = t1.SymCalc(SymCalcSettings::KeepNamedConstants);
  // t1.ConvertToComplex();
  std::wcout << t1.Print(false) << "\n";
}
