#pragma once

#include "Operation.h"

double TrivialSqrt(double lh, double rh);
std::unique_ptr<INode> NonTrivialSqrt(
    const OpInfo* op,
    std::vector<std::unique_ptr<INode>>* operands);

class SqrtOperation : public Operation {
 public:
  enum : size_t {
    ValueIndex = 0,
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
  void SimplifyChains(HotToken token,
                      std::unique_ptr<INode>* new_node) override;

  INodeImpl* Value() { return Operand(ValueIndex); }
  const INodeImpl* Value() const { return Operand(ValueIndex); }
  INodeImpl* Exp() { return Operand(ExpIndex); }
  const INodeImpl* Exp() const { return Operand(ExpIndex); }

 private:
  void SimplifyExp(std::unique_ptr<INode>* new_node);

  mutable PrintSize value_print_size_;
  mutable PrintSize pow_print_size_;
};