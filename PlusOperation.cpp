#include "PlusOperation.h"

#include <algorithm>
#include <cassert>

#include "Constant.h"
#include "INodeHelper.h"
#include "OpInfo.h"
#include "UnMinusOperation.h"

PlusOperation::PlusOperation(std::unique_ptr<INode> lh,
                             std::unique_ptr<INode> rh)
    : Operation(GetOpInfo(Op::Plus), std::move(lh), std::move(rh)) {}

PlusOperation::PlusOperation(std::vector<std::unique_ptr<INode>> operands)
    : Operation(GetOpInfo(Op::Plus), std::move(operands)) {}

std::unique_ptr<INode> PlusOperation::Clone() const {
  std::vector<std::unique_ptr<INode>> new_nodes;
  new_nodes.reserve(operands_.size());
  for (const auto& op : operands_)
    new_nodes.push_back(op->Clone());
  return std::make_unique<PlusOperation>(std::move(new_nodes));
}

PrintSize PlusOperation::Render(Canvas* canvas,
                                PrintBox print_box,
                                bool dry_run,
                                RenderBehaviour render_behaviour) const {
  return print_size_ =
             RenderOperandChain(canvas, print_box, dry_run, render_behaviour);
}

bool PlusOperation::HasFrontMinus() const {
  return false;
}

void PlusOperation::UnfoldChains() {
  Operation::UnfoldChains();

  std::vector<std::unique_ptr<INode>> new_nodes;
  INodeHelper::ExctractNodesWithOp(Op::Plus, &operands_, &new_nodes);
  operands_.swap(new_nodes);
  CheckIntegrity();
}


void PlusOperation::SimplifyChains(std::unique_ptr<INode>* new_node) {
  //UnfoldChain();

  Operation::SimplifyChains(new_node);
}

void PlusOperation::SimplifyConsts(std::unique_ptr<INode>* new_node) {
  Operation::SimplifyConsts(new_node);
  if (*new_node)
    return;

  size_t const_count = 0;
  double total_summ = 0;
  std::unique_ptr<INode>* first_const = nullptr;
  for (auto& node : operands_) {
    Constant* constant = INodeHelper::AsConstant(node.get());
    if (!constant)
      continue;
    if (constant->Value() == 0.0) {
      node.reset();
      continue;
    }
    ++const_count;
    if (const_count == 1) {
      first_const = &node;
      total_summ = constant->Value();
      continue;
    }
    total_summ = op_info_->trivial_f(total_summ, constant->Value());
    if (first_const)
      first_const->reset();
    node.reset();
  }
  INodeHelper::RemoveEmptyOperands(&operands_);
  if (operands_.empty()) {
    *new_node = INodeHelper::MakeConst(total_summ);
    return;
  }
  if (const_count > 1 && total_summ != 0.0)
    operands_.push_back(INodeHelper::MakeConst(total_summ));
  if (operands_.size() == 1) {
    *new_node = std::move(operands_[0]);
    return;
  }
  CheckIntegrity();
}

