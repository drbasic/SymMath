#pragma once

#include "Operation.h"

class PowOperation : public Operation {
 public:
  enum : size_t {
    BaseIndex = 0,
    PowIndex = 1,
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
  void OpenBracketsImpl(std::unique_ptr<INode>* new_node) override;

  // IOperation implementation
  std::optional<CanonicPow> GetCanonicPow() override;
  PowOperation* AsPowOperation() override { return this; }
  const PowOperation* AsPowOperation() const override { return this; }
  void SimplifyChains(std::unique_ptr<INode>* new_node) override;

  INodeImpl* Base();
  const INodeImpl* Base() const;
  INodeImpl* Exp();
  const INodeImpl* Exp() const;

 private:
  void SimplifyExp(std::unique_ptr<INode>* new_node);

  mutable PrintSize base_print_size_;
  mutable PrintSize pow_print_size_;
};
