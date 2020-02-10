#pragma once
#include <memory>
#include <string>
#include <vector>

struct OpInfo;
class ErrorNode;
class Operation;
class Constant;
enum class Op;

struct PrintSize {
  size_t width = 0;
  size_t height = 0;
};

struct Position {
  size_t x = 0;
  size_t y = 0;
};

class Canvas {
 public:
  void PrintAt(const Position& pos, const std::string& str);
  enum class Bracket {
    Left, Right
  };
  void RenderBracket(const Position& pos, Bracket br, size_t height);
};

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

  virtual PrintSize Render(Canvas* canvas,
                           const Position& pos,
                           bool dry_run,
                           bool ommit_front_minus) const = 0;
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
