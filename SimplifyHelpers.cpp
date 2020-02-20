#include "SimplifyHelpers.h"

#include <algorithm>
#include <map>

#include "Constant.h"
#include "DivOperation.h"
#include "INodeHelper.h"
#include "IOperation.h"
#include "MultOperation.h"
#include "PowOperation.h"

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
      if (!INodeHelper::IsNodesEqual(lhs[i], rhs[j]))
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
/*
std::vector<std::unique_ptr<INode>*> GetNodesPointers(
    std::vector<std::unique_ptr<INode>>& nodes,
    size_t skip) {
  std::vector<std::unique_ptr<INode>*> result;
  for (size_t i = 0; i < nodes.size(); ++i) {
    if (i == skip)
      continue;
    if (nodes[i].get())
      result.push_back(&nodes[i]);
  }
  return result;
}

std::vector<std::unique_ptr<INode>*> GetNodesPointersWithoutConstant(
    const std::vector<std::unique_ptr<INode>*>& src,
    double* constant) {
  std::vector<std::unique_ptr<INode>*> result;
  for (auto node : src) {
    if (auto c = INodeHelper::AsConstant(node->get()))
      *constant += c->Value();
    else
      result.push_back(node);
  }
  return result;
}

bool TakeTransitiveEqualNodesTheSame(
    const std::vector<std::unique_ptr<INode>*>& lhs,
    const std::vector<std::unique_ptr<INode>*>& rhs) {
  std::vector<bool> used;
  used.resize(rhs.size());
  for (size_t i = 0; i < lhs.size(); ++i) {
    bool equal_found = false;
    for (size_t j = 0; j < rhs.size(); ++j) {
      if (used[j])
        continue;
      if (!INodeHelper::IsNodesEqual(lhs[i]->get(), rhs[i]->get()))
        continue;
      used[j] = true;
      equal_found = true;
      break;
    }
    if (!equal_found)
      return false;
  }
  for (size_t i = 0; i < used.size(); ++i) {
    if (used[i])
      rhs[i]->reset();
  }
  return true;
}

double TakeTransitiveEqualNodesCount(
    const std::vector<std::unique_ptr<INode>*>& lhs,
    const std::vector<std::unique_ptr<INode>*>& rhs) {
  std::vector<double> used;
  used.resize(rhs.size());
  for (size_t i = 0; i < lhs.size(); ++i) {
    bool equal_found = false;
    CanonicMult canonic_lh = INodeHelper::GetCanonic(lhs[i]);

    for (size_t j = 0; j < rhs.size(); ++j) {
      if (used[j])
        continue;
      CanonicMult canonic_rh = INodeHelper::GetCanonic(rhs[j]);
      bool eq = IsNodesTransitiveEqual(canonic_lh.nodes, canonic_rh.nodes);
      if (!eq)
        continue;
      used[j] = (canonic_rh.a * canonic_lh.b) / (canonic_lh.a * canonic_rh.b);
      equal_found = true;
      break;
    }
    if (!equal_found)
      return 0;
  }

  std::map<double, size_t> popular;
  for (size_t i = 0; i < used.size(); ++i) {
    if (used[i] != 0.0)
      popular[used[i]]++;
  }

  auto it =
      std::max_element(std::begin(popular), std::end(popular),
                       [](auto lh, auto rh) { return lh.second < rh.second; });
  double counter = it->first;
  for (size_t i = 0; i < used.size(); ++i) {
    if (used[i] == 0.0)
      continue;
    if (used[i] == counter) {
      rhs[i]->reset();
    } else {
      CanonicMult canonic_rh = INodeHelper::GetCanonic(rhs[i]);
      double dividend = (canonic_rh.a - counter) * canonic_rh.b;
      double divider = canonic_rh.a;
      *rhs[i] = INodeHelper::MakeMult(dividend, divider, {rhs[i]});
    }
  }
  return counter;
}

double TakeTransitiveEqualNodes(const std::vector<std::unique_ptr<INode>*>& lhs,
                                const std::vector<std::unique_ptr<INode>*>& rhs,
                                double* remains) {
  if (TakeTransitiveEqualNodesTheSame(lhs, rhs)) {
    remains = 0;
    return 1;
  }
  std::vector<std::unique_ptr<INode>*> without_consts =
      GetNodesPointersWithoutConstant(lhs, remains);
  double count = TakeTransitiveEqualNodesCount(without_consts, rhs);
  *remains *= -count;
  return count;
}

double TryExctractSum(const CanonicMult& canonic,
                      std::vector<std::unique_ptr<INode>*> free_operands,
                      double* remains) {
  if (canonic.nodes.size() != 1)
    return 0.0;
  Operation* canonic_op = INodeHelper::AsOperation(canonic.nodes[0]->get());
  if (!canonic_op)
    return 0.0;

  if (INodeHelper::GetOperands(canonic_op).size() > free_operands.size())
    return 0.0;

  double count = TakeTransitiveEqualNodes(
      GetNodesPointers(INodeHelper::GetOperands(canonic_op),
                       std::numeric_limits<size_t>::max()),
      free_operands, remains);
  return count;
}

*/
bool MergeCanonicToNodes(const CanonicMult& lh,
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

bool MergeCanonicToNodesMult(const CanonicMult& lh,
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
  auto pow = INodeHelper::MakePow(std::move(base), INodeHelper::MakeConst(2));
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
