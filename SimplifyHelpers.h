#pragma once

#include <memory>
#include <vector>

enum class Op;
struct CanonicMult;
struct CanonicPow;
class INode;

bool IsNodesTransitiveEqual(const std::vector<const INode*>& lhs,
                            const std::vector<const INode*>& rhs);
bool IsNodesTransitiveEqual(const std::vector<std::unique_ptr<INode>*>& lhs,
                            const std::vector<std::unique_ptr<INode>*>& rhs);
bool IsNodesTransitiveEqual(const std::vector<std::unique_ptr<INode>>& lhs,
                            const std::vector<std::unique_ptr<INode>>& rhs);

void ExctractNodesWithOp(Op op,
                         std::vector<std::unique_ptr<INode>>* src,
                         std::vector<std::unique_ptr<INode>>* nodes);
void ExctractNodesWithOp(Op op,
                         std::unique_ptr<INode> src,
                         std::vector<std::unique_ptr<INode>>* nodes);
void ExctractNodesWithOp(Op op,
                         std::unique_ptr<INode> src,
                         std::vector<std::unique_ptr<INode>>* positive_nodes,
                         std::vector<std::unique_ptr<INode>>* negative_nodes);

bool MergeCanonicToPlus(const CanonicMult& lh,
                        const CanonicMult& rh,
                        std::unique_ptr<INode>* lh_node,
                        std::unique_ptr<INode>* rh_node);

bool MergeCanonicToMult(const CanonicMult& lh,
                        const CanonicMult& rh,
                        std::unique_ptr<INode>* lh_node,
                        std::unique_ptr<INode>* rh_node);

bool MergeCanonicToPow(CanonicPow lh,
                       CanonicPow rh,
                       std::vector<std::unique_ptr<INode>>* top,
                       std::vector<std::unique_ptr<INode>>* bottom);