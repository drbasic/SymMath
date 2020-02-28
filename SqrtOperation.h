#pragma once

#include "Operation.h"

double TrivialSqrt(double lh, double rh);
std::unique_ptr<INode> NonTrivialSqrt(
    const OpInfo* op,
    std::vector<std::unique_ptr<INode>>* operands);

class SqrtOperation : public Operation {
 public:
  enum class OperandIndex : size_t {
    Value = 0,
    ExpIndex = 1,
  };
  SqrtOperation(std::unique_ptr<INode> value, std::unique_ptr<INode> exp);

  // INode implementation
  std::unique_ptr<INode> Clone() const override;

  // INodeImpl interface
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
  bool HasFrontMinus() const override { return false; }

  // IOperation implementation
  std::optional<CanonicPow> GetCanonicPow() override;
  SqrtOperation* AsSqrtOperation() override { return this; }
  const SqrtOperation* AsSqrtOperation() const override { return this; }
  void SimplifyConsts(HotToken token,
                      std::unique_ptr<INode>* new_node) override;
  void SimplifyChains(HotToken token,
                      std::unique_ptr<INode>* new_node) override;

  INodeImpl* Value() {
    return Operand(static_cast<size_t>(OperandIndex::Value));
  }
  const INodeImpl* Value() const {
    return Operand(static_cast<size_t>(OperandIndex::Value));
  }
  INodeImpl* Exp() {
    return Operand(static_cast<size_t>(OperandIndex::ExpIndex));
  }
  const INodeImpl* Exp() const {
    return Operand(static_cast<size_t>(OperandIndex::ExpIndex));
  }
  std::unique_ptr<INode> TakeOperand(OperandIndex indx) {
    return Operation::TakeOperand(static_cast<size_t>(indx));
  }
  void SetOperand(OperandIndex indx, std::unique_ptr<INode> node) {
    Operation::SetOperand(static_cast<size_t>(indx), std::move(node));
  }

 private:
  void SimplifyExp(HotToken& token, std::unique_ptr<INode>* new_node);

  mutable PrintSize value_print_size_;
  mutable PrintSize pow_print_size_;
};