#include "IOperation.h"

void CanonicPow::Merge(CanonicPow rh) {
  base_nodes.reserve(base_nodes.size() + rh.base_nodes.size());
  for (auto& node_info : rh.base_nodes)
    base_nodes.push_back(std::move(node_info));
}

void CanonicPow::Add(double exp, std::unique_ptr<INode>* node) {
  base_nodes.push_back({exp, node});
}
