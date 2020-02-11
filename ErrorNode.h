#pragma once
#include "INode.h"

class ErrorNode : public INode {
 public:
  ErrorNode(std::string error);

  std::unique_ptr<INode> SymCalc() const override;

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

 private:
  mutable PrintSize print_size_;
  std::string error_;
  std::unique_ptr<INode> value_;
};
