#include "Tests.h"

#include <iostream>
#include <string_view>

#include "DivOperation.h"
#include "INode.h"
#include "INodeHelper.h"
#include "Operation.h"
#include "PlusOperation.h"
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
    {&Tests::TestSimplifyDivDiv, "TestSimplifyDivDiv"},
    {&Tests::TestSimplifyImaginary, "TestSimplifyImaginary"},
    {&Tests::TestOpenBrackets, "TestOpenBrackets"},
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
bool Tests::TestSimplifyMultChain() {
  auto a = Var(L"a", 1);
  auto b = Var(L"b", 2);
  auto c = Var(L"c", 3);
  auto d = Var(L"d", 4);
  auto e = Var(L"e", 5);
  Variable s = a * b * (-c) * (-d) * (-e);
  auto* op = s.AsOperation();
  op->UnfoldChains({});
  if (op->operands_.size() != 5)
    return false;
  auto expected_result = Const(-120);
  auto result = s.SymCalc(SymCalcSettings::Full);
  if (result->Compare(expected_result.get()) != CompareResult::Equal)
    return false;
  return true;
}

// static
bool Tests::TestSimplifyPlusChain() {
  auto a = Var(L"a", 1);
  auto b = Var(L"b", 2);
  auto c = Var(L"c", 3);
  auto d = Var(L"d", 4);
  auto e = Var(L"e", 5);
  Variable s = a + b - c - d - e;
  auto* op = s.AsOperation();
  op->UnfoldChains({});
  if (op->operands_.size() != 5)
    return false;
  auto expected_result = Const(-9);
  if (s.SymCalc(SymCalcSettings::Full)->Compare(expected_result.get()) !=
      CompareResult::Equal)
    return false;
  return true;
}

// static
bool Tests::TestSimplifyChainRecursive() {
  auto a = Var(L"a", 1);
  auto b = Var(L"b", 2);
  auto c = Var(L"c", 3);
  auto d = Var(L"d", 4);
  auto e = Var(L"e", 5);
  Variable s = (a + b + c) * (a + b + c) * (a + b + c);
  auto* op = s.AsOperation();
  op->UnfoldChains({});
  if (op->operands_.size() != 3)
    return false;
  for (auto& sub : op->operands_) {
    auto* sub_op = sub->AsNodeImpl()->AsOperation();
    if (sub_op->operands_.size() != 3)
      return false;
  }
  auto expected_result = Const(216);
  if (s.SymCalc(SymCalcSettings::Full)->Compare(expected_result.get()) !=
      CompareResult::Equal)
    return false;
  return true;
}

// static
bool Tests::TestSimplifyDivDiv() {
  auto a = Var(L"a", 240);
  auto b = Var(L"b", 2);
  auto c = Var(L"c", 3);
  auto d = Var(L"d", 4);
  auto e = Var(L"e", 5);
  Variable s = (a / b) / c / d / e;
  auto* op = s.AsOperation();
  op->SimplifyDivDiv({});
  auto* div = INodeHelper::AsDiv(op);
  if (!div)
    return false;
  if (!div->Dividend()->AsVariable())
    return false;
  if (div->Divider()->AsOperation()->operands_.size() != 4)
    return false;
  auto expected_result = Const(2);
  auto result = s.SymCalc(SymCalcSettings::Full);
  if (result->Compare(expected_result.get()) != CompareResult::Equal)
    return false;
  return true;
}

// static
bool Tests::TestSimplifyImaginary() {
  auto a = Var(L"a", 100);
  auto b = Var(L"b", 2);
  Variable s = Imag() * Imag() + a;
  auto expected_result = Const(99);
  auto result = s.SymCalc(SymCalcSettings::Full);
  if (result->Compare(expected_result.get()) != CompareResult::Equal)
    return false;
  return true;
}

// static
bool Tests::TestOpenBrackets() {
  auto a = Var(L"a", 1);
  auto b = Var(L"b", 2);
  auto c = Var(L"c", 3);
  auto d = Var(L"d", 4);
  auto e = Var(L"e", 5);
  Variable s = (a + b) * (b + c + d) * (a + b) * e;
  std::unique_ptr<INode> new_node;
  s.AsOperation()->SimplifyChains({}, &new_node);
  s.OpenBrackets();
  auto* plus = INodeHelper::AsPlus(s.AsOperation());
  if (!plus)
    return false;
  if (plus->operands_.size() != 12)
    return false;
  auto expected_result = Const(405);
  auto result = s.SymCalc(SymCalcSettings::Full);
  if (result->Compare(expected_result.get()) != CompareResult::Equal)
    return false;
  return true;
}