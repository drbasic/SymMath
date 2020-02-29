#pragma once

#include "Operation.h"

double TrivialLogCalc(double lh, double rh);

class LogOperation : public Operation {
 public:
  enum class OperandIndex : size_t {
    Base = 0,
    Value = 1,
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
  void OpenBracketsImpl(HotToken token,
                        std::unique_ptr<INode>* new_node) override;

  INodeImpl* Base() {
    return Operand(static_cast<size_t>(OperandIndex::Base));
  }
  const INodeImpl* Base() const {
    return Operand(static_cast<size_t>(OperandIndex::Base));
  }
  INodeImpl* Value() {
    return Operand(static_cast<size_t>(OperandIndex::Value));
  }
  const INodeImpl* Value() const {
    return Operand(static_cast<size_t>(OperandIndex::Value));
  }
  std::unique_ptr<INode> TakeOperand(OperandIndex indx) {
    return Operation::TakeOperand(static_cast<size_t>(indx));
  }
  void SetOperand(OperandIndex indx, std::unique_ptr<INode> node) {
    Operation::SetOperand(static_cast<size_t>(indx), std::move(node));
  }

 private:
  PrintSize RenderBase(Canvas* canvas,
                       PrintBox print_box,
                       bool dry_run,
                       RenderBehaviour render_behaviour) const;
  mutable PrintSize base_print_size_;
};
