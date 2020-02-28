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
  void UnfoldChains(HotToken token) override;
  void SimplifyConsts(HotToken token,
                      std::unique_ptr<INode>* new_node) override;
  void SimplifyTheSame(HotToken token,
                       std::unique_ptr<INode>* new_node) override;
  void OrderOperands(HotToken token) override;
  PlusOperation* AsPlusOperation() override { return this; }
  const PlusOperation* AsPlusOperation() const override { return this; }
  void OpenBracketsImpl(HotToken token,
                        std::unique_ptr<INode>* new_node) override;

  INodeImpl* Operand(size_t indx) { return Operation::Operand(indx); }
  const INodeImpl* Operand(size_t indx) const {
    return Operation::Operand(indx);
  }
  using Operation::SetOperand;
  using Operation::TakeAllOperands;
  using Operation::TakeOperand;

 private:
};
