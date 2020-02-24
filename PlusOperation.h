#pragma once

#include "Operation.h"

class PlusOperation : public Operation {
 public:
  PlusOperation(std::unique_ptr<INode> lh, std::unique_ptr<INode> rh);
  PlusOperation(std::vector<std::unique_ptr<INode>> operands);

  // INode implementation
  std::unique_ptr<INode> Clone() const override;

  // INodeImpl interface
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
  bool HasFrontMinus() const override;

  // IOperation implementation
  void UnfoldChains() override;
  void SimplifyConsts(std::unique_ptr<INode>* new_node) override;
  void SimplifyTheSame(std::unique_ptr<INode>* new_node) override;
  void OrderOperands() override;
  PlusOperation* AsPlusOperation() override { return this; }
  const PlusOperation* AsPlusOperation() const override { return this; }

 private:
};
