#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Canvas.h"
#include "RenderBehaviour.h"

struct OpInfo;
class ErrorNode;
class MultOperation;
class Operation;
class PlusOperation;
class Constant;
class Canvas;
class DivOperation;
class Variable;
class UnMinusOperation;
class TrigonometricOperation;
enum class Op;

class INode {
 public:
  virtual ~INode() {}

  virtual std::unique_ptr<INode> SymCalc() const = 0;

 protected:
  friend class Brackets;
  friend class Constant;
  friend class DivOperation;
  friend class ErrorNode;
  friend class INodeHelper;
  friend class MultOperation;
  friend class Operation;
  friend class PlusOperation;
  friend class Variable;
  friend class UnMinusOperation;
  friend class TrigonometricOperation;
  friend class Tests;

  virtual bool IsEqual(const INode* rh) const = 0;
  virtual std::unique_ptr<INode> Clone() const = 0;

  virtual PrintSize Render(Canvas* canvas,
                           PrintBox print_box,
                           bool dry_run,
                           RenderBehaviour render_behaviour) const = 0;
  virtual PrintSize LastPrintSize() const = 0;

  virtual int Priority() const = 0;
  virtual bool HasFrontMinus() const = 0;
  virtual bool CheckCircular(const INode* other) const = 0;

  virtual Constant* AsConstant() { return nullptr; }
  virtual const Constant* AsConstant() const { return nullptr; }
  virtual const ErrorNode* AsError() const { return nullptr; }
  virtual const Variable* AsVariable() const { return nullptr; }
  virtual Operation* AsOperation() { return nullptr; }
  virtual const Operation* AsOperation() const { return nullptr; }

  virtual bool SimplifyImpl(std::unique_ptr<INode>* new_node) { return false; }
};
