#pragma once

#include "Operation.h"

class TrigonometricOperation : public Operation {
 public:
  TrigonometricOperation(const OpInfo* op_info, std::unique_ptr<INode> lh);

  // INode implementation
  std::unique_ptr<INode> Clone() const override;

  // INodeImpl interface
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
};
