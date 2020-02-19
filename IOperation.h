#pragma once
#include <optional>

#include "INodeImpl.h"

struct OpInfo;
class DivOperation;
class MultOperation;
class PlusOperation;
class UnMinusOperation;

struct CanonicMult {
  double a = 1;
  double b = 1;
  std::vector<std::unique_ptr<INode>*> nodes;
};

class IOperation : public INodeImpl {
 public:
  virtual std::optional<CanonicMult> GetCanonic() = 0;

  virtual void ProcessImaginary(
      std::vector<std::unique_ptr<INode>>* nodes) const {};

  virtual UnMinusOperation* AsUnMinusOperation() { return nullptr; }
  virtual const UnMinusOperation* AsUnMinusOperation() const { return nullptr; }
  virtual PlusOperation* AsPlusOperation() { return nullptr; }
  virtual const PlusOperation* AsPlusOperation() const { return nullptr; }
  virtual MultOperation* AsMultOperation() { return nullptr; }
  virtual const MultOperation* AsMultOperation() const { return nullptr; }
  virtual DivOperation* AsDivOperation() { return nullptr; }
  virtual const DivOperation* AsDivOperation() const { return nullptr; }

  virtual void UnfoldChains() = 0;
  virtual void SimplifyUnMinus(std::unique_ptr<INode>* new_node) = 0;
  virtual void SimplifyChains(std::unique_ptr<INode>* new_node) = 0;
  virtual void SimplifyDivDiv() = 0;
  virtual void SimplifyDivMul(std::unique_ptr<INode>* new_node) = 0;
  virtual void SimplifyConsts(std::unique_ptr<INode>* new_node) = 0;
  virtual void SimplifyTheSame(std::unique_ptr<INode>* new_node) = 0;
  virtual void OpenBrackets(std::unique_ptr<INode>* new_node) = 0;
};
