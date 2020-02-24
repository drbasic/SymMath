#pragma once
#include <memory>
#include <string>
#include <vector>

#include "INodeImpl.h"

class Vector : public INodeImpl {
 public:
  Vector();
  Vector(std::unique_ptr<INode> a, std::unique_ptr<INode> b);
  Vector(std::unique_ptr<INode> a,
         std::unique_ptr<INode> b,
         std::unique_ptr<INode> c);
  Vector(std::vector<std::unique_ptr<INode>> values);

  // INode implementation
  bool IsEqual(const INode* rh) const override;
  std::unique_ptr<INode> Clone() const override;
  std::unique_ptr<INode> SymCalc() const override;

  // INodeImpl interface
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
  PrintSize LastPrintSize() const override { return print_size_; }
  int Priority() const override { return 100; }
  bool HasFrontMinus() const override { return false; }
  ValueType GetValueType() const override { return ValueType::Vector; }
  bool CheckCircular(const INodeImpl* other) const override;
  Vector* AsVector() override { return this; }
  const Vector* AsVector() const override { return this; }
  void SimplifyImpl(std::unique_ptr<INode>* new_node) override;
  void OpenBracketsImpl(std::unique_ptr<INode>* new_node) override;
  void ConvertToComplexImpl(std::unique_ptr<INode>* new_node) override;

  size_t Size() const { return values_.size(); }
  std::unique_ptr<INode> TakeValue(size_t indx);
  const INode* Value(size_t indx) const { return values_[indx].get(); }
  void Add(std::unique_ptr<Vector> rh);

 private:
  PrintSize RenderAllValues(Canvas* canvas,
                            PrintBox print_box,
                            bool dry_run,
                            RenderBehaviour render_behaviour) const;
  PrintSize RenderValue(const INode* value,
                        Canvas* canvas,
                        PrintBox print_box,
                        bool dry_run,
                        RenderBehaviour render_behaviour,
                        bool with_comma) const;

  mutable PrintSize print_size_;
  mutable PrintSize values_print_size_;
  std::vector<std::unique_ptr<INode>> values_;
};
