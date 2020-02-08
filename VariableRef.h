#pragma once

#include "INode.h"

class VariableRef : public INode {
 public:
  explicit VariableRef(const Variable* var);

  std::string PrintImpl(bool ommit_front_minus) const override;

 protected:
  int Priority() const override;
  bool HasFrontMinus() const override;
  bool CheckCircular(const INode* other) const override;
  std::unique_ptr<INode> SymCalc() const override;
  bool IsUnMinus() const override;
  Operation* AsUnMinus() override;
  Constant* AsConstant() override;
  const Variable* AsVariable() const override;
  std::vector<std::unique_ptr<INode>> TakeOperands(Op op) override;
  bool SimplifyImpl(std::unique_ptr<INode>* new_node) override;

 private:
  const Variable* var_ = nullptr;
};
