#pragma once
#include <memory>
#include <vector>

#include "OpInfo.h"

enum class BracketType;
enum class ValueType;

struct CanonicMult;
struct CanonicPow;

class Brackets;
class Canvas;
class CompareOperation;
class Constant;
class DivOperation;
class Imaginary;
class INode;
class INodeImpl;
class MultOperation;
class Operation;
class PlusOperation;
class PowOperation;
class TrigonometricOperation;
class UnMinusOperation;
class Variable;
class Vector;
class VectorMultOperation;

class INodeHelper {
 public:
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

  static CanonicMult GetCanonicMult(std::unique_ptr<INode>& node);
  static CanonicPow GetCanonicPow(std::unique_ptr<INode>& node);
  static CanonicMult MergeCanonic(const CanonicMult& lh, const CanonicMult& rh);

  static std::unique_ptr<INode> Negate(std::unique_ptr<INode> node);
  static std::unique_ptr<MultOperation> ConvertToMul(std::unique_ptr<INode> rh);
  static void RemoveEmptyOperands(std::vector<std::unique_ptr<INode>>* nodes);
  static bool HasAnyPlusOperation(
      const std::vector<std::unique_ptr<INode>>& nodes);
  static bool HasAnyValueType(
      const std::vector<std::unique_ptr<INode>>& operands,
      ValueType value_type);
  static bool HasAllValueType(
      const std::vector<std::unique_ptr<INode>>& operands,
      ValueType value_type);

  static std::unique_ptr<Operation> MakeEmpty(Op op);
  static std::unique_ptr<INode> MakeError();
  static std::unique_ptr<INode> MakeError(std::string err);
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
  static std::unique_ptr<INode> MakePlusIfNeeded(
      std::vector<std::unique_ptr<INode>> nodes);
  static std::unique_ptr<MultOperation> MakeMult(std::unique_ptr<INode> lh,
                                                 std::unique_ptr<INode> rh);
  static std::unique_ptr<MultOperation> MakeMult(
      std::vector<std::unique_ptr<INode>> operands);
  static std::unique_ptr<INode> MakeMult(
      double dividend,
      double divider,
      std::vector<std::unique_ptr<INode>*> nodes);
  static std::unique_ptr<INode> MakeMultIfNeeded(
      std::vector<std::unique_ptr<INode>> nodes);
  static std::unique_ptr<VectorMultOperation> MakeVectorMult(
      std::unique_ptr<INode> lh,
      std::unique_ptr<INode> rh);
  static std::unique_ptr<DivOperation> MakeDiv(std::unique_ptr<INode> lh,
                                               std::unique_ptr<INode> rh);
  static std::unique_ptr<PowOperation> MakePow(std::unique_ptr<INode> lh,
                                               std::unique_ptr<INode> rh);
  static std::unique_ptr<INode> MakePowIfNeeded(std::unique_ptr<INode> lh,
                                                double exp);
  static std::unique_ptr<TrigonometricOperation> MakeTrigonometric(
      Op op,
      std::unique_ptr<INode> value);
  static std::unique_ptr<CompareOperation>
  MakeCompare(Op op, std::unique_ptr<INode> lh, std::unique_ptr<INode> rh);
  static std::unique_ptr<Brackets> MakeBrackets(BracketType bracket_type,
                                                std::unique_ptr<INode> value);
  static std::unique_ptr<Vector> MakeVector(std::unique_ptr<INode> a,
                                            std::unique_ptr<INode> b);
  static std::unique_ptr<Vector> MakeVector(std::unique_ptr<INode> a,
                                            std::unique_ptr<INode> b,
                                            std::unique_ptr<INode> c);
  static std::unique_ptr<Vector> MakeVector(
      std::vector<std::unique_ptr<INode>> values);
};
