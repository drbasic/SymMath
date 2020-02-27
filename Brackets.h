#pragma once
#include <memory>
#include <string>
#include <vector>

#include "INodeImpl.h"

enum class BracketType {
  Round,
  Square,
  Fugure,
  Stright,
  Sqrt,
};

class Brackets : public INodeImpl {
 public:
  Brackets(BracketType bracket_type, std::unique_ptr<INode> value);

  static PrintSize RenderBrackets(const INodeImpl* node,
                                  BracketType bracket_type,
                                  Canvas* canvas,
                                  PrintBox print_box,
                                  bool dry_run,
                                  RenderBehaviour render_behaviour);

  // INode implementation
  bool IsEqual(const INode* rh) const override;
  std::unique_ptr<INode> Clone() const override;
  std::unique_ptr<INode> SymCalc(SymCalcSettings settings) const override;

  // INodeImpl implementation
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
  PrintSize LastPrintSize() const override;
  int Priority() const override { return 1000; }
  bool HasFrontMinus() const override { return false; }
  bool CheckCircular(const INodeImpl* other) const override;
  Constant* AsConstant() override;
  const Constant* AsConstant() const override;
  const ErrorNode* AsError() const override;
  const Variable* AsVariable() const override;
  Operation* AsOperation() override;
  const Operation* AsOperation() const override;

  void SimplifyImpl(HotToken token, std::unique_ptr<INode>* new_node) override;
  void OpenBracketsImpl(HotToken token,
                        std::unique_ptr<INode>* new_node) override;
  void ConvertToComplexImpl(HotToken token,
                            std::unique_ptr<INode>* new_node) override;

  INodeImpl* Value();
  const INodeImpl* Value() const;

 private:
  mutable PrintSize print_size_;
  bool transparent_ = false;
  BracketType bracket_type_ = BracketType::Round;
  std::unique_ptr<INode> value_;
};
