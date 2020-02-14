#pragma once
#include <memory>
#include <string>
#include <vector>

#include "INode.h"

class Constant : public INode {
 public:
  Constant(double val);

  std::unique_ptr<INode> SymCalc() const override;

  double Value() const { return value_; }

 protected:
  bool IsEqual(const INode* rh) const override;
  std::unique_ptr<INode> Clone() const override;

  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   MinusBehavior minus_behavior) const override;
  PrintSize LastPrintSize() const override;
  int Priority() const override;
  bool HasFrontMinus() const override;
  bool CheckCircular(const INode* other) const override;

  Constant* AsConstant() override;
  const Constant* AsConstant() const override;

 private:
  mutable PrintSize print_size_;
  double value_ = 0;
};
