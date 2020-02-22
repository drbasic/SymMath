#pragma once

#include <memory>
#include <vector>

enum class ValueType;
struct OpInfo;
class INode;

std::unique_ptr<INode> ScalarProduct(
    const OpInfo* op,
    std::vector<std::unique_ptr<INode>>* operands);

std::unique_ptr<INode> VectorProduct(
    const OpInfo* op,
    std::vector<std::unique_ptr<INode>>* operands);

ValueType GetMultResultType(ValueType lh, ValueType rh);