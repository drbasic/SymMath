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
  bool HasFrontMinus() const override { return false; }
  bool CheckCircular(const INodeImpl* other) const override;
  Operation* AsOperation() override { return this; }
  const Operation* AsOperation() const override { return this; }
  void SimplifyImpl(std::unique_ptr<INode>* new_node) override;
  void OpenBracketsImpl(std::unique_ptr<INode>* new_node) override;
  void ConvertToComplexImpl(std::unique_ptr<INode>* new_node) override;

  // IOperation implementation
  std::optional<CanonicMult> GetCanonicMult() override { return std::nullopt; }
  std::optional<CanonicPow> GetCanonicPow() override { return std::nullopt; }
  void UnfoldChains() override;
  void SimplifyUnMinus(std::unique_ptr<INode>* new_node) override;
  void SimplifyChains(std::unique_ptr<INode>* new_node) override;
  void SimplifyDivDiv() override;
  void SimplifyDivMul(std::unique_ptr<INode>* new_node) override;
  void SimplifyConsts(std::unique_ptr<INode>* new_node) override;
  void SimplifyTheSame(std::unique_ptr<INode>* new_node) override;
  void OrderOperands() override;

  Op op() const;
  size_t OperandsCount() const;
  INodeImpl* Operand(size_t indx);
  const INodeImpl* Operand(size_t indx) const;
  std::unique_ptr<INode> TakeOperand(size_t indx);
  std::vector<std::unique_ptr<INode>> TakeAllOperands();
  void CheckIntegrity() const;

 protected:
  friend class Tests;

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

  bool IsAllOperandsConst(
      const std::vector<std::unique_ptr<INode>>& operands) const;

  bool is_dead_ = false;
  const OpInfo* op_info_;
  mutable PrintSize print_size_;
  std::vector<std::unique_ptr<INode>> operands_;
};
