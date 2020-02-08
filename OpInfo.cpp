#include "OpInfo.h"

#include "Exception.h"
#include "Operations.h"

namespace {
constexpr OpInfo kOps[] = {
    {Op::UnMinus, 10, "-", [](double lh, double rh) { return -lh; }},
    {Op::Minus, 10, "-", [](double lh, double rh) { return lh - rh; }},
    {Op::Plus, 10, "+", [](double lh, double rh) { return lh + rh; }},
    {Op::Mult, 20, "*", [](double lh, double rh) { return lh * rh; }},
    {Op::Div, 21, "/", [](double lh, double rh) { return lh / rh; }},
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
