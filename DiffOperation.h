#pragma once

#include "Operation.h"

class VariableRef;

std::unique_ptr<INode> Differential(
    const OpInfo* op,
    std::vector<std::unique_ptr<INode>>* operands);

class DiffOperation : public Operation {
 public:
  DiffOperation(std::unique_ptr<INode> lh, std::unique_ptr<VariableRef> rh);

  // INode implementation
  std::unique_ptr<INode> Clone() const override;

  // INodeImpl implementation
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
  // IOperation implementation
  DiffOperation* AsDiffOperation() override { return this; }
  const DiffOperation* AsDiffOperation() const override { return this; }

  const INode* Value() const;
  const Variable* ByVar() const;

 private:
  enum class OperandIndex : size_t {
    Value = 0,
    ByVal = 1,
  };
  PrintSize RenderPrefix(Canvas* canvas,
                         PrintBox print_box,
                         bool dry_run,
                         RenderBehaviour render_behaviour) const;

  std::vector<const DiffOperation*> GetDiffChain() const;

  mutable std::unique_ptr<DivOperation> prefix_;
  mutable const INodeImpl* first_non_diff_operand_ = nullptr;
};
