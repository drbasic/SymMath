#pragma once

#include "INode.h"

class Variable : public INode {
 public:
  Variable(std::string name);
  Variable(std::unique_ptr<INode> value);
  Variable(const Variable& var);

  std::unique_ptr<INode> SymCalc() const override;

  bool Simplify();

  std::string GetName() const;
  void operator=(std::unique_ptr<INode> value);
  void operator=(const Variable& var);
  void operator=(double val);
  operator std::unique_ptr<INode>() const;

 protected:
  std::string PrintImpl(bool ommit_front_minus) const override;
  int Priority() const override;
  bool HasFrontMinus() const override;
  bool CheckCircular(const INode* other) const override;
  bool IsUnMinus() const override;
  Operation* AsUnMinus() override;
  Constant* AsConstant() override;
  const Variable* AsVariable() const override;
  std::vector<std::unique_ptr<INode>> TakeOperands(Op op) override;

  bool SimplifyImpl(std::unique_ptr<INode>* new_node) override;

 private:
  friend class VariableRef;
  INode* GetVisibleNode() const;
  std::string PrintRef(bool ommit_front_minus) const;
  std::string name_;
  std::unique_ptr<INode> value_;
};
