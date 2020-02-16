#pragma once
#include "Operation.h"

class TrigonometricOperation : public Operation {
 public:
  TrigonometricOperation(const OpInfo* op_info, std::unique_ptr<INode> lh);

 protected:
  std::unique_ptr<INode> Clone() const override;

 private:
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
};
