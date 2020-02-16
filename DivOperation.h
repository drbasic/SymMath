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
  std::optional<CanonicMult> GetCanonic() override;
  DivOperation* AsDivOperation() override { return this; }
  const DivOperation* AsDivOperation() const override { return this; }
};
