#pragma once

#include <memory>
#include <vector>

class INode;
struct CanonicMult;

bool IsNodesTransitiveEqual(const std::vector<const INode*>& lhs,
                            const std::vector<const INode*>& rhs);
bool IsNodesTransitiveEqual(const std::vector<std::unique_ptr<INode>*>& lhs,
                            const std::vector<std::unique_ptr<INode>*>& rhs);
bool IsNodesTransitiveEqual(const std::vector<std::unique_ptr<INode>>& lhs,
                            const std::vector<std::unique_ptr<INode>>& rhs);

std::vector<std::unique_ptr<INode>*> GetNodesPointers(
    std::vector<std::unique_ptr<INode>>& nodes,
    size_t skip);

double TryExctractSum(const CanonicMult& canonic,
                      std::vector<std::unique_ptr<INode>*> free_operands,
                      double* remains);

bool MergeCanonicToNodes(const CanonicMult& lh,
                         const CanonicMult& rh,
                         std::unique_ptr<INode>* lh_node,
                         std::unique_ptr<INode>* rh_node);
