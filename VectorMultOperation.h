#pragma once
#include "Operation.h"

class VectorMultOperation : public Operation {
 public:
  VectorMultOperation(std::unique_ptr<INode> lh, std::unique_ptr<INode> rh);

  // INode implementation
  std::unique_ptr<INode> Clone() const override;

  // INodeImpl interface
  NodeType GetNodeType() const override {
    return NodeType::VectorMultOperation;
  }
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
  bool HasFrontMinus() const override { return false; }
  ValueType GetValueType() const override { return ValueType::Vector; }

  // IOperation implementation
  VectorMultOperation* AsVectorMultOperation() override { return this; }
  const VectorMultOperation* AsVectorMultOperation() const override {
    return this;
  }

 private:
};
