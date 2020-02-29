#pragma once

#include <memory>
#include <optional>
#include <string>

#include "INodeImpl.h"

class Constant : public INodeImpl {
 public:
  explicit Constant(double val);
  explicit Constant(bool val);
  Constant(double val, std::wstring name);

  // INode implementation
  CompareResult Compare(const INode* rh) const override;
  std::unique_ptr<INode> Clone() const override;
  std::unique_ptr<INode> SymCalc(SymCalcSettings settings) const override;

  // INodeImpl interface
  NodeType GetNodeType() const override { return NodeType::Constant; }
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
  PrintSize LastPrintSize() const override;
  int Priority() const override { return 100; }
  bool HasFrontMinus() const override;
  bool CheckCircular(const INodeImpl* other) const override { return false; }
  Constant* AsConstant() override { return this; }
  const Constant* AsConstant() const override { return this; }

  void SimplifyImpl(HotToken token, std::unique_ptr<INode>* new_node) override;
  void OpenBracketsImpl(HotToken token,
                        std::unique_ptr<INode>* new_node) override;
  void ConvertToComplexImpl(HotToken token,
                            std::unique_ptr<INode>* new_node) override;

  double Value() const { return value_; }
  const std::wstring& Name() const { return name_; }
  bool IsNamed() const { return !name_.empty(); }

 private:
  mutable PrintSize print_size_;
  std::optional<bool> bool_value_;
  double value_ = 0;
  std::wstring name_;
};
