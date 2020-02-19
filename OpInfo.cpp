﻿#include "OpInfo.h"

#include "Exception.h"
#include "Operations.h"

namespace {
constexpr OpInfo kOps[] = {
    {Op::UnMinus, 11, L"-", [](double lh, double rh) { return -lh; }, false, 1},
    {Op::Minus, 10, L" - ", [](double lh, double rh) { return lh - rh; }, false,
     2},
    {Op::Plus, 10, L" + ", [](double lh, double rh) { return lh + rh; }, true},
    {Op::Mult, 20, L" ∙ ", [](double lh, double rh) { return lh * rh; }, true},
    {Op::Div, 20, L"/", [](double lh, double rh) { return lh / rh; }, false, 2},
    {Op::Pow, 30, L"^", [](double lh, double rh) { return pow(lh, rh); },
     false, 2},
    {Op::Sin, 1000, L"sin", [](double lh, double rh) { return sin(lh); }, false,
     1},
    {Op::Cos, 1000, L"cos", [](double lh, double rh) { return cos(lh); }, false,
     1},
};
}

const OpInfo* GetOpInfo(Op op) {
  if (static_cast<int>(op) >= std::extent<decltype(kOps)>::value)
    throw new Exception("Unknown op");
  auto result = &kOps[static_cast<int>(op)];
  if (result->op != op)
    throw new Exception("Wrong op order");
  return result;
}
