#include "SimplifyHelpers.h"

#include <algorithm>
#include <cmath>
#include <map>

#include "Constant.h"
#include "DivOperation.h"
#include "INodeHelper.h"
#include "IOperation.h"
#include "MultOperation.h"
#include "PowOperation.h"

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

void ExctractNodesWithOp(Op op,
                         std::vector<std::unique_ptr<INode>>* src,
                         std::vector<std::unique_ptr<INode>>* nodes) {
  for (auto& node : *src) {
    ExctractNodesWithOp(op, std::move(node), nodes);
  }
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
    ExctractNodesWithOp(op, operation->TakeOperand(0), negative_nodes,
                        positive_nodes);
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

bool MergeCanonicToPlus(const CanonicMult& lh,
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
  *lh_node = INodeHelper::MakeMult(dividend, divider, std::move(lh.nodes));
  rh_node->reset();
  return true;
}

bool MergeCanonicToMult(const CanonicMult& lh,
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

  auto base = INodeHelper::MakeMultIfNeeded(std::move(base_nodes));
  auto pow = INodeHelper::MakePow(std::move(base), INodeHelper::MakeConst(2.0));
  if (dividend == divider) {
    *lh_node = std::move(pow);
    rh_node->reset();
  } else {
    *lh_node = INodeHelper::MakeConst(dividend / divider);
    *rh_node = std::move(pow);
  }

  return true;
}

bool MergeCanonicToPow(CanonicPow lh,
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
    auto* lh_const = INodeHelper::AsConstant(lh_node_info.node->get());
    for (auto& rh_node_info : rh.base_nodes) {
      if (!rh_node_info.node)
        continue;
      if (lh_node_info.node->get()->IsEqual(rh_node_info.node->get())) {
        merged_nodes.emplace_back(lh_node_info.exp + rh_node_info.exp,
                                  std::move(*lh_node_info.node));
        lh_node_info.node = nullptr;
        rh_node_info.node = nullptr;
        break;
      }
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
