#pragma once

#include "INode.h"
#include "INodeImpl.h"

class Variable : protected INodeImpl {
 public:
  Variable(std::wstring name);
  Variable(std::unique_ptr<INode> value);
  Variable(std::wstring name, std::unique_ptr<INode> value);
  Variable(const Variable&) = delete;
  ~Variable() override;

  std::wstring Print(bool with_calc = false, uint32_t base_line = 0) const;
  void Simplify();
  void OpenBrackets();
  void ConvertToComplex();
  std::wstring GetName() const;

  void operator=(std::unique_ptr<INode> value);
  void operator=(const Variable& var);
  void operator=(double val);
  operator std::unique_ptr<INode>() const;

  // INode implementation
  bool IsEqual(const INode* rh) const override;
  CompareResult Compare(const INode* rh) const override;
  std::unique_ptr<INode> Clone() const override;
  std::unique_ptr<INode> SymCalc(SymCalcSettings settings) const override;

 protected:
  // INodeImpl interface
  NodeType GetNodeType() const override;
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
  PrintSize LastPrintSize() const override;
  int Priority() const override;
  bool HasFrontMinus() const override;
  ValueType GetValueType() const override;
  bool CheckCircular(const INodeImpl* other) const override;
  Constant* AsConstant() override;
  const Constant* AsConstant() const override;
  const ErrorNode* AsError() const override;
  Variable* AsVariable() override { return this; }
  const Variable* AsVariable() const override { return this; }
  Operation* AsOperation() override;
  const Operation* AsOperation() const override;

  void SimplifyImpl(HotToken token, std::unique_ptr<INode>* new_node) override;
  void OpenBracketsImpl(HotToken token,
                        std::unique_ptr<INode>* new_node) override;
  void ConvertToComplexImpl(HotToken token,
                            std::unique_ptr<INode>* new_node) override;

 private:
  friend class VariableRef;
  friend class Tests;
  PrintSize RenderName(Canvas* canvas,
                       PrintBox print_box,
                       bool dry_run,
                       RenderBehaviour render_behaviour,
                       bool equal_sign) const;
  PrintSize RenderValue(Canvas* canvas,
                        PrintBox print_box,
                        bool dry_run,
                        RenderBehaviour render_behaviour) const;

  INodeImpl* Value();
  const INodeImpl* Value() const;
  INodeImpl* GetVisibleNode();
  const INodeImpl* GetVisibleNode() const;

  mutable PrintSize print_size_;
  std::wstring name_;
  std::unique_ptr<INode> value_;
};
