#include "MultOperation.h"

#include <algorithm>
#include <cassert>

#include "Constant.h"
#include "INodeHelper.h"
#include "OpInfo.h"

MultOperation::MultOperation(std::unique_ptr<INode> lh,
                             std::unique_ptr<INode> rh)
    : Operation(GetOpInfo(Op::Mult), std::move(lh), std::move(rh)) {}

MultOperation::MultOperation(std::vector<std::unique_ptr<INode>> operands)
    : Operation(GetOpInfo(Op::Mult), std::move(operands)) {}

std::unique_ptr<INode> MultOperation::Clone() const {
  std::vector<std::unique_ptr<INode>> new_nodes;
  new_nodes.reserve(operands_.size());
  for (const auto& op : operands_)
    new_nodes.push_back(op->Clone());
  return std::make_unique<MultOperation>(std::move(new_nodes));
}

PrintSize MultOperation::Render(Canvas* canvas,
                                PrintBox print_box,
                                bool dry_run,
                                RenderBehaviour render_behaviour) const {
  return print_size_ =
             RenderOperandChain(canvas, print_box, dry_run, render_behaviour);
}

bool MultOperation::HasFrontMinus() const {
  return false;
}

std::optional<CanonicMult> MultOperation::GetCanonic() {
  CanonicMult result;
  for (auto& op : operands_) {
    Constant* constant = op->AsConstant();
    if (constant)
      result.a = op_info_->trivial_f(result.a, constant->Value());
    else
      INodeHelper::MergeCanonic(&op, &result);
  }
  return result;
}
