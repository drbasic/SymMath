#pragma once
#include <memory>
#include <string>
#include <vector>

struct OpInfo;
class Operation;
class Constant;
enum class Op;

class INode {
 public:
  virtual ~INode() {}

  virtual std::string Print() const;
  virtual std::unique_ptr<INode> SymCalc() const = 0;

 protected:
  friend class Operation;
  friend class Variable;

  virtual std::string PrintImpl(bool ommit_front_minus) const = 0;

  virtual int Priority() const = 0;
  virtual bool NeedBrackets() const = 0;
  virtual bool HasFrontMinus() const = 0;
  virtual bool CheckCircular(const INode* other) const = 0;

  virtual INode* GetVisibleNode() { return this; }
  virtual const INode* GetVisibleNode() const { return this; }

  virtual bool IsUnMinus() const = 0;
  virtual Operation* AsUnMinus() { return nullptr; }
  virtual Constant* AsConstant() { return nullptr; }

  virtual std::vector<std::unique_ptr<INode>> TakeOperands(Op op) { return {}; }

  virtual bool SimplifyImpl(std::unique_ptr<INode>* new_node) { return false; }
};
