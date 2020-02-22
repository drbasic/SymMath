#pragma once

#include "Operation.h"

std::unique_ptr<INode> CompareEqual(
    const OpInfo* op,
    std::vector<std::unique_ptr<INode>>* operands);

class CompareOperation : public Operation {
 public:
  CompareOperation(Op op, std::unique_ptr<INode> lh, std::unique_ptr<INode> rh);

  // INode implementation
  std::unique_ptr<INode> Clone() const override;

  // INodeImpl interface
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
  bool HasFrontMinus() const override { return false; }

  // IOperation implementation
  CompareOperation* AsCompareOperation() override { return this; }
  const CompareOperation* AsCompareOperation() const override { return this; }
};