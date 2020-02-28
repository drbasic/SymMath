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
  std::unique_ptr<INode> SymCalc(SymCalcSettings settings) const override;

  // INodeImpl interface
  PrintSize LastPrintSize() const override;
  int Priority() const override;
  bool HasFrontMinus() const override { return false; }
  bool CheckCircular(const INodeImpl* other) const override;
  Operation* AsOperation() override { return this; }
  const Operation* AsOperation() const override { return this; }
  void SimplifyImpl(HotToken token, std::unique_ptr<INode>* new_node) override;
  void OpenBracketsImpl(HotToken token,
                        std::unique_ptr<INode>* new_node) override;
  void ConvertToComplexImpl(HotToken token,
                            std::unique_ptr<INode>* new_node) override;

  // IOperation implementation
  std::optional<CanonicMult> GetCanonicMult() override { return std::nullopt; }
  std::optional<CanonicPow> GetCanonicPow() override { return std::nullopt; }
  void UnfoldChains(HotToken token) override;
  void SimplifyUnMinus(HotToken token,
                       std::unique_ptr<INode>* new_node) override;
  void SimplifyChains(HotToken token,
                      std::unique_ptr<INode>* new_node) override;
  void SimplifyDivDiv(HotToken token) override;
  void SimplifyDivMul(HotToken token,
                      std::unique_ptr<INode>* new_node) override;
  void SimplifyConsts(HotToken token,
                      std::unique_ptr<INode>* new_node) override;
  void SimplifyTheSame(HotToken token,
                       std::unique_ptr<INode>* new_node) override;
  void OrderOperands(HotToken token) override;

  Op op() const;
  size_t OperandsCount() const;
  void CheckIntegrity() const;

 protected:
  friend class Tests;
  friend class INodeHelper;

  INodeImpl* Operand(size_t indx);
  const INodeImpl* Operand(size_t indx) const;
  std::unique_ptr<INode> TakeOperand(size_t indx);
  void SetOperand(size_t indx, std::unique_ptr<INode> node);
  std::vector<std::unique_ptr<INode>> TakeAllOperands();

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

  std::unique_ptr<INode> SymCalcValue(
      std::vector<std::unique_ptr<INode>> calculated_operands,
      SymCalcSettings settings) const;

  bool IsAllOperandsConst(
      SymCalcSettings settings,
      const std::vector<std::unique_ptr<INode>>& operands) const;

  bool is_dead_ = false;
  const OpInfo* op_info_;
  mutable PrintSize print_size_;
  std::vector<std::unique_ptr<INode>> operands_;
};
