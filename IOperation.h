#pragma once
#include <optional>

#include "INodeImpl.h"

struct OpInfo;
class DivOperation;
class MultOperation;
class VectorMultOperation;
class PlusOperation;
class PowOperation;
class CompareOperation;
class UnMinusOperation;

struct CanonicMult {
  double a = 1;
  double b = 1;
  std::vector<std::unique_ptr<INode>*> nodes;
};

struct CanonicPow {
  void Merge(CanonicPow rh);
  void Add(double exp, std::unique_ptr<INode>* node);

  struct NodeInfo {
    double exp = 1;
    std::unique_ptr<INode>* node = nullptr;
  };
  std::vector<NodeInfo> base_nodes;
};

class IOperation : public INodeImpl {
 public:
  virtual std::optional<CanonicMult> GetCanonicMult() = 0;
  virtual std::optional<CanonicPow> GetCanonicPow() = 0;

  virtual void ProcessImaginary(
      std::vector<std::unique_ptr<INode>>* nodes) const {}

  virtual UnMinusOperation* AsUnMinusOperation() { return nullptr; }
  virtual const UnMinusOperation* AsUnMinusOperation() const { return nullptr; }
  virtual PlusOperation* AsPlusOperation() { return nullptr; }
  virtual const PlusOperation* AsPlusOperation() const { return nullptr; }
  virtual MultOperation* AsMultOperation() { return nullptr; }
  virtual const MultOperation* AsMultOperation() const { return nullptr; }
  virtual VectorMultOperation* AsVectorMultOperation() { return nullptr; }
  virtual const VectorMultOperation* AsVectorMultOperation() const {
    return nullptr;
  }
  virtual DivOperation* AsDivOperation() { return nullptr; }
  virtual const DivOperation* AsDivOperation() const { return nullptr; }
  virtual PowOperation* AsPowOperation() { return nullptr; }
  virtual const PowOperation* AsPowOperation() const { return nullptr; }
  virtual CompareOperation* AsCompareOperation() { return nullptr; }
  virtual const CompareOperation* AsCompareOperation() const { return nullptr; }

  virtual void UnfoldChains() = 0;
  virtual void SimplifyUnMinus(std::unique_ptr<INode>* new_node) = 0;
  virtual void SimplifyChains(std::unique_ptr<INode>* new_node) = 0;
  virtual void SimplifyDivDiv() = 0;
  virtual void SimplifyDivMul(std::unique_ptr<INode>* new_node) = 0;
  virtual void SimplifyConsts(std::unique_ptr<INode>* new_node) = 0;
  virtual void SimplifyTheSame(std::unique_ptr<INode>* new_node) = 0;
  virtual void OrderOperands() = 0;
};
