#include "INodeHelper.h"

#include <cassert>

#include "Constant.h"
#include "DivOperation.h"
#include "INode.h"
#include "MultOperation.h"
#include "OpInfo.h"
#include "Operation.h"
#include "UnMinusOperation.h"
#include "ValueHelpers.h"

// static
bool INodeHelper::IsNodesEqual(const INode* lh, const INode* rh) {
  return lh->IsEqual(rh);
}

// static
const Constant* INodeHelper::AsConstant(const INode* lh) {
  return lh->AsConstant();
}

// static
Operation* INodeHelper::AsOperation(INode* lh) {
  return lh->AsOperation();
}

// static
const Operation* INodeHelper::AsOperation(const INode* lh) {
  return lh->AsOperation();
}

// static
MultOperation* INodeHelper::AsMult(INode* lh) {
  auto result = lh->AsOperation();
  return (result) ? result->AsMultOperation() : nullptr;
}

// static
const MultOperation* INodeHelper::AsMult(const INode* lh) {
  auto result = lh->AsOperation();
  return (result) ? result->AsMultOperation() : nullptr;
}

// static
std::vector<std::unique_ptr<INode>>& INodeHelper::GetOperands(Operation* op) {
  return op->operands_;
}

// static
bool INodeHelper::IsUnMinus(const INode* lh) {
  auto result = lh->AsOperation();
  return (result) ? result->AsUnMinusOperation() != nullptr : false;
}

// static
UnMinusOperation* INodeHelper::AsUnMinus(INode* lh) {
  auto result = lh->AsOperation();
  return (result) ? result->AsUnMinusOperation() : nullptr;
}

// static
const UnMinusOperation* INodeHelper::AsUnMinus(const INode* lh) {
  auto result = lh->AsOperation();
  return (result) ? result->AsUnMinusOperation() : nullptr;
}

// static
const DivOperation* INodeHelper::AsDiv(const INode* lh) {
  auto result = lh->AsOperation();
  return (result) ? result->AsDivOperation() : nullptr;
}

// static
DivOperation* INodeHelper::AsDiv(INode* lh) {
  auto result = lh->AsOperation();
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

// static
std::unique_ptr<Constant> INodeHelper::MakeConst(double value)
{
  return std::make_unique<Constant>(std::move(value));
}

// static
std::unique_ptr<UnMinusOperation> INodeHelper::MakeUnMinus(
    std::unique_ptr<INode> value) {
  return std::make_unique<UnMinusOperation>(std::move(value));
}

// static
std::unique_ptr<DivOperation> INodeHelper::MakeDiv(std::unique_ptr<INode> lh,
                                                   std::unique_ptr<INode> rh) {
  return std::make_unique<DivOperation>(std::move(lh), std::move(rh));
}
