#include "OpInfo.h"

#include "CompareOperation.h"
#include "DiffOperation.h"
#include "Exception.h"
#include "LogOperation.h"
#include "SqrtOperation.h"
#include "VectorScalarProduct.h"

namespace {
constexpr OpInfo kOps[] = {
    {Op::UnMinus, 11, L"-", [](double lh, double rh) { return -lh; }, false, 1,
     VectorUnMinus},
    {Op::Minus, 10, L" - ", [](double lh, double rh) { return lh - rh; }, false,
     2},
    {Op::Plus, 10, L" + ", [](double lh, double rh) { return lh + rh; }, true,
     -1, VectorAdd},
    {Op::Mult, 20, L"∙", [](double lh, double rh) { return lh * rh; }, true, -1,
     ScalarProduct},
    {Op::VectorMult, 20, L" ╲╱ ", nullptr, true, 2, VectorProduct},
    {Op::Div, 20, L"/", [](double lh, double rh) { return lh / rh; }, false, 2},
    {Op::Pow, 30, L"^", [](double lh, double rh) { return pow(lh, rh); }, false,
     2},
    {Op::Sqrt, 30, L"sqrt", nullptr, false, 2, NonTrivialSqrt},
    {Op::Sin, 1000, L"sin", [](double lh, double rh) { return sin(lh); }, false,
     1},
    {Op::Cos, 1000, L"cos", [](double lh, double rh) { return cos(lh); }, false,
     1},
    {Op::Log, 1000, L"log", TrivialLogCalc, false, 2},
    {Op::Equal, 5, L" == ",
     [](double lh, double rh) -> double { return (lh == rh) ? 1.0 : 0.0; },
     true, 2, CompareEqual},
    {Op::Diff, 5, L" x/dx ", nullptr, true, 2, Differential},
};
}

const OpInfo* GetOpInfo(Op op) {
  if (static_cast<int>(op) >= std::extent<decltype(kOps)>::value)
    throw Exception("Unknown op");
  auto result = &kOps[static_cast<int>(op)];
  if (result->op != op)
    throw Exception("Wrong op order");
  return result;
}
