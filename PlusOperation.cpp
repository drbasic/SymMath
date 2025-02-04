#include "PlusOperation.h"

#include <algorithm>
#include <cassert>

#include "Constant.h"
#include "DivOperation.h"
#include "INodeHelper.h"
#include "MultOperation.h"
#include "OpInfo.h"
#include "SimplifyHelpers.h"
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
  return INodeHelper::MakePlus(std::move(new_nodes));
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

void PlusOperation::UnfoldChains(HotToken token) {
  Operation::UnfoldChains({&token});

  auto operands_count = OperandsCount();
  std::vector<std::unique_ptr<INode>> new_nodes;
  ExctractNodesWithOp(Op::Plus, &operands_, &new_nodes);
  operands_.swap(new_nodes);
  if (operands_count != OperandsCount())
    token.SetChanged();
  CheckIntegrity();
}

void PlusOperation::SimplifyConsts(HotToken token,
                                   std::unique_ptr<INode>* new_node) {
  Operation::SimplifyConsts({&token}, new_node);
  if (*new_node)
    return;
  auto params_change_counter = token.CountParamsChanged(this);

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
}

void PlusOperation::SimplifyTheSame(HotToken token,
                                    std::unique_ptr<INode>* new_node) {
  Operation::SimplifyTheSame({&token}, nullptr);
  auto params_change_counter = token.CountParamsChanged(this);

  bool need_try = true;
  while (need_try) {
    need_try = false;
    for (size_t i = 0; i < operands_.size(); ++i) {
      if (!operands_[i])
        continue;
      CanonicMult canonic_1 = INodeHelper::GetCanonicMult(operands_[i]);
      if (canonic_1.nodes.empty()) {
        // skip constants.
        continue;
      }

      for (size_t j = i + 1; j < operands_.size(); ++j) {
        if (!operands_[j])
          continue;
        CanonicMult canonic_2 = INodeHelper::GetCanonicMult(operands_[j]);
        if (canonic_2.nodes.empty())
          continue;

        bool is_combined = MergeCanonicToPlus(token, canonic_1, canonic_2,
                                              &operands_[i], &operands_[j]);
        if (!operands_[i])
          break;
        if (is_combined) {
          need_try = true;
          canonic_1 = INodeHelper::GetCanonicMult(operands_[i]);
        }
      }
    }
    INodeHelper::RemoveEmptyOperands(&operands_);
    if (operands_.empty())
      break;
    SimplifyConsts({&token}, new_node);
    if (*new_node)
      return;
  }

  if (operands_.size() == 1) {
    *new_node = std::move(operands_[0]);
    return;
  }
  if (operands_.size() == 0) {
    *new_node = INodeHelper::MakeConst(0.0);
    return;
  }
}

void PlusOperation::OrderOperands(HotToken token) {
  Operation::OrderOperands({&token});

  ReorderOperands(&operands_, false);
}

void PlusOperation::OpenBracketsImpl(HotToken token,
                                     std::unique_ptr<INode>* new_node) {
  Operation::OpenBracketsImpl({&token}, nullptr);

  if (!INodeHelper::HasAnyOperation(Op::Div, operands_))
    return;
  auto params_change_counter = token.CountParamsChanged(this);

  std::vector<std::pair<size_t, std::unique_ptr<INode>>> dividers;
  for (size_t i = 0; i < OperandsCount(); ++i) {
    if (auto* as_div = INodeHelper::AsDiv(Operand(i))) {
      dividers.emplace_back(
          i, as_div->TakeOperand(DivOperation::OperandIndex::Divider));
    }
  }

  std::vector<std::unique_ptr<INode>> new_dividents;
  for (size_t i = 0; i < OperandsCount(); ++i) {
    std::unique_ptr<INode> node;
    if (auto* as_div = INodeHelper::AsDiv(Operand(i)))
      node = as_div->TakeOperand(DivOperation::OperandIndex::Dividend);
    else
      node = TakeOperand(i);

    std::vector<std::unique_ptr<INode>> dividents;
    dividents.reserve(dividers.size() + 1);
    dividents.push_back(std::move(node));
    for (auto& divider : dividers) {
      if (i == divider.first)
        continue;
      dividents.push_back(divider.second->Clone());
    }
    new_dividents.push_back(
        INodeHelper::MakeMultIfNeeded(std::move(dividents)));
  }
  std::vector<std::unique_ptr<INode>> new_dividerss;
  for (auto& divider : dividers) {
    new_dividerss.push_back(std::move(divider.second));
  }

  std::unique_ptr<INode> new_div = INodeHelper::MakeDiv(
      INodeHelper::MakePlusIfNeeded(std::move(new_dividents)),
      INodeHelper::MakeMultIfNeeded(std::move(new_dividerss)));
  new_div->AsNodeImpl()->OpenBracketsImpl({&token}, new_node);
  if (!*new_node)
    *new_node = std::move(new_div);
}