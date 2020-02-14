#pragma once
#include <memory>
#include <string>
#include <vector>

#include "INode.h"

enum class BracketType {
  Round,
  Square,
  Fugure,
};

class Brackets : public INode {
 public:
  Brackets(BracketType bracket_type, std::unique_ptr<INode> value);

  static PrintSize RenderBrackets(const INode* node,
                                  BracketType bracket_type,
                                  Canvas* canvas,
                                  PrintBox print_box,
                                  bool dry_run,
                                  MinusBehavior minus_behavior);

  std::unique_ptr<INode> SymCalc() const override;

 protected:
  bool IsEqual(const INode* rh) const override;
  std::unique_ptr<INode> Clone() const override;

  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
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

 private:
  mutable PrintSize print_size_;
  bool transparent_ = false;
  BracketType bracket_type_ = BracketType::Round;
  std::unique_ptr<INode> value_;
};
