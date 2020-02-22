#pragma once

#include <memory>
#include <optional>

#include "INodeImpl.h"

class Constant : public INodeImpl {
 public:
  Constant(double val);
  Constant(bool val);

  // INode implementation
  bool IsEqual(const INode* rh) const override;
  std::unique_ptr<INode> Clone() const override;
  std::unique_ptr<INode> SymCalc() const override;

  // INodeImpl interface
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
  PrintSize LastPrintSize() const override;
  int Priority() const override { return 100; }
  bool HasFrontMinus() const override;
  bool CheckCircular(const INodeImpl* other) const override { return false; }
  Constant* AsConstant() override { return this; }
  const Constant* AsConstant() const override { return this; }

  double Value() const { return value_; }

 private:
  mutable PrintSize print_size_;
  std::optional<bool> bool_value_;
  double value_ = 0;
};
