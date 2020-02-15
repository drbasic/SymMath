#pragma once
#include "Operation.h"

class TrigonometricOperator : public Operation {
 public:
  TrigonometricOperator(const OpInfo* op_info, std::unique_ptr<INode> lh);

 protected:
  std::unique_ptr<INode> Clone() const override;

 private:
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
};
