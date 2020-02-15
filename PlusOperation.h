#pragma once
#include "Operation.h"

class PlusOperation : public Operation {
 public:
  PlusOperation(std::unique_ptr<INode> lh, std::unique_ptr<INode> rh);
  PlusOperation(std::vector<std::unique_ptr<INode>> operands);

 protected:
  std::unique_ptr<INode> Clone() const override;

 private:
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
  PlusOperation* AsPlusOperation() override { return this; }
  const PlusOperation* AsPlusOperation() const override { return this; }
  bool HasFrontMinus() const override;
};
