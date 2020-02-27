#pragma once

#include "Operation.h"

class DivOperation : public Operation {
 public:
  enum : size_t {
    DividendIndex = 0,
    DividerIndex = 1,
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

  INodeImpl* Dividend() { return Operand(DividendIndex); }
  const INodeImpl* Dividend() const { return Operand(DividendIndex); }
  INodeImpl* Divider() { return Operand(DividerIndex); }
  const INodeImpl* Divider() const { return Operand(DividerIndex); }

 private:
  friend class Tests;

  void SimplifyCanonicConstants(HotToken& token,
                                std::unique_ptr<INode>* new_node);
  void SimplifyMultipliers(HotToken& token, std::unique_ptr<INode>* new_node);
};
