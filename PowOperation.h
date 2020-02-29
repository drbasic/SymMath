#pragma once

#include "Operation.h"

class PowOperation : public Operation {
 public:
  enum class OperandIndex : size_t {
    Base = 0,
    Pow = 1,
  };
  PowOperation(std::unique_ptr<INode> lh, std::unique_ptr<INode> rh);

  static std::unique_ptr<INode> MakeIfNeeded(std::unique_ptr<INode> base,
                                             double exp);

  // INode implementation
  std::unique_ptr<INode> Clone() const override;

  // INodeImpl interface
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
  bool HasFrontMinus() const override { return false; }
  void OpenBracketsImpl(HotToken token,
                        std::unique_ptr<INode>* new_node) override;

  // IOperation implementation
  std::optional<CanonicPow> GetCanonicPow() override;
  PowOperation* AsPowOperation() override { return this; }
  const PowOperation* AsPowOperation() const override { return this; }
  void SimplifyConsts(HotToken token,
                      std::unique_ptr<INode>* new_node) override;

  INodeImpl* Base() { return Operand(static_cast<size_t>(OperandIndex::Base)); }
  const INodeImpl* Base() const {
    return Operand(static_cast<size_t>(OperandIndex::Base));
  }
  INodeImpl* Exp() { return Operand(static_cast<size_t>(OperandIndex::Pow)); }
  const INodeImpl* Exp() const {
    return Operand(static_cast<size_t>(OperandIndex::Pow));
  }
  std::unique_ptr<INode> TakeOperand(OperandIndex indx) {
    return Operation::TakeOperand(static_cast<size_t>(indx));
  }
  void SetOperand(OperandIndex indx, std::unique_ptr<INode> node) {
    Operation::SetOperand(static_cast<size_t>(indx), std::move(node));
  }

 private:
  void SimplifyExp(HotToken& token, std::unique_ptr<INode>* new_node);

  mutable PrintSize base_print_size_;
  mutable PrintSize pow_print_size_;
};
