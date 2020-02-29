#pragma once

#include "Canvas.h"
#include "HotToken.h"
#include "INode.h"
#include "RenderBehaviour.h"

class AbstractSequence;
class Brackets;
class Constant;
class ErrorNode;
class Imaginary;
class Variable;
class Vector;
class Sequence;
class Operation;

enum class ValueType {
  Scalar = 0,
  Vector = 1,
  Matrix = 2,
  Sequence = 3,
  Last = Sequence,
};

enum class NodeType {
  INodeImpl,
  Brackets,
  Operation,
  Constant,
  Variable,
  ErrorNode,
  Imaginary,
  Sequence,
  Vector,
  DivOperation,
  MultOperation,
  PlusOperation,
  MinusOperation,
  DiffOperation,
  PowOperation,
  SqrtOperation,
  SinOperation,
  CosOperation,
  UnMinusOperation,
  VectorMultOperation,
  LogOperation,
  CompareOperation,
};

class INodeImpl : public INode {
 public:
  // INode implementation
  INodeImpl* AsNodeImpl() override { return this; }
  const INodeImpl* AsNodeImpl() const override { return this; }

  // INodeImpl interface
  virtual NodeType GetNodeType() const = 0;
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
  virtual Sequence* AsSequence() { return nullptr; }
  virtual const Sequence* AsSequence() const { return nullptr; }
  virtual AbstractSequence* AsAbstractSequence() { return nullptr; }
  virtual const AbstractSequence* AsAbstractSequence() const { return nullptr; }
  virtual Imaginary* AsImaginary() { return nullptr; }
  virtual const Imaginary* AsImaginary() const { return nullptr; }
  virtual Brackets* AsBrackets() { return nullptr; }
  virtual const Brackets* AsBrackets() const { return nullptr; }
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

  CompareResult CompareType(const INode* rh) const;
};
