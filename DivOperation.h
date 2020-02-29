#pragma once

#include "Operation.h"

class DivOperation : public Operation {
 public:
  enum class OperandIndex : size_t {
    Dividend = 0,
    Divider = 1,
  };
  DivOperation(std::unique_ptr<INode> top, std::unique_ptr<INode> bottom);

  // INode implementation
  std::unique_ptr<INode> Clone() const override;

  // INodeImpl interface
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
  bool HasFrontMinus() const override;

  // IOperation implementation
  std::optional<CanonicMult> GetCanonicMult() override;
  std::optional<CanonicPow> GetCanonicPow() override;
  void SimplifyUnMinus(HotToken token,
                       std::unique_ptr<INode>* new_node) override;
  void SimplifyDivDiv(HotToken token) override;
  void SimplifyConsts(HotToken token,
                      std::unique_ptr<INode>* new_node) override;
  void SimplifyTheSame(HotToken token,
                       std::unique_ptr<INode>* new_node) override;
  DivOperation* AsDivOperation() override { return this; }
  const DivOperation* AsDivOperation() const override { return this; }

  INodeImpl* Dividend() {
    return Operand(static_cast<size_t>(OperandIndex::Dividend));
  }
  const INodeImpl* Dividend() const {
    return Operand(static_cast<size_t>(OperandIndex::Dividend));
  }
  INodeImpl* Divider() {
    return Operand(static_cast<size_t>(OperandIndex::Divider));
  }
  const INodeImpl* Divider() const {
    return Operand(static_cast<size_t>(OperandIndex::Divider));
  }
  std::unique_ptr<INode> TakeOperand(OperandIndex indx) {
    return Operation::TakeOperand(static_cast<size_t>(indx));
  }
  void SetOperand(OperandIndex indx, std::unique_ptr<INode> node) {
    Operation::SetOperand(static_cast<size_t>(indx), std::move(node));
  }

 private:
  friend class Tests;

  void SimplifyCanonicConstants(HotToken& token,
                                std::unique_ptr<INode>* new_node);
  void SimplifyMultipliers(HotToken& token, std::unique_ptr<INode>* new_node);
};
