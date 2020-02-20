#include "INodeHelper.h"

#include <algorithm>
#include <cassert>

#include "Constant.h"
#include "DivOperation.h"
#include "INode.h"
#include "Imaginary.h"
#include "MultOperation.h"
#include "OpInfo.h"
#include "Operation.h"
#include "PlusOperation.h"
#include "PowOperation.h"
#include "TrigonometricOperation.h"
#include "UnMinusOperation.h"
#include "ValueHelpers.h"

// static
bool INodeHelper::IsNodesEqual(const INode* lh, const INode* rh) {
  return lh->IsEqual(rh);
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
std::vector<std::unique_ptr<INode>>& INodeHelper::GetOperands(Operation* op) {
  return op->operands_;
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
CanonicMult INodeHelper::GetCanonic(std::unique_ptr<INode>& node) {
  if (auto* operation = AsOperation(node.get())) {
    auto inner_canonic = operation->GetCanonic();
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

void INodeHelper::ExctractNodesWithOp(
    Op op,
    std::vector<std::unique_ptr<INode>>* src,
    std::vector<std::unique_ptr<INode>>* nodes) {
  for (auto& node : *src) {
    ExctractNodesWithOp(op, std::move(node), nodes);
  }
}

void INodeHelper::ExctractNodesWithOp(
    Op op,
    std::unique_ptr<INode> src,
    std::vector<std::unique_ptr<INode>>* nodes) {
  std::vector<std::unique_ptr<INode>> positive_nodes;
  std::vector<std::unique_ptr<INode>> negative_nodes;
  INodeHelper::ExctractNodesWithOp(op, std::move(src), &positive_nodes,
                                   &negative_nodes);
  nodes->reserve(nodes->size() + positive_nodes.size() + negative_nodes.size());
  for (auto& node : positive_nodes) {
    nodes->push_back(std::move(node));
  }
  for (auto& node : negative_nodes) {
    nodes->push_back(INodeHelper::Negate(std::move(node)));
  }
}

// static
void INodeHelper::ExctractNodesWithOp(
    Op op,
    std::unique_ptr<INode> src,
    std::vector<std::unique_ptr<INode>>* positive_nodes,
    std::vector<std::unique_ptr<INode>>* negative_nodes) {
  Operation* operation = AsOperation(src.get());
  if (!operation) {
    positive_nodes->push_back(std::move(src));
    return;
  }
  if (operation->op_info_->op == Op::UnMinus) {
    ExctractNodesWithOp(op, std::move(operation->operands_[0]), negative_nodes,
                        positive_nodes);
    return;
  }
  if (operation->op_info_->op != op) {
    positive_nodes->push_back(std::move(src));
    return;
  }
  for (auto& node : operation->operands_) {
    ExctractNodesWithOp(op, std::move(node), positive_nodes, negative_nodes);
  }
}

// static
std::unique_ptr<INode> INodeHelper::Negate(std::unique_ptr<INode> node) {
  if (auto* un_minus = AsUnMinus(node.get())) {
    return std::move(un_minus->operands_[0]);
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
std::unique_ptr<Constant> INodeHelper::MakeConst(double value) {
  return std::make_unique<Constant>(std::move(value));
}

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
  return std::make_unique<PlusOperation>(std::move(lh), std::move(rh));
}

// static
std::unique_ptr<PlusOperation> INodeHelper::MakePlus(
    std::vector<std::unique_ptr<INode>> operands) {
  return std::make_unique<PlusOperation>(std::move(operands));
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
