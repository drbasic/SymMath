#pragma once

#include <memory>
#include <string>
#include <vector>

#include "INodeImpl.h"

class Imaginary : public INodeImpl {
 public:
  Imaginary();

  // INode implementation
  bool IsEqual(const INode* rh) const override;
  std::unique_ptr<INode> Clone() const override;
  std::unique_ptr<INode> SymCalc() const override;

  // INodeImpl interface
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
  PrintSize LastPrintSize() const override;
  int Priority() const override { return 100; }
  bool HasFrontMinus() const override {return false;}
  bool CheckCircular(const INodeImpl* other) const override { return false; }
  Imaginary* AsImaginary() override { return this; }
  const Imaginary* AsImaginary() const override { return this; }

 private:
  mutable PrintSize print_size_;
};