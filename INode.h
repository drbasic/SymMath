#pragma once
#include <memory>
#include <string>
#include <vector>

struct OpInfo;
class ErrorNode;
class Operation;
class Constant;
enum class Op;

class INode {
 public:
  virtual ~INode() {}

  virtual std::unique_ptr<INode> SymCalc() const = 0;

 protected:
  friend class Constant;
  friend class ErrorNode;
  friend class Operation;
  friend class Variable;
  friend class INodeAcessor;

  virtual bool IsEqual(const INode* rh) const = 0;
  virtual std::unique_ptr<INode> Clone() const = 0;

  virtual std::string PrintImpl(bool ommit_front_minus) const = 0;

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
