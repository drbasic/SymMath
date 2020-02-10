#pragma once
#include <memory>
#include <string>
#include <vector>

#include "INode.h"

class Constant : public INode {
 public:
  Constant(double val);

  std::unique_ptr<INode> SymCalc() const override;
  std::unique_ptr<INode> Clone() const override;

  double Value() const { return value_; }

 protected:
  bool IsEqual(const INode* rh) const override;

  std::string PrintImpl(bool ommit_front_minus) const override;
  int Priority() const override;
  bool HasFrontMinus() const override;
  bool CheckCircular(const INode* other) const override;

  Constant* AsConstant() override;
  const Constant* AsConstant() const override;

 private:
  double value_ = 0;
};
