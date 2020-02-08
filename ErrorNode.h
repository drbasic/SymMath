#pragma once
#include "INode.h"

class ErrorNode : public INode {
 public:
  ErrorNode(std::string error);

  std::unique_ptr<INode> SymCalc() const override;

 protected:
  std::string PrintImpl(bool ommit_front_minus) const override;
  int Priority() const override;
  bool HasFrontMinus() const override;
  bool CheckCircular(const INode* other) const override;
  bool IsUnMinus() const override;

 private:
  std::string error_;
  std::unique_ptr<INode> value_;
};

