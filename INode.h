#pragma once
#include <memory>
#include <string>
#include <vector>

class INodeImpl;

class INode {
 public:
  virtual ~INode() {}

  virtual bool IsEqual(const INode* rh) const = 0;
  virtual std::unique_ptr<INode> Clone() const = 0;
  virtual std::unique_ptr<INode> SymCalc() const = 0;

  virtual INodeImpl* AsNodeImpl() = 0;
  virtual const INodeImpl* AsNodeImpl() const = 0;
};

