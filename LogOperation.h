#pragma once

#include "Operation.h"

double TrivialLogCalc(double lh, double rh);

class LogOperation : public Operation {
 public:
  enum : size_t {
    BaseIndex = 0,
    ValueIndex = 1,
  };
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
  void OpenBracketsImpl(HotToken token, std::unique_ptr<INode>* new_node) override;

  INodeImpl* Base() { return Operand(BaseIndex); }
  const INodeImpl* Base() const { return Operand(BaseIndex); }
  INodeImpl* Value() { return Operand(ValueIndex); }
  const INodeImpl* Value() const { return Operand(ValueIndex); }

 private:
  PrintSize RenderBase(Canvas* canvas,
                       PrintBox print_box,
                       bool dry_run,
                       RenderBehaviour render_behaviour) const;
  mutable PrintSize base_print_size_;
};
