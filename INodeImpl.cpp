#include "INodeImpl.h"

CompareResult INodeImpl::CompareType(const INode* rh) const {
  int a = static_cast<int>(GetNodeType());
  int b = static_cast<int>(rh->AsNodeImpl()->GetNodeType());
  return CompareTrivial(a, b);
}