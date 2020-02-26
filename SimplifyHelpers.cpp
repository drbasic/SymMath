#include "SimplifyHelpers.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <map>

#include "Constant.h"
#include "DivOperation.h"
#include "INodeHelper.h"
#include "IOperation.h"
#include "MultOperation.h"
#include "PlusOperation.h"
#include "PowOperation.h"
#include "UnMinusOperation.h"
#include "Variable.h"

namespace {

bool ReduceFullMultiplicity(double top,
                            double bottom,
                            double* new_top,
                            double* new_bottom) {
  bool result = false;
  if (bottom < 0) {
    result = true;
    *new_top = top = -top;
    *new_bottom = bottom = -bottom;
  }

  double quotient = top / bottom;
  double intpart;
  if (std::modf(quotient, &intpart) == 0.0) {
    *new_top = quotient;
    *new_bottom = 1.0;
    return true;
  }
  quotient = bottom / top;
  if (std::modf(quotient, &intpart) == 0.0) {
    *new_top = 1.0;
    *new_bottom = quotient;
    return true;
  }
  return result;
}

std::wstring GetBaseName(const INode* node) {
  if (const auto* as_var = INodeHelper::AsVariable(node))
    return as_var->GetName();
  if (const auto* as_un_minus = INodeHelper::AsUnMinus(node))
    return GetBaseName(as_un_minus->Operand(0));
  if (const auto* as_pow = INodeHelper::AsPow(node)) {
    return GetBaseName(as_pow->Base());
  }
  if (const auto* as_mult = INodeHelper::AsMult(node)) {
    std::wstring result;
    for (size_t i = 0; i < as_mult->OperandsCount(); ++i) {
      result += GetBaseName(as_mult->Operand(i));
    }
    return result;
  }
  if (const auto* as_div = INodeHelper::AsDiv(node)) {
    std::wstring result =
        GetBaseName(as_div->Operand(0)) + GetBaseName(as_div->Operand(1));
    return result;
  }
  return std::wstring();
}

std::optional<double> Factorize(double val,
                                double max_val,
                                std::vector<std::unique_ptr<INode>>* result) {
  bool found = false;
  if (val < 0) {
    val = -val;
    found = true;
    result->push_back(INodeHelper::MakeConst(-1.0));
  }
  max_val = std::max(std::abs(max_val), sqrt(val));

  double v = 2;
  for (; v <= max_val;) {
    if (std::remainder(val, v) == 0.0) {
      result->push_back(INodeHelper::MakeConst(v));
      val /= v;
      found = true;
      continue;
    }
    if (v == 2)
      v += 1;
    else
      v += 2;
  }
  if (found)
    return v;
  return std::nullopt;
}

}  // namespace

bool IsNodesTransitiveEqual(const std::vector<const INode*>& lhs,
                            const std::vector<const INode*>& rhs) {
  if (lhs.size() != rhs.size())
    return false;

  std::vector<bool> used;
  used.resize(rhs.size());
  for (size_t i = 0; i < lhs.size(); ++i) {
    bool equal_found = false;
    for (size_t j = 0; j < rhs.size(); ++j) {
      if (used[j])
        continue;
      if (!lhs[i]->IsEqual(rhs[j]))
        continue;
      used[j] = true;
      equal_found = true;
      break;
    }
    if (!equal_found)
      return false;
  }
  return true;
}

bool IsNodesTransitiveEqual(const std::vector<std::unique_ptr<INode>*>& lhs,
                            const std::vector<std::unique_ptr<INode>*>& rhs) {
  auto transform = [](const std::vector<std::unique_ptr<INode>*>& nodes)
      -> std::vector<const INode*> {
    std::vector<const INode*> result;
    result.resize(nodes.size());
    for (size_t i = 0; i < nodes.size(); ++i) {
      result[i] = nodes[i]->get();
    }
    return result;
  };
  return IsNodesTransitiveEqual(transform(lhs), transform(rhs));
}

bool IsNodesTransitiveEqual(const std::vector<std::unique_ptr<INode>>& lhs,
                            const std::vector<std::unique_ptr<INode>>& rhs) {
  auto transform = [](const std::vector<std::unique_ptr<INode>>& nodes)
      -> std::vector<const INode*> {
    std::vector<const INode*> result;
    result.resize(nodes.size());
    for (size_t i = 0; i < nodes.size(); ++i) {
      result[i] = nodes[i].get();
    }
    return result;
  };
  return IsNodesTransitiveEqual(transform(lhs), transform(rhs));
}

std::vector<std::unique_ptr<INode>> TakeEqualNodes(
    std::vector<std::unique_ptr<INode>>* lhs,
    std::vector<std::unique_ptr<INode>>* rhs) {
  std::vector<std::unique_ptr<INode>> result;

  for (size_t i = 0; i < lhs->size(); ++i) {
    if (!(*lhs)[i])
      continue;
    for (size_t j = 0; j < rhs->size(); ++j) {
      if (!(*rhs)[j])
        continue;
      if (!(*lhs)[i]->IsEqual((*rhs)[j].get()))
        continue;
      result.push_back(std::move((*lhs)[i]));
      (*lhs)[i].reset();
      (*rhs)[j].reset();
      break;
    }
  }
  return result;
}

std::vector<std::unique_ptr<INode>> RemoveEqualNodes(
    const std::vector<std::unique_ptr<INode>>& lhs,
    std::vector<std::unique_ptr<INode>>* rhs) {
  for (size_t i = 0; i < lhs.size(); ++i) {
    for (size_t j = 0; j < rhs->size(); ++j) {
      if (!(*rhs)[j])
        continue;
      if (lhs[i]->IsEqual((*rhs)[j].get())) {
        (*rhs)[j].reset();
        break;
      }
    }
  }
  INodeHelper::RemoveEmptyOperands(rhs);
  return std::move(*rhs);
}

void ExctractNodesWithOp(Op op,
                         std::vector<std::unique_ptr<INode>>* src,
                         std::vector<std::unique_ptr<INode>>* nodes) {
  assert(!src->empty());
  for (auto& node : *src) {
    ExctractNodesWithOp(op, std::move(node), nodes);
  }
  assert(!nodes->empty());
}

void ExctractNodesWithOp(Op op,
                         std::unique_ptr<INode> src,
                         std::vector<std::unique_ptr<INode>>* nodes) {
  std::vector<std::unique_ptr<INode>> positive_nodes;
  std::vector<std::unique_ptr<INode>> negative_nodes;
  ExctractNodesWithOp(op, std::move(src), &positive_nodes, &negative_nodes);
  nodes->reserve(nodes->size() + positive_nodes.size() + negative_nodes.size());
  for (auto& node : positive_nodes) {
    nodes->push_back(std::move(node));
  }
  for (auto& node : negative_nodes) {
    nodes->push_back(INodeHelper::Negate(std::move(node)));
  }
}

void ExctractNodesWithOp(Op op,
                         std::unique_ptr<INode> src,
                         std::vector<std::unique_ptr<INode>>* positive_nodes,
                         std::vector<std::unique_ptr<INode>>* negative_nodes) {
  Operation* operation = INodeHelper::AsOperation(src.get());
  if (!operation) {
    positive_nodes->push_back(std::move(src));
    return;
  }
  if (operation->op() == Op::UnMinus) {
    if (op == Op::Mult) {
      ExctractNodesWithOp(op, operation->TakeOperand(0), positive_nodes,
                          negative_nodes);
      negative_nodes->push_back(std::move(positive_nodes->back()));
      positive_nodes->pop_back();
    } else {
      ExctractNodesWithOp(op, operation->TakeOperand(0), negative_nodes,
                          positive_nodes);
    }
    return;
  }
  if (operation->op() != op) {
    positive_nodes->push_back(std::move(src));
    return;
  }
  for (size_t i = 0; i < operation->OperandsCount(); ++i) {
    ExctractNodesWithOp(op, operation->TakeOperand(i), positive_nodes,
                        negative_nodes);
  }
}

bool MergeCanonicToPlus(HotToken& token,
                        const CanonicMult& lh,
                        const CanonicMult& rh,
                        std::unique_ptr<INode>* lh_node,
                        std::unique_ptr<INode>* rh_node) {
  bool is_tr_equal = IsNodesTransitiveEqual(lh.nodes, rh.nodes);
  if (!is_tr_equal)
    return false;

  double dividend = lh.a * rh.b + rh.a * lh.b;
  double divider = lh.b * rh.b;
  if (dividend == 0.0) {
    lh_node->reset();
    rh_node->reset();
    return true;
  }
  *lh_node = INodeHelper::MakeMult(dividend, divider, lh.nodes);
  rh_node->reset();
  return true;
}

bool MergeCanonicToMult(HotToken& token,
                        const CanonicMult& lh,
                        const CanonicMult& rh,
                        std::unique_ptr<INode>* lh_node,
                        std::unique_ptr<INode>* rh_node) {
  bool is_tr_equal = IsNodesTransitiveEqual(lh.nodes, rh.nodes);
  if (!is_tr_equal)
    return false;

  double dividend = lh.a * rh.a;
  double divider = lh.b * rh.b;
  if (dividend == 0.0) {
    lh_node->reset();
    rh_node->reset();
    return true;
  }

  std::vector<std::unique_ptr<INode>> base_nodes;
  for (auto node : lh.nodes)
    base_nodes.push_back(std::move(*node));

  auto pow =
      INodeHelper::MakePow(INodeHelper::MakeMultIfNeeded(std::move(base_nodes)),
                           INodeHelper::MakeConst(2.0));
  std::unique_ptr<INode> new_pow;
  pow->SimplifyImpl({&token}, &new_pow);
  if (!new_pow)
    new_pow = std::move(pow);
  if (dividend == divider) {
    *lh_node = std::move(new_pow);
    rh_node->reset();
  } else {
    *lh_node = INodeHelper::MakeConst(dividend / divider);
    *rh_node = std::move(new_pow);
  }

  return true;
}

bool MergeCanonicToPow(HotToken& token,
                       CanonicPow lh,
                       CanonicPow rh,
                       std::vector<std::unique_ptr<INode>>* top,
                       std::vector<std::unique_ptr<INode>>* bottom) {
  struct NodeAndExp {
    NodeAndExp(double exp, std::unique_ptr<INode> node)
        : exp(exp), node(std::move(node)) {}
    double exp;
    std::unique_ptr<INode> node;
  };
  std::vector<NodeAndExp> merged_nodes;

  for (auto& lh_node_info : lh.base_nodes) {
    if (!lh_node_info.node)
      continue;

    for (auto& rh_node_info : rh.base_nodes) {
      if (!rh_node_info.node)
        continue;

      auto* lh_pow = INodeHelper::AsPow(lh_node_info.node->get());
      auto* rh_pow = INodeHelper::AsPow(rh_node_info.node->get());
      if (lh_pow && rh_pow && lh_pow->Base()->IsEqual(rh_pow->Base())) {
        // x^a * x^b
        auto new_pow = INodeHelper::MakePow(
            lh_pow->Base()->Clone(),
            INodeHelper::MakePlus(lh_pow->TakeOperand(PowOperation::PowIndex),
                                  rh_pow->TakeOperand(PowOperation::PowIndex)));
        merged_nodes.emplace_back(1.0, std::move(new_pow));
        lh_node_info.node = nullptr;
        rh_node_info.node = nullptr;
        break;
      }
      if (lh_pow && !rh_pow &&
          lh_pow->Base()->IsEqual(rh_node_info.node->get())) {
        // x^a * x
        auto new_pow = INodeHelper::MakePow(
            lh_pow->Base()->Clone(),
            INodeHelper::MakePlus(lh_pow->TakeOperand(PowOperation::PowIndex),
                                  INodeHelper::MakeConst(1.0)));
        merged_nodes.emplace_back(1.0, std::move(new_pow));
        lh_node_info.node = nullptr;
        rh_node_info.node = nullptr;
        break;
      }
      if (rh_pow && !lh_pow &&
          rh_pow->Base()->IsEqual(lh_node_info.node->get())) {
        // x * x^b
        auto new_pow = INodeHelper::MakePow(
            rh_pow->Base()->Clone(),
            INodeHelper::MakePlus(rh_pow->TakeOperand(PowOperation::PowIndex),
                                  INodeHelper::MakeConst(1.0)));
        merged_nodes.emplace_back(1.0, std::move(new_pow));
        lh_node_info.node = nullptr;
        rh_node_info.node = nullptr;
        break;
      }

      if (lh_node_info.node->get()->IsEqual(rh_node_info.node->get())) {
        // x^3 * x^2
        merged_nodes.emplace_back(lh_node_info.exp + rh_node_info.exp,
                                  std::move(*lh_node_info.node));
        lh_node_info.node = nullptr;
        rh_node_info.node = nullptr;
        break;
      }

      auto* lh_const = INodeHelper::AsConstant(lh_node_info.node->get());
      if (!lh_const)
        continue;
      auto* rh_const = INodeHelper::AsConstant(rh_node_info.node->get());
      if (!rh_const)
        continue;
      double new_lh = 0;
      double new_rh = 0;
      if (ReduceFullMultiplicity(lh_const->Value(), rh_const->Value(), &new_lh,
                                 &new_rh)) {
        if (new_lh != 1.0) {
          merged_nodes.emplace_back(lh_node_info.exp,
                                    INodeHelper::MakeConst(new_lh));
        }
        if (new_rh != 1.0) {
          merged_nodes.emplace_back(rh_node_info.exp,
                                    INodeHelper::MakeConst(new_rh));
        }
        lh_node_info.node = nullptr;
        rh_node_info.node = nullptr;
        break;
      }
    }
  }
  if (merged_nodes.empty())
    return false;

  auto pow_node_maker = [](const CanonicPow& canonic,
                           std::vector<NodeAndExp>* new_pows) {
    for (auto& node_info : canonic.base_nodes) {
      if (!node_info.node)
        continue;
      new_pows->emplace_back(node_info.exp, std::move(*node_info.node));
    }
  };

  pow_node_maker(lh, &merged_nodes);
  pow_node_maker(rh, &merged_nodes);
  for (auto& node_and_exp : merged_nodes) {
    bool to_bottom = bottom && node_and_exp.exp < 0;
    if (to_bottom)
      node_and_exp.exp *= -1.0;
    auto new_node = INodeHelper::MakePowIfNeeded(std::move(node_and_exp.node),
                                                 node_and_exp.exp);
    if (new_node)
      (to_bottom ? bottom : top)->push_back(std::move(new_node));
  }

  return true;
}

void ReorderOperands(std::vector<std::unique_ptr<INode>>* operands,
                     bool move_const_to_front) {
  {
    auto order_by_name = [](const std::unique_ptr<INode>& lh,
                            const std::unique_ptr<INode>& rh) {
      std::wstring lh_name = GetBaseName(lh.get());
      std::wstring rh_name = GetBaseName(rh.get());
      if (lh_name.empty() || rh_name.empty()) {
        return lh_name.empty() < rh_name.empty();
      }
      return lh_name < rh_name;
    };
    std::sort(operands->begin(), operands->end(), order_by_name);
  }
  {
    auto const_to_front = [move_const_to_front](
                              const std::unique_ptr<INode>& lh,
                              const std::unique_ptr<INode>& rh) {
      auto lh_as_const = INodeHelper::AsConstant(lh.get());
      if (lh_as_const && lh_as_const->IsNamed())
        lh_as_const = nullptr;
      auto rh_as_const = INodeHelper::AsConstant(rh.get());
      if (rh_as_const && rh_as_const->IsNamed())
        rh_as_const = nullptr;
      if (!lh_as_const && !rh_as_const)
        return false;
      if (lh_as_const && rh_as_const) {
        return lh_as_const->Value() < rh_as_const->Value();
      }

      return move_const_to_front ? lh_as_const > rh_as_const
                                 : lh_as_const < rh_as_const;
    };
    std::stable_sort(operands->begin(), operands->end(), const_to_front);
  }
  {
    auto i_to_back = [](const std::unique_ptr<INode>& lh,
                        const std::unique_ptr<INode>& rh) {
      auto lh_as_i = INodeHelper::AsImaginary(lh.get());
      auto rh_as_i = INodeHelper::AsImaginary(rh.get());
      return lh_as_i < rh_as_i;
    };
    std::stable_sort(operands->begin(), operands->end(), i_to_back);
  }
}

std::vector<std::unique_ptr<INode>> ExtractMultipliers(const INode* node) {
  std::vector<std::unique_ptr<INode>> result;
  if (auto* as_mult = INodeHelper::AsMult(node)) {
    result.reserve(as_mult->OperandsCount());
    for (size_t i = 0; i < as_mult->OperandsCount(); ++i) {
      result.push_back(as_mult->Operand(i)->Clone());
    }
  } else if (auto* as_pow = INodeHelper::AsPow(node)) {
    result.push_back(as_pow->Base()->Clone());
  } else if (auto* as_un_minus = INodeHelper::AsUnMinus(node)) {
    result.push_back(INodeHelper::MakeConst(-1.0));
    auto nodes = ExtractMultipliers(as_un_minus->Operand(0));
    for (auto& n : nodes)
      result.push_back(std::move(n));
  } else {
    result.push_back(node->Clone());
  }

  for (size_t i = 0, n = result.size(); i < n; ++i) {
    if (auto* as_const = INodeHelper::AsConstant(result[i].get())) {
      std::optional<double> max_f =
          Factorize(as_const->Value(), as_const->Value(), &result);
      if (max_f) {
        result[i] = std::move(result.back());
        result.pop_back();
      }
    }
  }
  return result;
}
