#pragma once

#include "INode.h"

class Variable : public INode {
 public:
  Variable(std::string name);
  Variable(std::unique_ptr<INode> value);
  Variable(const Variable& var);

  std::unique_ptr<INode> SymCalc() const override;

  std::string Print() const;
  bool Simplify();
  std::string GetName() const;

  void operator=(std::unique_ptr<INode> value);
  void operator=(const Variable& var);
  void operator=(double val);
  operator std::unique_ptr<INode>() const;

 protected:
  bool IsEqual(const INode* rh) const override;
  std::unique_ptr<INode> Clone() const override;
  PrintSize Render(Canvas* canvas,
                   const Position& pos,
                   bool dry_run,
                   MinusBehavior minus_behavior) const override;
  PrintSize LastPrintSize() const override;
  int Priority() const override;
  bool HasFrontMinus() const override;
  bool CheckCircular(const INode* other) const override;
  Constant* AsConstant() override;
  const Constant* AsConstant() const override;
  const ErrorNode* AsError() const override;
  const Variable* AsVariable() const override;
  Operation* AsOperation() override;
  const Operation* AsOperation() const override;

  bool SimplifyImpl(std::unique_ptr<INode>* new_node) override;

 private:
  friend class VariableRef;
  INode* GetVisibleNode() const;
  std::string PrintRef(bool ommit_front_minus) const;

  mutable PrintSize print_size_;
  std::string name_;
  std::unique_ptr<INode> value_;
};
