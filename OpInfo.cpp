#include "OpInfo.h"

#include "CompareOperation.h"
#include "DiffOperation.h"
#include "Exception.h"
#include "INodeImpl.h"
#include "LogOperation.h"
#include "SqrtOperation.h"
#include "VectorScalarProduct.h"

namespace {
constexpr OpInfo kOps[] = {
    {Op::UnMinus, NodeType::UnMinusOperation, 11, L"-",
     [](double lh, double rh) { return -lh; }, false, 1, VectorUnMinus},

    {Op::Minus, NodeType::MinusOperation, 10, L" - ", nullptr, true, 2},

    {Op::Plus, NodeType::PlusOperation, 10, L" + ",
     [](double lh, double rh) { return lh + rh; }, true, -1, VectorAdd},

    {Op::Mult, NodeType::MultOperation, 20, L"∙",
     [](double lh, double rh) { return lh * rh; }, true, -1, ScalarProduct},

    {Op::VectorMult, NodeType::VectorMultOperation, 20, L" ╲╱ ", nullptr, true,
     2, VectorProduct},

    {Op::Div, NodeType::DivOperation, 20, L"/",
     [](double lh, double rh) { return lh / rh; }, false, 2},

    {Op::Pow, NodeType::PowOperation, 30, L"^",
     [](double lh, double rh) { return pow(lh, rh); }, false, 2},

    {Op::Sqrt, NodeType::SqrtOperation, 30, L"sqrt", nullptr, false, 2,
     NonTrivialSqrt},

    {Op::Sin, NodeType::SinOperation, 1000, L"sin",
     [](double lh, double rh) { return sin(lh); }, false, 1},

    {Op::Cos, NodeType::CosOperation, 1000, L"cos",
     [](double lh, double rh) { return cos(lh); }, false, 1},

    {Op::Log, NodeType::LogOperation, 1000, L"log", TrivialLogCalc, false, 2},

    {Op::Equal, NodeType::CompareOperation, 5, L" == ",
     [](double lh, double rh) -> double { return (lh == rh) ? 1.0 : 0.0; },
     true, 2, CompareEqual},

    {Op::Diff, NodeType::DiffOperation, 5, L" x/dx ", nullptr, true, 2,
     Differential},
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
