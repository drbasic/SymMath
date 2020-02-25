#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "HotToken.h"

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
std::vector<std::unique_ptr<INode>> TakeEqualNodes(
    std::vector<std::unique_ptr<INode>>* lhs,
    std::vector<std::unique_ptr<INode>>* rhs);
std::vector<std::unique_ptr<INode>> RemoveEqualNodes(
    const std::vector<std::unique_ptr<INode>>& lhs,
    std::vector<std::unique_ptr<INode>>* rhs);

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

bool MergeCanonicToPlus(HotToken& token,
                        const CanonicMult& lh,
                        const CanonicMult& rh,
                        std::unique_ptr<INode>* lh_node,
                        std::unique_ptr<INode>* rh_node);

bool MergeCanonicToMult(HotToken& token,
                        const CanonicMult& lh,
                        const CanonicMult& rh,
                        std::unique_ptr<INode>* lh_node,
                        std::unique_ptr<INode>* rh_node);

bool MergeCanonicToPow(HotToken& token,
                       CanonicPow lh,
                       CanonicPow rh,
                       std::vector<std::unique_ptr<INode>>* top,
                       std::vector<std::unique_ptr<INode>>* bottom);

void ReorderOperands(std::vector<std::unique_ptr<INode>>* operands,
                     bool move_const_to_front);

std::vector<std::unique_ptr<INode>> ExtractMultipliers(const INode* node);
