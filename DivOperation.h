#pragma once

#include "Operation.h"

class DivOperation : public Operation {
 public:
  DivOperation(std::unique_ptr<INode> top, std::unique_ptr<INode> bottom);

  // INode implementation
  std::unique_ptr<INode> Clone() const override;

  // INodeImpl interface
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
  bool HasFrontMinus() const override;

  // IOperation implementation
  std::optional<CanonicMult> GetCanonicMult() override;
  std::optional<CanonicPow> GetCanonicPow() override;
  void SimplifyUnMinus(std::unique_ptr<INode>* new_node) override;
  void SimplifyDivDiv() override;
  void SimplifyConsts(std::unique_ptr<INode>* new_node) override;
  void SimplifyTheSame(std::unique_ptr<INode>* new_node) override;
  DivOperation* AsDivOperation() override { return this; }
  const DivOperation* AsDivOperation() const override { return this; }

 private:
  friend class Tests;

  INodeImpl* Top();
  const INodeImpl* Top() const;
  INodeImpl* Bottom();
  const INodeImpl* Bottom() const;
};
