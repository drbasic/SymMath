#pragma once

#include "Operation.h"

class MultOperation : public Operation {
 public:
  MultOperation(std::unique_ptr<INode> lh, std::unique_ptr<INode> rh);
  MultOperation(std::vector<std::unique_ptr<INode>> operands);

  // INode implementation
  std::unique_ptr<INode> Clone() const override;

  // INodeImpl interface
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
  bool HasFrontMinus() const override;

  // IOperation implementation
  std::optional<CanonicMult> GetCanonic() override;
  void ProcessImaginary(
      std::vector<std::unique_ptr<INode>>* nodes) const override;
  void UnfoldChains() override;
  void SimplifyUnMinus(std::unique_ptr<INode>* new_node) override;
  void SimplifyChains(std::unique_ptr<INode>* new_node) override;
  void SimplifyDivMul(std::unique_ptr<INode>* new_node) override;
  void SimplifyConsts(std::unique_ptr<INode>* new_node) override;
  void OpenBrackets(std::unique_ptr<INode>* new_node) override;
  MultOperation* AsMultOperation() override { return this; }
  const MultOperation* AsMultOperation() const override { return this; }

 private:
};