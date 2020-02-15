#pragma once
#include <memory>
#include <vector>

class INode;
class Operation;
class Constant;
class Canvas;
class Variable;

class INodeHelper {
 public:
  static bool IsNodesEqual(const INode* lh, const INode* rh);

  static const Constant* AsConstant(const INode* lh);

  static const Operation* AsOperation(const INode* lh);

  static Operation* AsOperation(INode* lh);

  static Operation* AsMult(INode* lh);

  static const Operation* AsMult(const INode* lh);

  static std::vector<std::unique_ptr<INode>>& GetOperands(Operation* op);

  static bool IsUnMinus(const INode* lh);

  static Operation* AsUnMinus(INode* lh);

  static const Operation* AsDiv(const INode* lh);

  static Operation* AsDiv(INode* lh);

  static std::unique_ptr<Operation> ConvertToMul(std::unique_ptr<INode> rh);

  static std::unique_ptr<Operation> MakeUnMinus(std::unique_ptr<INode> inner);

  static std::unique_ptr<Operation> MakeDiv(std::unique_ptr<INode> lh,
                                            std::unique_ptr<INode> rh);
};