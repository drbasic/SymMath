#pragma once
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "INode.h"

struct CanonicMult {
  double a = 1;
  double b = 1;
  std::vector<std::unique_ptr<INode>*> nodes;
};

class IOperation : public INode {
 public:
  virtual ~IOperation(){};
  virtual std::optional<CanonicMult> GetCanonic() = 0;
  virtual void SimplifyChain() = 0;

  virtual UnMinusOperation* AsUnMinusOperation() { return nullptr; }
  virtual const UnMinusOperation* AsUnMinusOperation() const { return nullptr; }
  virtual PlusOperation* AsPlusOperation() { return nullptr; }
  virtual const PlusOperation* AsPlusOperation() const { return nullptr; }
  virtual MultOperation* AsMultOperation() { return nullptr; }
  virtual const MultOperation* AsMultOperation() const { return nullptr; }
  virtual DivOperation* AsDivOperation() { return nullptr; }
  virtual const DivOperation* AsDivOperation() const { return nullptr; }
};

class Operation : public IOperation {
 public:
  Operation(const OpInfo* op_info, std::unique_ptr<INode> lh);
  Operation(const OpInfo* op_info,
            std::unique_ptr<INode> lh,
            std::unique_ptr<INode> rh);
  Operation(const OpInfo* op_info,
            std::vector<std::unique_ptr<INode>> operands);

  std::unique_ptr<INode> SymCalc() const override;

 protected:
  bool IsEqual(const INode* rh) const override;

  PrintSize LastPrintSize() const override;
  int Priority() const override;
  bool HasFrontMinus() const override { return false; };
  bool CheckCircular(const INode* other) const override;

  Operation* AsOperation() override { return this; }
  const Operation* AsOperation() const override { return this; }

  std::optional<CanonicMult> GetCanonic() override;
  void SimplifyChain() override;

  bool SimplifyImpl(std::unique_ptr<INode>* new_node) override;

  PrintSize RenderOperandChain(Canvas* canvas,
                               PrintBox print_box,
                               bool dry_run,
                               RenderBehaviour render_behaviour) const;
  PrintSize RenderOperand(const INode* node,
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
