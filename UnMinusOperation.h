#pragma once

#include "Operation.h"

class UnMinusOperation : public Operation {
 public:
  UnMinusOperation(std::unique_ptr<INode> value);

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
  void SimplifyUnMinus(HotToken token, std::unique_ptr<INode>* new_node) override;
  UnMinusOperation* AsUnMinusOperation() override { return this; }
  const UnMinusOperation* AsUnMinusOperation() const override { return this; }
};