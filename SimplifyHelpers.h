#pragma once

#include <memory>
#include <vector>

class INode;
struct CanonicMult;
struct CanonicPow;

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

bool MergeCanonicToNodesMult(const CanonicMult& lh,
                             const CanonicMult& rh,
                             std::unique_ptr<INode>* lh_node,
                             std::unique_ptr<INode>* rh_node);

bool MergeCanonicToPow( CanonicPow lh,
                        CanonicPow rh,
                       std::unique_ptr<INode>* node_1,
                       std::unique_ptr<INode>* node_2,
                       std::unique_ptr<INode>* node_3);