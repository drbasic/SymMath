#pragma once

#include "Operation.h"

class VariableRef;

std::unique_ptr<INode> Differential (
    const OpInfo* op,
    std::vector<std::unique_ptr<INode>>* operands);

class DiffOperation : public Operation {
 public:
  DiffOperation(std::unique_ptr<INode> lh, std::unique_ptr<VariableRef> rh);

  // INode implementation
  std::unique_ptr<INode> Clone() const override;

  // INodeImpl interface
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;

 private:
  PrintSize RenderPrefix(Canvas* canvas,
                         PrintBox print_box,
                         bool dry_run,
                         RenderBehaviour render_behaviour) const;
};
