#include "VectorScalarProduct.h"

#include <cassert>

#include "INodeHelper.h"
#include "INodeImpl.h"
#include "MultOperation.h"
#include "PlusOperation.h"
#include "UnMinusOperation.h"
#include "ValueHelpers.h"
#include "Vector.h"

namespace {

template <int I>
struct Int2Type {
  enum { value = I };
};

enum CountableValueType : uint32_t {
  ScalarT = static_cast<int>(ValueType::Scalar),
  VectorT = static_cast<int>(ValueType::Vector),
  MatrixT = static_cast<int>(ValueType::Matrix),
  LastT = static_cast<int>(ValueType::Last),
};

uint32_t GetValueTypeIndex(ValueType value_type) {
  uint32_t result = static_cast<uint32_t>(value_type);
  assert(result <= LastT);
  return result;
}

std::unique_ptr<INode> Convert(Int2Type<ScalarT>, std::unique_ptr<INode> node) {
  return node;
}
std::unique_ptr<Vector> Convert(Int2Type<VectorT>,
                                std::unique_ptr<INode> node) {
  if (auto* as_vector = node->AsNodeImpl()->AsVector()) {
    node.release();
    return std::unique_ptr<Vector>(as_vector);
  }
  assert(false);
  return nullptr;
}
std::unique_ptr<INode> Convert(Int2Type<MatrixT>, std::unique_ptr<INode> node) {
  return std::unique_ptr<Vector>(node.release()->AsNodeImpl()->AsVector());
}

template <typename LH, typename RH>
std::pair<LH, RH> Swap(Int2Type<false>, LH lh, RH rh) {
  return std::pair<LH, RH>(std::move(lh), std::move(rh));
}
template <typename LH, typename RH>
std::pair<RH, LH> Swap(Int2Type<true>, LH lh, RH rh) {
  return std::pair<RH, LH>(std::move(rh), std::move(lh));
}

std::unique_ptr<INode> DoMult(std::unique_ptr<INode> lh,
                              std::unique_ptr<INode> rh);
std::unique_ptr<INode> DoMult(std::unique_ptr<INode> lh,
                              std::unique_ptr<Vector> rh);
std::unique_ptr<INode> DoMult(std::unique_ptr<Vector> lh,
                              std::unique_ptr<Vector> rh);

template <typename LhType, typename RhType, typename SwapType>
std::unique_ptr<INode> DoTemplateMult(std::unique_ptr<INode> lh,
                                      std::unique_ptr<INode> rh) {
  auto converted_lh = Convert(LhType(), std::move(lh));
  auto converted_rh = Convert(RhType(), std::move(rh));
  auto swaped =
      Swap(SwapType(), std::move(converted_lh), std::move(converted_rh));

  return DoMult(std::move(swaped.first), std::move(swaped.second));
}

struct MultInfo {
  using MultF = std::unique_ptr<INode> (*)(std::unique_ptr<INode> lh,
                                           std::unique_ptr<INode> rh);
  ValueType result_type;
  MultF mult_f;
};

/*Scalar = 0, Vector = 1, Matrix = 2,*/
const MultInfo kMultResult[3][3]{
    {
        // Scalar * Scalar
        {ValueType::Scalar,
         DoTemplateMult<Int2Type<ScalarT>, Int2Type<ScalarT>, Int2Type<false>>},
        // Scalar * Vector
        {ValueType::Vector,
         DoTemplateMult<Int2Type<ScalarT>, Int2Type<VectorT>, Int2Type<false>>},
        // Scalar * Matric
        {ValueType::Matrix,
         DoTemplateMult<Int2Type<ScalarT>, Int2Type<MatrixT>, Int2Type<false>>},
    },
    {
        // Vector * Scalar
        {ValueType::Vector,
         DoTemplateMult<Int2Type<VectorT>, Int2Type<ScalarT>, Int2Type<true>>},
        // Vector * Vector
        {ValueType::Scalar,
         DoTemplateMult<Int2Type<VectorT>, Int2Type<VectorT>, Int2Type<false>>},
        // Vector * Matric
        {ValueType::Vector,
         DoTemplateMult<Int2Type<VectorT>, Int2Type<MatrixT>, Int2Type<false>>},
    },
    {
        // Matric * Scalar
        {ValueType::Matrix,
         DoTemplateMult<Int2Type<MatrixT>, Int2Type<ScalarT>, Int2Type<true>>},
        // Matric * Vector
        {ValueType::Vector,
         DoTemplateMult<Int2Type<MatrixT>, Int2Type<VectorT>, Int2Type<true>>},
        // Matric * Matric
        {ValueType::Matrix,
         DoTemplateMult<Int2Type<MatrixT>, Int2Type<MatrixT>, Int2Type<false>>},
    },
};

std::unique_ptr<INode> DoMult(std::unique_ptr<INode> lh,
                              std::unique_ptr<INode> rh) {
  auto result = INodeHelper::MakeMult(std::move(lh), std::move(rh));
  result->UnfoldChains(HotToken::Make());
  return result->SymCalc();
}

std::unique_ptr<INode> DoMult(std::unique_ptr<INode> lh,
                              std::unique_ptr<Vector> rh) {
  std::vector<std::unique_ptr<INode>> values;
  values.reserve(rh->Size());
  for (size_t i = 0; i < rh->Size(); ++i) {
    auto value = INodeHelper::MakeMult(lh->Clone(), rh->TakeValue(i));
    value->UnfoldChains(HotToken::Make());
    values.push_back(value->SymCalc());
  }
  return INodeHelper::MakeVector(std::move(values));
}

std::unique_ptr<INode> DoMult(std::unique_ptr<Vector> lh,
                              std::unique_ptr<Vector> rh) {
  if (lh->Size() != rh->Size()) {
    return INodeHelper::MakeError("Vector sizes not match " +
                                  std::to_string(lh->Size()) +
                                  " != " + std::to_string(rh->Size()));
  }

  std::vector<std::unique_ptr<INode>> values;
  values.reserve(rh->Size());
  for (size_t i = 0; i < rh->Size(); ++i) {
    auto value = INodeHelper::MakeMult(lh->TakeValue(i), rh->TakeValue(i));
    value->UnfoldChains(HotToken::Make());
    values.push_back(value->SymCalc());
  }
  return INodeHelper::MakePlus(std::move(values))->SymCalc();
}

}  // namespace

ValueType GetMultResultType(ValueType lh, ValueType rh) {
  return kMultResult[GetValueTypeIndex(lh)][GetValueTypeIndex(rh)].result_type;
}

std::unique_ptr<INode> ScalarProduct(
    const OpInfo* op,
    std::vector<std::unique_ptr<INode>>* operands) {
  assert(op->op == Op::Mult);
  if (!INodeHelper::HasAnyValueType(*operands, ValueType::Vector))
    return nullptr;
  if (operands->size() < 2)
    return nullptr;
  std::unique_ptr<INode> lh = std::move((*operands)[0]);
  for (size_t i = 1; i < operands->size(); ++i) {
    std::unique_ptr<INode> rh = std::move((*operands)[i]);
    const auto& mult_info =
        kMultResult[GetValueTypeIndex(lh->AsNodeImpl()->GetValueType())]
                   [GetValueTypeIndex(rh->AsNodeImpl()->GetValueType())];
    lh = mult_info.mult_f(std::move(lh), std::move(rh));
  }
  return lh;
}

std::unique_ptr<INode> VectorProduct(
    const OpInfo* op,
    std::vector<std::unique_ptr<INode>>* operands) {
  assert(op->op == Op::VectorMult);
  if (operands->size() != 2)
    return INodeHelper::MakeError("Must have 2 operands");
  if (!INodeHelper::HasAllValueType(*operands, ValueType::Vector))
    return INodeHelper::MakeError("All operands must be vectors");

  auto lh = Convert(Int2Type<VectorT>(), std::move((*operands)[0]));
  assert(lh);
  auto rh = Convert(Int2Type<VectorT>(), std::move((*operands)[1]));
  assert(rh);
  if (lh->Size() != 3 || rh->Size() != 3)
    return INodeHelper::MakeError("All operands must be Vector3");

  constexpr size_t X = 0;
  constexpr size_t Y = 1;
  constexpr size_t Z = 2;
  auto x = (lh->Value(Y)->Clone() * rh->Value(Z)->Clone()) -
           (lh->Value(Z)->Clone() * rh->Value(Y)->Clone());
  auto y = (lh->Value(Z)->Clone() * rh->Value(X)->Clone()) -
           (lh->Value(X)->Clone() * rh->Value(Z)->Clone());
  auto z = (lh->Value(X)->Clone() * rh->Value(Y)->Clone()) -
           (lh->Value(Y)->Clone() * rh->Value(X)->Clone());
  return INodeHelper::MakeVector(std::move(x), std::move(y), std::move(z));
}

std::unique_ptr<INode> VectorAdd(
    const OpInfo* op,
    std::vector<std::unique_ptr<INode>>* operands) {
  assert(op->op == Op::Plus);

  size_t vector_count = 0;
  for (auto& operand : *operands) {
    if (operand->AsNodeImpl()->GetValueType() == ValueType::Vector)
      ++vector_count;
  }
  if (vector_count < 2)
    return nullptr;

  std::unique_ptr<Vector> vector_result;
  for (auto& operand : *operands) {
    if (operand->AsNodeImpl()->GetValueType() != ValueType::Vector)
      continue;
    if (!vector_result) {
      vector_result = Convert(Int2Type<VectorT>(), std::move(operand));
    } else {
      vector_result->Add(Convert(Int2Type<VectorT>(), std::move(operand)));
    }
  }
  INodeHelper::RemoveEmptyOperands(operands);
  operands->push_back(std::move(vector_result));
  return INodeHelper::MakePlusIfNeeded(std::move(*operands));
}

std::unique_ptr<INode> VectorUnMinus(
    const OpInfo* op,
    std::vector<std::unique_ptr<INode>>* operands) {
  assert(op->op == Op::UnMinus);
  assert(operands->size() == 1);

  auto* as_vector = operands->front()->AsNodeImpl()->AsVector();
  if (!as_vector)
    return nullptr;

  std::vector<std::unique_ptr<INode>> result_values;
  result_values.reserve(as_vector->Size());
  for (size_t i = 0; i < as_vector->Size(); ++i) {
    result_values.push_back(INodeHelper::MakeUnMinus(as_vector->TakeValue(i)));
  }

  return INodeHelper::MakeVector(std::move(result_values));
}
