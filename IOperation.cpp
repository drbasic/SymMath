#include "IOperation.h"

void CanonicPow::Merge(CanonicPow rh) {
  base_nodes.reserve(base_nodes.size() + rh.base_nodes.size());
  for (auto& node_info : rh.base_nodes)
    base_nodes.push_back(std::move(node_info));
}

void CanonicPow::Add(double exp_up,
                     double exp_down,
                     std::unique_ptr<INode>* node) {
  base_nodes.push_back({exp_up, exp_down, node});
}

void CanonicPow::NodeInfo::Apply(double e_up, double e_down) {
  exp_up = (exp_up * e_down) + (e_up * exp_down);
  exp_down *= e_down;
}