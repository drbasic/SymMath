#pragma once

#include "Operation.h"

double TrivialLogCalc(double lh, double rh);

class LogOperation : public Operation {
 public:
  LogOperation(std::unique_ptr<INode> base, std::unique_ptr<INode> value);

  // INode implementation
  std::unique_ptr<INode> Clone() const override;

  // INodeImpl interface
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
  bool HasFrontMinus() const override { return false; }

  // IOperation implementation
  LogOperation* AsLogOperation() override { return this; }
  const LogOperation* AsLogOperation() const override { return this; }

  INodeImpl* Base();
  const INodeImpl* Base() const;

 private:
  PrintSize RenderBase(Canvas* canvas,
                       PrintBox print_box,
                       bool dry_run,
                       RenderBehaviour render_behaviour) const;
  mutable PrintSize base_print_size_;
};
