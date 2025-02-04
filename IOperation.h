#pragma once
#include <optional>

#include "INodeImpl.h"

struct OpInfo;
class DiffOperation;
class DivOperation;
class LogOperation;
class MultOperation;
class VectorMultOperation;
class PlusOperation;
class PowOperation;
class SqrtOperation;
class CompareOperation;
class TrigonometricOperation;
class UnMinusOperation;

struct CanonicMult {
  double a = 1;
  double b = 1;
  std::vector<std::unique_ptr<INode>*> nodes;
};

struct CanonicPow {
  void Merge(CanonicPow rh);
  void Add(double exp_up, double exp_down, std::unique_ptr<INode>* node);

  struct NodeInfo {
    void Apply(double e_up, double e_down);

    double exp_up = 1;
    double exp_down = 1;
    std::unique_ptr<INode>* node = nullptr;
  };
  std::vector<NodeInfo> base_nodes;
};

class IOperation : public INodeImpl {
 public:
  virtual std::optional<CanonicMult> GetCanonicMult() = 0;
  virtual std::optional<CanonicPow> GetCanonicPow() = 0;

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
  virtual SqrtOperation* AsSqrtOperation() { return nullptr; }
  virtual const SqrtOperation* AsSqrtOperation() const { return nullptr; }
  virtual CompareOperation* AsCompareOperation() { return nullptr; }
  virtual const CompareOperation* AsCompareOperation() const { return nullptr; }
  virtual LogOperation* AsLogOperation() { return nullptr; }
  virtual const LogOperation* AsLogOperation() const { return nullptr; }
  virtual TrigonometricOperation* AsTrigonometricOperation() { return nullptr; }
  virtual const TrigonometricOperation* AsTrigonometricOperation() const {
    return nullptr;
  }
  virtual DiffOperation* AsDiffOperation() { return nullptr; }
  virtual const DiffOperation* AsDiffOperation() const { return nullptr; }

  virtual void UnfoldChains(HotToken token) = 0;
  virtual void SimplifyUnMinus(HotToken token,
                               std::unique_ptr<INode>* new_node) = 0;
  virtual void SimplifyChains(HotToken token,
                              std::unique_ptr<INode>* new_node) = 0;
  virtual void SimplifyDivDiv(HotToken token) = 0;
  virtual void SimplifyDivMul(HotToken token,
                              std::unique_ptr<INode>* new_node) = 0;
  virtual void SimplifyConsts(HotToken token,
                              std::unique_ptr<INode>* new_node) = 0;
  virtual void SimplifyTheSame(HotToken token,
                               std::unique_ptr<INode>* new_node) = 0;
  virtual void OrderOperands(HotToken token) = 0;
};
