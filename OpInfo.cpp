#include "OpInfo.h"

#include "Exception.h"
#include "Operations.h"

namespace {
constexpr OpInfo kOps[] = {
    {Op::UnMinus, 11, "un(-)", [](double lh, double rh) { return -lh; }, false},
    {Op::Minus, 10, " - ", [](double lh, double rh) { return lh - rh; }, false},
    {Op::Plus, 10, " + ", [](double lh, double rh) { return lh + rh; }, true},
    {Op::Mult, 20, " * ", [](double lh, double rh) { return lh * rh; }, true},
    {Op::Div, 20, "/", [](double lh, double rh) { return lh / rh; }, false},
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
