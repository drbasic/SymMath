#pragma once
#include <memory>
#include <vector>

#include "OpInfo.h"

struct CanonicMult;
class Canvas;
class Constant;
class DivOperation;
class Imaginary;
class INode;
class INodeImpl;
class Operation;
class MultOperation;
class PlusOperation;
class TrigonometricOperation;
class UnMinusOperation;
class Variable;

class INodeHelper {
 public:
  static bool IsNodesEqual(const INode* lh, const INode* rh);

  static std::vector<std::unique_ptr<INode>>& GetOperands(Operation* op);

  static bool IsUnMinus(const INode* lh);
  static Constant* AsConstant(INode* lh);
  static const Constant* AsConstant(const INode* lh);
  static Operation* AsOperation(INode* lh);
  static const Operation* AsOperation(const INode* lh);
  static UnMinusOperation* AsUnMinus(INode* lh);
  static const UnMinusOperation* AsUnMinus(const INode* lh);
  static PlusOperation* AsPlus(INode* lh);
  static const PlusOperation* AsPlus(const INode* lh);
  static MultOperation* AsMult(INode* lh);
  static const MultOperation* AsMult(const INode* lh);
  static DivOperation* AsDiv(INode* lh);
  static const DivOperation* AsDiv(const INode* lh);

  static CanonicMult GetCanonic(std::unique_ptr<INode>& node);
  static CanonicMult MergeCanonic(const CanonicMult& lh, const CanonicMult& rh);

  static void ExctractNodesWithOp(Op op,
                                  std::vector<std::unique_ptr<INode>>* src,
                                  std::vector<std::unique_ptr<INode>>* nodes);
  static void ExctractNodesWithOp(Op op,
                                  std::unique_ptr<INode> src,
                                  std::vector<std::unique_ptr<INode>>* nodes);
  static void ExctractNodesWithOp(
      Op op,
      std::unique_ptr<INode> src,
      std::vector<std::unique_ptr<INode>>* positive_nodes,
      std::vector<std::unique_ptr<INode>>* negative_nodes);
  static std::unique_ptr<INode> Negate(std::unique_ptr<INode> node);
  static std::unique_ptr<INode> MakeMultIfNeeded(
      std::vector<std::unique_ptr<INode>> nodes);
  static std::unique_ptr<MultOperation> ConvertToMul(std::unique_ptr<INode> rh);
  static void RemoveEmptyOperands(std::vector<std::unique_ptr<INode>>* nodes);
  static bool HasAnyPlusOperation(
      const std::vector<std::unique_ptr<INode>>& nodes);

  static std::unique_ptr<Constant> MakeConst(double value);
  static std::unique_ptr<Imaginary> MakeImaginary();
  static std::unique_ptr<UnMinusOperation> MakeUnMinus(
      std::unique_ptr<INode> value);
  static std::unique_ptr<PlusOperation> MakeMinus(std::unique_ptr<INode> lh,
                                                  std::unique_ptr<INode> rh);
  static std::unique_ptr<PlusOperation> MakePlus(std::unique_ptr<INode> lh,
                                                 std::unique_ptr<INode> rh);
  static std::unique_ptr<PlusOperation> MakePlus(
      std::vector<std::unique_ptr<INode>> operands);
  static std::unique_ptr<MultOperation> MakeMult(std::unique_ptr<INode> lh,
                                                 std::unique_ptr<INode> rh);
  static std::unique_ptr<MultOperation> MakeMult(
      std::vector<std::unique_ptr<INode>> operands);
  static std::unique_ptr<INode> MakeMult(
      double dividend,
      double divider,
      std::vector<std::unique_ptr<INode>*> nodes);
  static std::unique_ptr<DivOperation> MakeDiv(std::unique_ptr<INode> lh,
                                               std::unique_ptr<INode> rh);
  static std::unique_ptr<TrigonometricOperation> MakeTrigonometric(
      Op op,
      std::unique_ptr<INode> value);
};
