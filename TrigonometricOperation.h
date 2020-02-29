#pragma once

#include "Operation.h"

class TrigonometricOperation : public Operation {
 public:
  TrigonometricOperation(const OpInfo* op_info, std::unique_ptr<INode> lh);

  // INode implementation
  std::unique_ptr<INode> Clone() const override;

  // INodeImpl interface
  NodeType GetNodeType() const override {
    return NodeType::TrigonometricOperation;
  }
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
  void ConvertToComplexImpl(HotToken token,
                            std::unique_ptr<INode>* new_node) override;

  // IOperation implementation
  TrigonometricOperation* AsTrigonometricOperation() override { return this; }
  const TrigonometricOperation* AsTrigonometricOperation() const override {
    return this;
  }

  INodeImpl* Operand() { return Operation::Operand(0); }
  const INodeImpl* Operand() const { return Operation::Operand(0); }
};
