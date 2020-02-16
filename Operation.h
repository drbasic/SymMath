#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "INodeImpl.h"
#include "IOperation.h"
#include "OpInfo.h"

class Operation : public IOperation {
 public:
  Operation(const OpInfo* op_info, std::unique_ptr<INode> lh);
  Operation(const OpInfo* op_info,
            std::unique_ptr<INode> lh,
            std::unique_ptr<INode> rh);
  Operation(const OpInfo* op_info,
            std::vector<std::unique_ptr<INode>> operands);

  // INode implementation
  bool IsEqual(const INode* rh) const override;
  std::unique_ptr<INode> SymCalc() const override;

  // INodeImpl interface
  PrintSize LastPrintSize() const override;
  int Priority() const override;
  bool HasFrontMinus() const override { return false; };
  bool CheckCircular(const INodeImpl* other) const override;
  Operation* AsOperation() override { return this; }
  const Operation* AsOperation() const override { return this; }
  bool SimplifyImpl(std::unique_ptr<INode>* new_node) override;

  // IOperation implementation
  std::optional<CanonicMult> GetCanonic() override;
  void SimplifyChain() override;

 protected:
  INodeImpl* Operand(size_t indx);
  const INodeImpl* Operand(size_t indx) const;

  PrintSize RenderOperandChain(Canvas* canvas,
                               PrintBox print_box,
                               bool dry_run,
                               RenderBehaviour render_behaviour) const;
  PrintSize RenderOperand(const INodeImpl* node,
                          Canvas* canvas,
                          PrintBox print_box,
                          bool dry_run,
                          RenderBehaviour render_behaviour,
                          bool with_op) const;

  friend class INodeHelper;
  friend class Tests;

  void CheckIntegrity() const;
  bool SimplifyUnMinus(std::unique_ptr<INode>* new_node);
  bool SimplifyDivExtractUnMinus(std::unique_ptr<INode>* new_node);
  bool SimplifyDivDiv();
  bool SimplifyDivMul();
  bool SimplifySame(std::unique_ptr<INode>* new_node);
  bool IsAllOperandsConst(
      const std::vector<std::unique_ptr<INode>>& operands) const;
  bool SimplifyConsts(std::unique_ptr<INode>* new_node);

  bool ReduceFor(double val);

  std::vector<std::unique_ptr<INode>> TakeOperands(Op op);

  void RemoveEmptyOperands();

  const OpInfo* op_info_;
  mutable PrintSize print_size_;
  std::vector<std::unique_ptr<INode>> operands_;
};
