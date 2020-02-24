#include "INodeHelper.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#define _USE_MATH_DEFINES
#include <math.h>

#include "Brackets.h"
#include "CompareOperation.h"
#include "Constant.h"
#include "DiffOperation.h"
#include "DivOperation.h"
#include "ErrorNode.h"
#include "INode.h"
#include "Imaginary.h"
#include "LogOperation.h"
#include "MultOperation.h"
#include "OpInfo.h"
#include "Operation.h"
#include "PlusOperation.h"
#include "PowOperation.h"
#include "TrigonometricOperation.h"
#include "UnMinusOperation.h"
#include "ValueHelpers.h"
#include "VariableRef.h"
#include "Vector.h"
#include "VectorMultOperation.h"

namespace {
Variable kErrorVar("<error variable>");
}

// static
Constant* INodeHelper::AsConstant(INode* lh) {
  return lh->AsNodeImpl()->AsConstant();
}

// static
const Constant* INodeHelper::AsConstant(const INode* lh) {
  return lh->AsNodeImpl()->AsConstant();
}

// static
Operation* INodeHelper::AsOperation(INode* lh) {
  return lh->AsNodeImpl()->AsOperation();
}

// static
const Operation* INodeHelper::AsOperation(const INode* lh) {
  return lh->AsNodeImpl()->AsOperation();
}

// static
MultOperation* INodeHelper::AsMult(INode* lh) {
  auto result = lh->AsNodeImpl()->AsOperation();
  return (result) ? result->AsMultOperation() : nullptr;
}

// static
const MultOperation* INodeHelper::AsMult(const INode* lh) {
  auto result = lh->AsNodeImpl()->AsOperation();
  return (result) ? result->AsMultOperation() : nullptr;
}

// static
bool INodeHelper::IsUnMinus(const INode* lh) {
  auto result = lh->AsNodeImpl()->AsOperation();
  return (result) ? result->AsUnMinusOperation() != nullptr : false;
}

// static
UnMinusOperation* INodeHelper::AsUnMinus(INode* lh) {
  auto result = lh->AsNodeImpl()->AsOperation();
  return (result) ? result->AsUnMinusOperation() : nullptr;
}

// static
const UnMinusOperation* INodeHelper::AsUnMinus(const INode* lh) {
  auto result = lh->AsNodeImpl()->AsOperation();
  return (result) ? result->AsUnMinusOperation() : nullptr;
}

PlusOperation* INodeHelper::AsPlus(INode* lh) {
  auto result = lh->AsNodeImpl()->AsOperation();
  return (result) ? result->AsPlusOperation() : nullptr;
}

// static
const DivOperation* INodeHelper::AsDiv(const INode* lh) {
  auto result = lh->AsNodeImpl()->AsOperation();
  return (result) ? result->AsDivOperation() : nullptr;
}

// static
Variable* INodeHelper::AsVariable(INode* lh) {
  return lh->AsNodeImpl()->AsVariable();
}

// static
const Variable* INodeHelper::AsVariable(const INode* lh) {
  return lh->AsNodeImpl()->AsVariable();
}

// static
PowOperation* INodeHelper::AsPow(INode* lh) {
  auto result = lh->AsNodeImpl()->AsOperation();
  return (result) ? result->AsPowOperation() : nullptr;
}

// static
const PowOperation* INodeHelper::AsPow(const INode* lh) {
  auto result = lh->AsNodeImpl()->AsOperation();
  return (result) ? result->AsPowOperation() : nullptr;
}

// static
CanonicMult INodeHelper::GetCanonicMult(std::unique_ptr<INode>& node) {
  if (auto* operation = AsOperation(node.get())) {
    auto inner_canonic = operation->GetCanonicMult();
    if (inner_canonic)
      return *inner_canonic;
  }
  CanonicMult result;
  result.nodes.push_back(&node);
  return result;
}

// static
CanonicPow INodeHelper::GetCanonicPow(std::unique_ptr<INode>& node) {
  if (auto* operation = AsOperation(node.get())) {
    auto inner_canonic = operation->GetCanonicPow();
    if (inner_canonic)
      return *inner_canonic;
  }
  CanonicPow result;
  result.Add(1, &node);
  return result;
}

// static
CanonicMult INodeHelper::MergeCanonic(const CanonicMult& lh,
                                      const CanonicMult& rh) {
  CanonicMult result;
  result.a = lh.a * rh.a;
  result.b = lh.b * rh.b;
  result.nodes.reserve(lh.nodes.size() + rh.nodes.size());
  for (auto node : lh.nodes)
    result.nodes.push_back(node);
  for (auto node : rh.nodes)
    result.nodes.push_back(node);
  return result;
}

// static
std::unique_ptr<INode> INodeHelper::Negate(std::unique_ptr<INode> node) {
  if (auto* un_minus = AsUnMinus(node.get())) {
    return un_minus->TakeOperand(0);
  }
  return MakeUnMinus(std::move(node));
}

// static
std::unique_ptr<INode> INodeHelper::MakeMultIfNeeded(
    std::vector<std::unique_ptr<INode>> nodes) {
  if (nodes.size() == 0)
    return MakeConst(1.0);
  if (nodes.size() == 1)
    return std::move(nodes[0]);
  return MakeMult(std::move(nodes));
}

// static
std::unique_ptr<VectorMultOperation> INodeHelper::MakeVectorMult(
    std::unique_ptr<INode> lh,
    std::unique_ptr<INode> rh) {
  return std::make_unique<VectorMultOperation>(std::move(lh), std::move(rh));
}

// static
DivOperation* INodeHelper::AsDiv(INode* lh) {
  auto result = lh->AsNodeImpl()->AsOperation();
  return (result) ? result->AsDivOperation() : nullptr;
}

// static
std::unique_ptr<MultOperation> INodeHelper::ConvertToMul(
    std::unique_ptr<INode> rh) {
  assert(rh);
  if (auto* mult = AsMult(rh.get())) {
    rh.release();
    return std::unique_ptr<MultOperation>(mult);
  }
  return std::make_unique<MultOperation>(std::move(rh), Const(1.0));
}

void INodeHelper::RemoveEmptyOperands(
    std::vector<std::unique_ptr<INode>>* nodes) {
  nodes->erase(
      std::remove_if(std::begin(*nodes), std::end(*nodes),
                     [](const std::unique_ptr<INode>& node) { return !node; }),
      std::end(*nodes));
}

// static
bool INodeHelper::HasAnyPlusOperation(
    const std::vector<std::unique_ptr<INode>>& nodes) {
  for (const auto& node : nodes) {
    if (const auto* plus_op = AsPlus(node.get()))
      return true;
  }
  return false;
}

// static
bool INodeHelper::HasAnyValueType(
    const std::vector<std::unique_ptr<INode>>& operands,
    ValueType value_type) {
  for (auto& operand : operands) {
    if (operand->AsNodeImpl()->GetValueType() == value_type)
      return true;
  }
  return false;
}

// static
bool INodeHelper::HasAllValueType(
    const std::vector<std::unique_ptr<INode>>& operands,
    ValueType value_type) {
  for (auto& operand : operands) {
    if (operand->AsNodeImpl()->GetValueType() != value_type)
      return false;
  }
  return true;
}

// static
std::unique_ptr<Operation> INodeHelper::MakeEmpty(Op op) {
  switch (op) {
    case Op::UnMinus:
      return MakeUnMinus(MakeError());
      break;
    case Op::Minus:
      assert(false);
      break;
    case Op::Plus:
      return MakePlus(MakeError(), MakeError());
      break;
    case Op::Mult:
      return MakeMult(MakeError(), MakeError());
      break;
    case Op::VectorMult:
      return MakeVectorMult(MakeError(), MakeError());
      break;
    case Op::Div:
      return MakeDiv(MakeError(), MakeError());
      break;
    case Op::Pow:
      return MakePow(MakeError(), MakeError());
      break;
    case Op::Sin:
    case Op::Cos:
      return MakeTrigonometric(op, MakeError());
      break;
    case Op::Ln:
      return MakeLn(MakeError());
      break;
    case Op::Equal:
      return MakeCompare(op, MakeError(), MakeError());
      break;
    case Op::Diff:
      return MakeDiff(MakeError(), kErrorVar);
      break;
  }
  assert(false);
  return nullptr;
}

std::unique_ptr<INode> INodeHelper::MakeError() {
  return std::make_unique<ErrorNode>(std::string());
}

std::unique_ptr<INode> INodeHelper::MakeError(std::string err) {
  return std::make_unique<ErrorNode>(std::move(err));
}

// static
std::unique_ptr<Constant> INodeHelper::MakeConst(double value) {
  return std::make_unique<Constant>(std::move(value));
}

// static
std::unique_ptr<Constant> INodeHelper::MakeConst(bool value) {
  return std::make_unique<Constant>(value);
}

// static
std::unique_ptr<Imaginary> INodeHelper::MakeImaginary() {
  return std::make_unique<Imaginary>();
}

// static
std::unique_ptr<UnMinusOperation> INodeHelper::MakeUnMinus(
    std::unique_ptr<INode> value) {
  return std::make_unique<UnMinusOperation>(std::move(value));
}

// static
std::unique_ptr<PlusOperation> INodeHelper::MakeMinus(
    std::unique_ptr<INode> lh,
    std::unique_ptr<INode> rh) {
  return std::make_unique<PlusOperation>(std::move(lh),
                                         MakeUnMinus(std::move(rh)));
}

// static
std::unique_ptr<PlusOperation> INodeHelper::MakePlus(
    std::unique_ptr<INode> lh,
    std::unique_ptr<INode> rh) {
  auto* lh_plus = AsPlus(lh.get());
  auto* rh_plus = AsPlus(rh.get());
  if (lh_plus && rh_plus) {
    auto lh_operands = lh_plus->TakeAllOperands();
    auto rh_operands = rh_plus->TakeAllOperands();
    lh_operands.reserve(lh_operands.size() + rh_operands.size());
    for (auto& node : rh_operands)
      lh_operands.push_back(std::move(node));
    return MakePlus(std::move(lh_operands));
  }
  if (lh_plus) {
    auto lh_operands = lh_plus->TakeAllOperands();
    lh_operands.push_back(std::move(rh));
    return MakePlus(std::move(lh_operands));
  }
  if (rh_plus) {
    auto rh_operands = rh_plus->TakeAllOperands();
    rh_operands.insert(rh_operands.begin(), std::move(lh));
    return MakePlus(std::move(rh_operands));
  }
  return std::make_unique<PlusOperation>(std::move(lh), std::move(rh));
}

// static
std::unique_ptr<PlusOperation> INodeHelper::MakePlus(
    std::vector<std::unique_ptr<INode>> operands) {
  return std::make_unique<PlusOperation>(std::move(operands));
}

// static
std::unique_ptr<INode> INodeHelper::MakePlusIfNeeded(
    std::vector<std::unique_ptr<INode>> nodes) {
  assert(nodes.size() > 0);
  if (nodes.size() == 1)
    return std::move(nodes[0]);

  return MakePlus(std::move(nodes));
}

// static
std::unique_ptr<MultOperation> INodeHelper::MakeMult(
    std::unique_ptr<INode> lh,
    std::unique_ptr<INode> rh) {
  return std::make_unique<MultOperation>(std::move(lh), std::move(rh));
}

// static
std::unique_ptr<MultOperation> INodeHelper::MakeMult(
    std::vector<std::unique_ptr<INode>> operands) {
  return std::make_unique<MultOperation>(std::move(operands));
}

// static
std::unique_ptr<INode> INodeHelper::MakeMult(
    double dividend,
    double divider,
    std::vector<std::unique_ptr<INode>*> nodes) {
  if (dividend == divider && nodes.size() == 1) {
    return std::move(*nodes[0]);
  }
  if (dividend == divider) {
    std::vector<std::unique_ptr<INode>> operands;
    for (auto& node : nodes) {
      operands.push_back(std::move(*node));
    }
    return INodeHelper::MakeMult(std::move(operands));
  }

  std::unique_ptr<INode> result;
  if (dividend != 1.0 || nodes.size() > 1) {
    std::vector<std::unique_ptr<INode>> operands;
    operands.reserve(nodes.size() + (dividend != 1.0 ? 1 : 0));
    if (dividend != 1.0)
      operands.push_back(INodeHelper::MakeConst(dividend));
    for (auto& node : nodes) {
      operands.push_back(std::move(*node));
    }
    result = INodeHelper::MakeMult(std::move(operands));
  } else {
    result = std::move(*nodes[0]);
  }
  if (divider == 1.0)
    return result;
  result =
      INodeHelper::MakeDiv(std::move(result), INodeHelper::MakeConst(divider));
  return result;
}

// static
std::unique_ptr<DivOperation> INodeHelper::MakeDiv(std::unique_ptr<INode> lh,
                                                   std::unique_ptr<INode> rh) {
  return std::make_unique<DivOperation>(std::move(lh), std::move(rh));
}

// static
std::unique_ptr<PowOperation> INodeHelper::MakePow(std::unique_ptr<INode> base,
                                                   std::unique_ptr<INode> exp) {
  return std::make_unique<PowOperation>(std::move(base), std::move(exp));
}

std::unique_ptr<INode> INodeHelper::MakePowIfNeeded(std::unique_ptr<INode> base,
                                                    double exp) {
  if (exp == 0.0)
    return nullptr;
  if (exp == 1.0)
    return base;
  return MakePow(std::move(base), INodeHelper::MakeConst(exp));
}

// static
std::unique_ptr<TrigonometricOperation> INodeHelper::MakeTrigonometric(
    Op op,
    std::unique_ptr<INode> value) {
  return std::make_unique<TrigonometricOperation>(GetOpInfo(op),
                                                  std::move(value));
}

// static
std::unique_ptr<LogOperation> INodeHelper::MakeLn(
    std::unique_ptr<INode> value) {
  return std::make_unique<LogOperation>(Const(M_E), std::move(value));
}

// static
std::unique_ptr<LogOperation> INodeHelper::MakeLog2(
    std::unique_ptr<INode> value) {
  return std::make_unique<LogOperation>(Const(2.0), std::move(value));
}

// static
std::unique_ptr<LogOperation> INodeHelper::MakeLog10(
    std::unique_ptr<INode> value) {
  return std::make_unique<LogOperation>(Const(10.0), std::move(value));
}

// static
std::unique_ptr<LogOperation> INodeHelper::MakeLog(
    std::unique_ptr<INode> base,
    std::unique_ptr<INode> value) {
  return std::make_unique<LogOperation>(std::move(base), std::move(value));
}

// static
std::unique_ptr<CompareOperation> INodeHelper::MakeCompare(
    Op op,
    std::unique_ptr<INode> lh,
    std::unique_ptr<INode> rh) {
  return std::make_unique<CompareOperation>(op, std::move(lh), std::move(rh));
}

std::unique_ptr<Brackets> INodeHelper::MakeBrackets(
    BracketType bracket_type,
    std::unique_ptr<INode> value) {
  return std::make_unique<Brackets>(bracket_type, std::move(value));
}

std::unique_ptr<Vector> INodeHelper::MakeVector(std::unique_ptr<INode> a,
                                                std::unique_ptr<INode> b) {
  return std::make_unique<Vector>(std::move(a), std::move(b));
}

std::unique_ptr<Vector> INodeHelper::MakeVector(std::unique_ptr<INode> a,
                                                std::unique_ptr<INode> b,
                                                std::unique_ptr<INode> c) {
  return std::make_unique<Vector>(std::move(a), std::move(b), std::move(c));
}

std::unique_ptr<Vector> INodeHelper::MakeVector(
    std::vector<std::unique_ptr<INode>> values) {
  return std::make_unique<Vector>(std::move(values));
}

std::unique_ptr<DiffOperation> INodeHelper::MakeDiff(std::unique_ptr<INode> lh,
                                                     const Variable& var) {
  return std::make_unique<DiffOperation>(std::move(lh),
                                         std::make_unique<VariableRef>(&var));
}
