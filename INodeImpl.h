#pragma once

#include "Canvas.h"
#include "INode.h"
#include "RenderBehaviour.h"
#include "HotToken.h"

class Constant;
class ErrorNode;
class Imaginary;
class Variable;
class Vector;
class Operation;

enum class ValueType {
  Scalar = 0,
  Vector = 1,
  Matrix = 2,
  Last = Matrix,
};

class INodeImpl : public INode {
 public:
  // INode implementation
  INodeImpl* AsNodeImpl() override { return this; }
  const INodeImpl* AsNodeImpl() const override { return this; }

  // INodeImpl interface
  virtual PrintSize Render(Canvas* canvas,
                           PrintBox print_box,
                           bool dry_run,
                           RenderBehaviour render_behaviour) const = 0;
  virtual PrintSize LastPrintSize() const = 0;

  virtual int Priority() const = 0;
  virtual bool HasFrontMinus() const = 0;
  virtual ValueType GetValueType() const { return ValueType::Scalar; }
  virtual bool CheckCircular(const INodeImpl* other) const = 0;

  virtual Constant* AsConstant() { return nullptr; }
  virtual const Constant* AsConstant() const { return nullptr; }
  virtual Vector* AsVector() { return nullptr; }
  virtual const Vector* AsVector() const { return nullptr; }
  virtual Imaginary* AsImaginary() { return nullptr; }
  virtual const Imaginary* AsImaginary() const { return nullptr; }
  virtual const ErrorNode* AsError() const { return nullptr; }
  virtual Variable* AsVariable() { return nullptr; }
  virtual const Variable* AsVariable() const { return nullptr; }
  virtual Operation* AsOperation() { return nullptr; }
  virtual const Operation* AsOperation() const { return nullptr; }

  virtual void SimplifyImpl(HotToken token,
                            std::unique_ptr<INode>* new_node) = 0;
  virtual void OpenBracketsImpl(HotToken token,
                                std::unique_ptr<INode>* new_node) = 0;
  virtual void ConvertToComplexImpl(HotToken token,
                                    std::unique_ptr<INode>* new_node) = 0;
};
