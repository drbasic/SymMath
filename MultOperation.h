#pragma once

#include "Operation.h"

class MultOperation : public Operation {
 public:
  MultOperation(std::unique_ptr<INode> lh, std::unique_ptr<INode> rh);
  MultOperation(std::vector<std::unique_ptr<INode>> operands);

  // INode implementation
  std::unique_ptr<INode> Clone() const override;

  // INodeImpl interface
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
  bool HasFrontMinus() const override;
  ValueType GetValueType() const override;
  void OpenBracketsImpl(HotToken token, std::unique_ptr<INode>* new_node) override;

  // IOperation implementation
  std::optional<CanonicMult> GetCanonicMult() override;
  std::optional<CanonicPow> GetCanonicPow() override;
  void UnfoldChains(HotToken token) override;
  void SimplifyUnMinus(HotToken token, std::unique_ptr<INode>* new_node) override;
  void SimplifyChains(HotToken token, std::unique_ptr<INode>* new_node) override;
  void SimplifyDivMul(HotToken token, std::unique_ptr<INode>* new_node) override;
  void SimplifyConsts(HotToken token, std::unique_ptr<INode>* new_node) override;
  void SimplifyTheSame(HotToken token, std::unique_ptr<INode>* new_node) override;
  void OrderOperands() override;
  MultOperation* AsMultOperation() override { return this; }
  const MultOperation* AsMultOperation() const override { return this; }

   static std::unique_ptr<INode> ProcessImaginary(
      std::vector<std::unique_ptr<INode>>* nodes);
 private:
  void SimplifyTheSameMult(std::unique_ptr<INode>* new_node);
  void SimplifyTheSamePow(std::unique_ptr<INode>* new_node);

  void OpenPlusBrackets(std::unique_ptr<INode>* new_node);
};
