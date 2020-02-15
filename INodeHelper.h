#pragma once
#include <memory>
#include <vector>

class INode;
class Operation;
class Constant;
class Canvas;
class MultOperation;
class PlusOperation;
class DivOperation;
class UnMinusOperation;
class Variable;

class INodeHelper {
 public:
  static bool IsNodesEqual(const INode* lh, const INode* rh);

  static const Constant* AsConstant(const INode* lh);

  static Operation* AsOperation(INode* lh);
  static const Operation* AsOperation(const INode* lh);

  static MultOperation* AsMult(INode* lh);
  static const MultOperation* AsMult(const INode* lh);

  static std::vector<std::unique_ptr<INode>>& GetOperands(Operation* op);

  static bool IsUnMinus(const INode* lh);
  static UnMinusOperation* AsUnMinus(INode* lh);
  static const UnMinusOperation* AsUnMinus(const INode* lh);

  static DivOperation* AsDiv(INode* lh);
  static const DivOperation* AsDiv(const INode* lh);

  static std::unique_ptr<MultOperation> ConvertToMul(std::unique_ptr<INode> rh);

  static std::unique_ptr<Constant> MakeConst(double value);
  static std::unique_ptr<UnMinusOperation> MakeUnMinus(
      std::unique_ptr<INode> value);
  static std::unique_ptr<PlusOperation> MakeMinus(std::unique_ptr<INode> lh,
                                                  std::unique_ptr<INode> rh);
  static std::unique_ptr<PlusOperation> MakePlus(std::unique_ptr<INode> lh,
                                                 std::unique_ptr<INode> rh);
  static std::unique_ptr<MultOperation> MakeMult(std::unique_ptr<INode> lh,
                                                 std::unique_ptr<INode> rh);
  static std::unique_ptr<MultOperation> MakeMult(
      std::vector<std::unique_ptr<INode>> operands);
  static std::unique_ptr<DivOperation> MakeDiv(std::unique_ptr<INode> lh,
                                               std::unique_ptr<INode> rh);
};