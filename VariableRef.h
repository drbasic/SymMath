#pragma once

#include "INode.h"

class VariableRef : public INode {
 public:
  explicit VariableRef(const Variable* var);

  std::unique_ptr<INode> SymCalc() const override;

 protected:
  bool IsEqual(const INode* rh) const override;
  std::unique_ptr<INode> Clone() const override;
  PrintSize Render(Canvas* canvas,
                   const Position& pos,
                   bool dry_run,
                   bool ommit_front_minus) const override;
  PrintSize LastPrintSize() const override;
  int Priority() const override;
  bool HasFrontMinus() const override;
  bool CheckCircular(const INode* other) const override;

  Constant* AsConstant() override;
  const Constant* AsConstant() const override;
  const ErrorNode* AsError() const override;
  const Variable* AsVariable() const override;
  Operation* AsOperation() override;
  const Operation* AsOperation() const override;
  bool SimplifyImpl(std::unique_ptr<INode>* new_node) override;

 private:
  const Variable* var_ = nullptr;
};
