#include "Operation.h"

#include <algorithm>
#include <cassert>
#include <map>
#include <numeric>
#include <optional>
#include <sstream>

#include "Brackets.h"
#include "Constant.h"
#include "DivOperation.h"
#include "Exception.h"
#include "INodeHelper.h"
#include "MultOperation.h"
#include "OpInfo.h"
#include "Operation.h"
#include "Sequence.h"
#include "SimplifyHelpers.h"
#include "UnMinusOperation.h"
#include "ValueHelpers.h"

class HotTokenHelper {
 public:
  static void Disarm(HotToken* token) { token->Disarm(); }
};

namespace {

using SimplificatorFunc = void (*)(HotToken& token,
                                   Operation* operation,
                                   std::unique_ptr<INode>* new_node);
void ApplySimplification(HotToken token,
                         SimplificatorFunc simplificator,
                         std::vector<std::unique_ptr<INode>>* operands) {
  for (auto& node : *operands) {
    if (Operation* operation = INodeHelper::AsOperation(node.get())) {
      operation->CheckIntegrity();
      std::unique_ptr<INode> new_sub_node;
      simplificator(token, operation, &new_sub_node);
      if (new_sub_node)
        node = std::move(new_sub_node);
      if (Operation* op = INodeHelper::AsOperation(node.get()))
        op->CheckIntegrity();
    }
  }
  HotTokenHelper::Disarm(&token);
}

void ApplySimplifications(HotToken token,
                          const SimplificatorFunc* begin,
                          const SimplificatorFunc* end,
                          Operation* current,
                          std::unique_ptr<INode>* new_node) {
  current->CheckIntegrity();
  for (const SimplificatorFunc* it = begin; it != end; ++it) {
    std::unique_ptr<INode> temp_node;
    (*it)(token, current, &temp_node);
    if (temp_node) {
      *new_node = std::move(temp_node);
      current = INodeHelper::AsOperation(new_node->get());
      if (!current)
        break;
      it = begin;
    }
    current->CheckIntegrity();
  }
}

std::vector<std::unique_ptr<INode>> CalcOperands(
    SymCalcSettings settings,
    const std::vector<std::unique_ptr<INode>>& operands) {
  std::vector<std::unique_ptr<INode>> result;
  result.reserve(operands.size());
  for (const auto& operand : operands) {
    result.push_back(operand->SymCalc(settings));
  }
  return result;
}

}  // namespace

Operation::Operation(const OpInfo* op_info, std::unique_ptr<INode> lh)
    : op_info_(op_info) {
  operands_.push_back(std::move(lh));
  CheckIntegrity();
}

Operation::Operation(const OpInfo* op_info,
                     std::unique_ptr<INode> lh,
                     std::unique_ptr<INode> rh)
    : op_info_(op_info) {
  operands_.reserve(2);
  operands_.push_back(std::move(lh));
  operands_.push_back(std::move(rh));
  CheckIntegrity();
}

Operation::Operation(const OpInfo* op_info,
                     std::vector<std::unique_ptr<INode>> operands)
    : op_info_(op_info), operands_(std::move(operands)) {
  CheckIntegrity();
}

int Operation::Priority() const {
  return op_info_->priority;
}

std::unique_ptr<INode> Operation::SymCalc(SymCalcSettings settings) const {
  std::vector<std::unique_ptr<INode>> calculated_operands =
      CalcOperands(settings, operands_);
  auto result = SymCalcValue(std::move(calculated_operands), settings);
  if (auto* as_seq = INodeHelper::AsSequence(result.get())) {
    as_seq->Unfold();
  }
  return result;
}

std::unique_ptr<INode> Operation::SymCalcValue(
    std::vector<std::unique_ptr<INode>> calculated_operands,
    SymCalcSettings settings) const {
  auto clone_operands =
      [](const std::vector<std::unique_ptr<INode>>& operands) {
        std::vector<std::unique_ptr<INode>> result;
        result.reserve(operands.size());
        for (auto& operand : operands)
          result.push_back(operand->Clone());
        return result;
      };

  for (size_t i = 0; i < calculated_operands.size(); ++i) {
    if (auto* as_seq = INodeHelper::AsSequence(calculated_operands[i].get())) {
      std::unique_ptr<INode> seq = std::move(calculated_operands[i]);
      auto new_seq = INodeHelper::MakeSequence();
      for (size_t j = 0; j < as_seq->Size(); ++j) {
        calculated_operands[i] = as_seq->TakeValue(j);
        new_seq->AddValue(
            SymCalcValue(clone_operands(calculated_operands), settings));
      }
      return new_seq;
    }
  }

  if (op_info_->op == Op::Mult) {
    auto i_node = MultOperation::ProcessImaginary(&calculated_operands);
    if (i_node)
      return i_node;
  }

  if (op_info_->calc_f) {
    std::unique_ptr<INode> result =
        op_info_->calc_f(op_info_, &calculated_operands);
    if (result)
      return result;
  }

  if (!IsAllOperandsConst(settings, operands_) || !op_info_->trivial_f) {
    auto result = INodeHelper::MakeEmpty(op_info_->op);
    INodeHelper::AsOperation(result.get())->operands_.swap(calculated_operands);
    return result;
  }

  if (calculated_operands.size() == 1 && op_info_->operands_count == 1) {
    double result = op_info_->trivial_f(
        calculated_operands[0]->AsNodeImpl()->AsConstant()->Value(), 0.0);
    return INodeHelper::MakeConst(result);
  }

  if (calculated_operands.size() == 1 && op_info_->operands_count == -1) {
    return std::move(calculated_operands[0]);
  }

  if (calculated_operands.size() == 2 && op_info_->operands_count == 2) {
    double result = op_info_->trivial_f(
        calculated_operands[0]->AsNodeImpl()->AsConstant()->Value(),
        calculated_operands[1]->AsNodeImpl()->AsConstant()->Value());
    return INodeHelper::MakeConst(result);
  }

  auto trivial_f = op_info_->trivial_f;
  auto node_adaptor = [trivial_f](double lh, const std::unique_ptr<INode>& rh) {
    auto rh_val = rh->AsNodeImpl()->AsConstant();
    assert(rh_val);
    return trivial_f(lh, rh_val->Value());
  };

  double result = std::accumulate(
      std::begin(calculated_operands) + 1, std::end(calculated_operands),
      calculated_operands[0]->AsNodeImpl()->AsConstant()->Value(),
      node_adaptor);
  return INodeHelper::MakeConst(result);
}

PrintSize Operation::LastPrintSize() const {
  return print_size_;
}

bool Operation::CheckCircular(const INodeImpl* other) const {
  for (const auto& operand : operands_) {
    if (operand->AsNodeImpl()->CheckCircular(other))
      return true;
  }
  return false;
}

bool Operation::IsEqual(const INode* rh) const {
  const Operation* rh_op = rh->AsNodeImpl()->AsOperation();
  if (!rh_op)
    return false;
  if (op_info_ != rh_op->op_info_)
    return false;
  if (operands_.size() != rh_op->operands_.size())
    return false;
  if (!op_info_->is_transitive) {
    for (size_t i = 0; i < operands_.size(); ++i) {
      if (!operands_[i]->IsEqual(rh_op->operands_[i].get()))
        return false;
    }
    return true;
  }
  return IsNodesTransitiveEqual(operands_, rh_op->operands_);
}

void Operation::SimplifyImpl(HotToken token, std::unique_ptr<INode>* new_node) {
  SimplificatorFunc simplificators[] = {
      [](HotToken& token, Operation* current,
         std::unique_ptr<INode>* new_node) {
        current->SimplifyUnMinus({&token}, new_node);
      },
      [](HotToken& token, Operation* current,
         std::unique_ptr<INode>* new_node) {
        current->SimplifyDivDiv({&token});
      },
      [](HotToken& token, Operation* current,
         std::unique_ptr<INode>* new_node) { current->UnfoldChains({&token}); },
      [](HotToken& token, Operation* current,
         std::unique_ptr<INode>* new_node) {
        current->SimplifyChains({&token}, new_node);
      },
      [](HotToken& token, Operation* current,
         std::unique_ptr<INode>* new_node) {
        current->SimplifyDivMul({&token}, new_node);
      },
      [](HotToken& token, Operation* current,
         std::unique_ptr<INode>* new_node) {
        current->SimplifyDivDiv({&token});
      },
      [](HotToken& token, Operation* current,
         std::unique_ptr<INode>* new_node) {
        current->SimplifyConsts({&token}, new_node);
      },
      [](HotToken& token, Operation* current,
         std::unique_ptr<INode>* new_node) {
        current->SimplifyTheSame({&token}, new_node);
      },
      [](HotToken& token, Operation* current,
         std::unique_ptr<INode>* new_node) {
        current->SimplifyConsts({&token}, new_node);
      },
      [](HotToken& token, Operation* current,
         std::unique_ptr<INode>* new_node) {
        current->OrderOperands({&token});
      },
  };
  ApplySimplifications({&token}, std::begin(simplificators),
                       std::end(simplificators), this, new_node);
}

void Operation::OpenBracketsImpl(HotToken token,
                                 std::unique_ptr<INode>* new_node) {
  UnfoldChains({&token});

  for (auto& node : operands_) {
    std::unique_ptr<INode> temp_node;
    node->AsNodeImpl()->OpenBracketsImpl({&token}, &temp_node);
    if (temp_node)
      node = std::move(temp_node);
  }
}

void Operation::ConvertToComplexImpl(HotToken token,
                                     std::unique_ptr<INode>* new_node) {
  for (auto& node : operands_) {
    std::unique_ptr<INode> temp_node;
    node->AsNodeImpl()->ConvertToComplexImpl({&token}, &temp_node);
    if (temp_node)
      node = std::move(temp_node);
  }
}

void Operation::CheckIntegrity() const {
  assert(op_info_);
  if (op_info_->operands_count >= 0) {
    assert(op_info_->operands_count == static_cast<int>(operands_.size()));
  } else {
    switch (op_info_->op) {
      case Op::Plus:
        assert(operands_.size() > 1);
        break;
      case Op::Mult:
        assert(operands_.size() > 1);
        break;
      default:
        assert(false);
    }
  }
  for (const auto& operand : operands_) {
    assert(operand);
  }
}

bool Operation::IsAllOperandsConst(
    SymCalcSettings settings,
    const std::vector<std::unique_ptr<INode>>& operands) const {
  for (const auto& operand : operands) {
    Constant* constant = operand->AsNodeImpl()->AsConstant();
    if (!constant)
      return false;
    if (settings == SymCalcSettings::KeepNamedConstants && constant->IsNamed())
      return false;
  }
  return true;
}

void Operation::UnfoldChains(HotToken token) {
  ApplySimplification(
      {&token},
      [](HotToken& token, Operation* current,
         std::unique_ptr<INode>* new_node) { current->UnfoldChains({&token}); },
      &operands_);
}

void Operation::SimplifyUnMinus(HotToken token,
                                std::unique_ptr<INode>* new_node) {
  ApplySimplification(
      {&token},
      [](HotToken& token, Operation* operation,
         std::unique_ptr<INode>* new_node) {
        operation->SimplifyUnMinus({&token}, new_node);
      },
      &operands_);
}

void Operation::SimplifyChains(HotToken token,
                               std::unique_ptr<INode>* new_node) {
  ApplySimplification(
      {&token},
      [](HotToken& token, Operation* operation,
         std::unique_ptr<INode>* new_node) {
        operation->SimplifyChains({&token}, new_node);
      },
      &operands_);
  if (operands_.size() == 1 && op_info_->operands_count == -1)
    *new_node = std::move(operands_[0]);
}

void Operation::SimplifyDivDiv(HotToken token) {
  ApplySimplification(
      {&token},
      [](HotToken& token, Operation* current,
         std::unique_ptr<INode>* new_node) {
        current->SimplifyDivDiv({&token});
      },
      &operands_);
}

void Operation::SimplifyDivMul(HotToken token,
                               std::unique_ptr<INode>* new_node) {
  ApplySimplification(
      {&token},
      [](HotToken& token, Operation* operation,
         std::unique_ptr<INode>* new_node) {
        operation->SimplifyDivMul({&token}, new_node);
      },
      &operands_);
}

void Operation::SimplifyConsts(HotToken token,
                               std::unique_ptr<INode>* new_node) {
  ApplySimplification(
      {&token},
      [](HotToken& token, Operation* operation,
         std::unique_ptr<INode>* new_node) {
        operation->SimplifyConsts({&token}, new_node);
      },
      &operands_);
  auto settings = SymCalcSettings::KeepNamedConstants;
  if (IsAllOperandsConst(settings, operands_)) {
    *new_node = SymCalc(settings);
    return;
  }
}

void Operation::SimplifyTheSame(HotToken token,
                                std::unique_ptr<INode>* new_node) {
  ApplySimplification(
      {&token},
      [](HotToken& token, Operation* operation,
         std::unique_ptr<INode>* new_node) {
        operation->SimplifyTheSame({&token}, new_node);
      },
      &operands_);
}

void Operation::OrderOperands(HotToken token) {
  ApplySimplification(
      {&token},
      [](HotToken& token, Operation* operation,
         std::unique_ptr<INode>* new_node) {
        operation->OrderOperands({&token});
      },
      &operands_);
}

Op Operation::op() const {
  return op_info_->op;
}

size_t Operation::OperandsCount() const {
  return operands_.size();
}

std::unique_ptr<INode> Operation::TakeOperand(size_t indx) {
  return std::move(operands_[indx]);
}

void Operation::SetOperand(size_t indx, std::unique_ptr<INode> node) {
  operands_[indx] = std::move(node);
}

std::vector<std::unique_ptr<INode>> Operation::TakeAllOperands() {
  is_dead_ = true;
  return std::move(operands_);
}

INodeImpl* Operation::Operand(size_t indx) {
  return operands_[indx]->AsNodeImpl();
}

const INodeImpl* Operation::Operand(size_t indx) const {
  return operands_[indx]->AsNodeImpl();
}

PrintSize Operation::RenderOperandChain(
    Canvas* canvas,
    PrintBox print_box,
    bool dry_run,
    RenderBehaviour render_behaviour) const {
  //  1
  //  ~ + 1 -- base_line
  //  2       1
  //  ~~~~~ + ~ -- base_line
  //    b     3

  PrintSize total_print_size = {};
  PrintBox operand_box{print_box};
  for (size_t i = 0; i < operands_.size(); ++i) {
    bool with_op = i != 0;
    if (op_info_->op == Op::Mult && i == operands_.size() - 1 &&
        operands_[i]->AsNodeImpl()->AsImaginary()) {
      with_op = false;
    }
    auto operand_size = RenderOperand(Operand(i), canvas, operand_box, dry_run,
                                      render_behaviour, with_op);
    render_behaviour.TakeMinus();
    render_behaviour.TakeBrackets();
    operand_box = operand_box.ShrinkLeft(operand_size.width);
    total_print_size = total_print_size.GrowWidth(operand_size, true);
  }
  if (!dry_run) {
    assert(print_size_ == total_print_size);
  }
  return print_size_ = total_print_size;
}

PrintSize Operation::RenderOperand(const INodeImpl* node,
                                   Canvas* canvas,
                                   PrintBox print_box,
                                   bool dry_run,
                                   RenderBehaviour render_behaviour,
                                   bool with_op) const {
  auto brackets_behaviour = render_behaviour.TakeBrackets();

  bool need_br = (brackets_behaviour == BracketsBehaviour::Force) ||
                 ((brackets_behaviour != BracketsBehaviour::Ommit) &&
                  (node->AsNodeImpl()->Priority() < Priority()));
  if (brackets_behaviour != BracketsBehaviour::Ommit && with_op &&
      INodeHelper::AsMult(this) && node->HasFrontMinus()) {
    need_br = true;
  }
  if (brackets_behaviour != BracketsBehaviour::Force && !with_op &&
      INodeHelper::AsMult(this) && node->HasFrontMinus()) {
    // when un minus first in multyple, remove brackets. Remove brackets -a *
    // b ;  Keep brackets b * (-a);
    need_br = false;
  }

  auto op_to_print = op_info_;
  if (with_op && op_info_->op == Op::Plus && node->HasFrontMinus()) {
    // +-1 -> -1 // minus (-) print here, so ommit (-) in operand
    op_to_print = GetOpInfo(Op::Minus);
    render_behaviour.SetMunus(MinusBehaviour::Ommit);
  } else if (with_op && op_info_->op == Op::UnMinus &&
             INodeHelper::AsDiv(node)) {
    assert(!node->HasFrontMinus());
    assert(HasFrontMinus());
    // div operand print un minus instead of us
    with_op = false;
    render_behaviour.SetMunus(MinusBehaviour::Force);
  }

  PrintSize total_operand_size;
  if (with_op) {
    auto op_size = canvas->PrintAt(print_box, op_to_print->name,
                                   render_behaviour.GetSubSuper(), dry_run);
    total_operand_size = total_operand_size.GrowWidth(op_size, true);
    print_box = print_box.ShrinkLeft(op_size.width);
  }

  // Render operand
  auto node_size =
      need_br ? Brackets::RenderBrackets(node, BracketType::Round, canvas,
                                         print_box, dry_run, render_behaviour)
              : node->Render(canvas, print_box, dry_run, render_behaviour);
  if (!need_br)
    assert(node_size == node->LastPrintSize());
  total_operand_size = total_operand_size.GrowWidth(node_size, true);

  return total_operand_size;
}

//=============================================================================
