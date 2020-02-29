#pragma once
#include <memory>
#include <string>
#include <vector>

class INodeImpl;

enum class SymCalcSettings {
  Full,
  KeepNamedConstants,
};

enum class CompareResult {
  Less,
  Equal,
  Greater,
};

class INode {
 public:
  virtual ~INode() {}

  virtual CompareResult Compare(const INode* rh) const = 0;
  virtual std::unique_ptr<INode> Clone() const = 0;
  virtual std::unique_ptr<INode> SymCalc(SymCalcSettings settings) const = 0;

  virtual INodeImpl* AsNodeImpl() = 0;
  virtual const INodeImpl* AsNodeImpl() const = 0;
};

template <typename T>
CompareResult CompareTrivial(T a, T b) {
  return (a == b) ? CompareResult::Equal
                  : (a < b ? CompareResult::Less : CompareResult::Greater);
}