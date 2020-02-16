#pragma once
#include "Operation.h"

class MultOperation : public Operation {
 public:
  MultOperation(std::unique_ptr<INode> lh, std::unique_ptr<INode> rh);
  MultOperation(std::vector<std::unique_ptr<INode>> operands);

 protected:
  std::unique_ptr<INode> Clone() const override;

 private:
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
  bool HasFrontMinus() const override;
  std::optional<CanonicMult> GetCanonic() override;
  void SimplifyChain() override;
  MultOperation* AsMultOperation() override { return this; }
  const MultOperation* AsMultOperation() const override { return this; }
  void UnfoldChain();
};
