#pragma once
#include <memory>
#include <string>
#include <vector>

#include "INode.h"

class Constant : public INode {
 public:
  Constant(double val);

  std::unique_ptr<INode> SymCalc() const override;

  double Value() const { return value_; }

 protected:
  std::string PrintImpl(bool ommit_front_minus) const override;
  int Priority() const override;
  bool NeedBrackets() const override;
  bool HasFrontMinus() const override;
  bool CheckCircular(const INode* other) const override;
  bool IsUnMinus() const override;
  Constant* AsConstant() override;

 private:
  double value_ = 0;
};
