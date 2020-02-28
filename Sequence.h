#pragma once

#include <memory>

#include "AbstractSequence.h"
#include "INodeImpl.h"

class Sequence : public AbstractSequence {
 public:
  Sequence();

  // INode implementation
  bool IsEqual(const INode* rh) const override;
  std::unique_ptr<INode> Clone() const override;
  std::unique_ptr<INode> SymCalc(SymCalcSettings settings) const override;

  // INodeImpl interface
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
  int Priority() const override { return 100; }
  ValueType GetValueType() const override { return ValueType::Sequence; }
  Sequence* AsSequence() override { return this; }
  const Sequence* AsSequence() const override { return this; }

 private:
};
