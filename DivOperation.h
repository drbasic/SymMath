#pragma once
#include "Operation.h"

class DivOperation : public Operation {
 public:
  DivOperation(std::unique_ptr<INode> top, std::unique_ptr<INode> bottom);

 protected:
  std::unique_ptr<INode> Clone() const override;

 private:
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
};
