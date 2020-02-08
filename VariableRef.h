#pragma once

#include "INode.h"

class VariableRef : public INode {
 public:
  explicit VariableRef(const Variable* var);

  std::string PrintImpl(bool ommit_front_minus) const override;

 protected:
  int Priority() const override;
  bool NeedBrackets() const override;
  bool HasFrontMinus() const override;
  bool CheckCircular(const INode* other) const override;
  std::unique_ptr<INode> SymCalc() const override;
  INode* GetVisibleNode() override;
  const INode* GetVisibleNode() const override;
  bool IsUnMinus() const override;
  bool SimplifyImpl(std::unique_ptr<INode>* new_node) override;

  std::string GetName() const;

 private:
  const Variable* var_ = nullptr;
};
