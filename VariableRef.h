#pragma once

#include "INode.h"
#include "INodeImpl.h"

class VariableRef : public INodeImpl {
 public:
  explicit VariableRef(const Variable* var);

  // INode interface
  bool IsEqual(const INode* rh) const override;
  std::unique_ptr<INode> Clone() const override;
  std::unique_ptr<INode> SymCalc() const override;

  // INodeImpl interface
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
  PrintSize LastPrintSize() const override;
  int Priority() const override;
  bool HasFrontMinus() const override;
  bool CheckCircular(const INodeImpl* other) const override;

  Constant* AsConstant() override;
  const Constant* AsConstant() const override;
  const ErrorNode* AsError() const override;
  Variable* AsVariable() override;
  const Variable* AsVariable() const override;
  Operation* AsOperation() override;
  const Operation* AsOperation() const override;
  void SimplifyImpl(std::unique_ptr<INode>* new_node) override;

 private:
  mutable PrintSize print_size_;
  const Variable* var_ = nullptr;
};
