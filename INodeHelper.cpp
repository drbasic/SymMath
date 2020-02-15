#include "INodeHelper.h"

#include <cassert>

#include "Constant.h"
#include "INode.h"
#include "OpInfo.h"
#include "Operation.h"
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
const Operation* INodeHelper::AsOperation(const INode* lh) {
  return lh->AsOperation();
}

// static
Operation* INodeHelper::AsOperation(INode* lh) {
  return lh->AsOperation();
}

// static
Operation* INodeHelper::AsMult(INode* lh) {
  auto result = lh->AsOperation();
  if (result && result->op_info_->op == Op::Mult) {
    return result;
  }
  return nullptr;
}

// static
const Operation* INodeHelper::AsMult(const INode* lh) {
  auto result = lh->AsOperation();
  if (result && result->op_info_->op == Op::Mult) {
    return result;
  }
  return nullptr;
}

// static
std::vector<std::unique_ptr<INode>>& INodeHelper::GetOperands(Operation* op) {
  return op->operands_;
}

// static
bool INodeHelper::IsUnMinus(const INode* lh) {
  if (auto op = AsOperation(lh))
    return op->IsUnMinus();
  return false;
}

// static
Operation* INodeHelper::AsUnMinus(INode* lh) {
  if (auto op = AsOperation(lh))
    return op->IsUnMinus() ? op : nullptr;
  return nullptr;
}

// static
const Operation* INodeHelper::AsDiv(const INode* lh) {
  if (auto op = AsOperation(lh))
    return op->op_info_->op == Op::Div ? op : nullptr;
  return nullptr;
}

// static
Operation* INodeHelper::AsDiv(INode* lh) {
  if (auto op = AsOperation(lh))
    return op->op_info_->op == Op::Div ? op : nullptr;
  return nullptr;
}

// static
std::unique_ptr<Operation> INodeHelper::ConvertToMul(
    std::unique_ptr<INode> rh) {
  assert(rh);
  if (auto* mult = AsMult(rh.get())) {
    rh.release();
    return std::unique_ptr<Operation>(mult);
  }
  return std::make_unique<Operation>(GetOpInfo(Op::Mult), std::move(rh),
                                     Const(1.0));
}

// static
std::unique_ptr<Operation> INodeHelper::MakeUnMinus(
    std::unique_ptr<INode> inner) {
  return std::make_unique<Operation>(GetOpInfo(Op::UnMinus), std::move(inner));
}

// static
std::unique_ptr<Operation> INodeHelper::MakeDiv(std::unique_ptr<INode> lh,
                                                std::unique_ptr<INode> rh) {
  return std::make_unique<Operation>(GetOpInfo(Op::Div), std::move(lh),
                                     std::move(rh));
}
