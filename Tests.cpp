#include "Tests.h"

#include <iostream>
#include <string_view>

#include "INode.h"
#include "INodeHelper.h"
#include "Operation.h"
#include "ValueHelpers.h"

namespace {
using TestF = bool (*)();
struct TestInfo {
  TestF test_f;
  std::string_view Name;
};
const TestInfo kTests[] = {
    {&Tests::TestSimplifyPlusChain, "TestSimplifyPlusChain"},
    {&Tests::TestSimplifyMultChain, "TestSimplifyMultChain"},
    {&Tests::TestSimplifyChainRecursive, "TestSimplifyChainRecursive"},
};
}  // namespace

void Tests::Run() {
  for (auto test : kTests) {
    bool succ = test.test_f();
    std::string response(test.Name);
    if (succ)
      response += " OK";
    else
      response += " Failed";
    std::wcout << std::wstring(response.begin(), response.end()) << std::endl;
  }
}

// static
bool Tests::TestSimplifyPlusChain() {
  auto a = Var("a", 1);
  auto b = Var("b", 2);
  auto c = Var("c", 3);
  auto d = Var("d", 4);
  auto e = Var("e", 5);
  Variable s = a * b * (-c) * (-d) * (-e);
  auto* op = s.AsOperation();
  op->SimplifyChain();
  if (op->operands_.size() != 5)
    return false;
  auto expected_result = Const(-120);
  auto result = s.SymCalc();
  if (!result->IsEqual(expected_result.get()))
    return false;
  return true;
}

// static
bool Tests::TestSimplifyMultChain() {
  auto a = Var("a", 1);
  auto b = Var("b", 2);
  auto c = Var("c", 3);
  auto d = Var("d", 4);
  auto e = Var("e", 5);
  Variable s = a + b - c - d - e;
  auto* op = s.AsOperation();
  op->SimplifyChain();
  if (op->operands_.size() != 5)
    return false;
  auto expected_result = Const(-9);
  if (!s.SymCalc()->IsEqual(expected_result.get()))
    return false;
  return true;
}

// static
bool Tests::TestSimplifyChainRecursive() {
  auto a = Var("a", 1);
  auto b = Var("b", 2);
  auto c = Var("c", 3);
  auto d = Var("d", 4);
  auto e = Var("e", 5);
  Variable s = (a + b + c) * (a + b + c) * (a + b + c);
  auto* op = s.AsOperation();
  op->SimplifyChain();
  if (op->operands_.size() != 3)
    return false;
  for (auto& sub : op->operands_) {
    auto* sub_op = sub->AsNodeImpl()->AsOperation();
    if (sub_op->operands_.size() != 3)
      return false;
  }
  auto expected_result = Const(216);
  if (!s.SymCalc()->IsEqual(expected_result.get()))
    return false;
  return true;
}
