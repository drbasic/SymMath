#pragma once
#include <memory>

#include "AbstractSequence.h"
#include "INodeImpl.h"

class Vector : public AbstractSequence {
 public:
  Vector();
  Vector(std::unique_ptr<INode> a, std::unique_ptr<INode> b);
  Vector(std::unique_ptr<INode> a,
         std::unique_ptr<INode> b,
         std::unique_ptr<INode> c);
  Vector(std::vector<std::unique_ptr<INode>> values);

  // INode implementation
  std::unique_ptr<INode> Clone() const override;
  std::unique_ptr<INode> SymCalc(SymCalcSettings settings) const override;

  // INodeImpl interface
  NodeType GetNodeType() const override { return NodeType::Vector; }
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
  int Priority() const override { return 100; }
  ValueType GetValueType() const override { return ValueType::Vector; }
  Vector* AsVector() override { return this; }
  const Vector* AsVector() const override { return this; }

  void Add(std::unique_ptr<Vector> rh);

 private:
};
