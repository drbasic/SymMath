#include "DivOperation.h"

#include <algorithm>
#include <cassert>

#include "Constant.h"
#include "INodeHelper.h"
#include "MultOperation.h"
#include "OpInfo.h"
#include "PlusOperation.h"
#include "SimplifyHelpers.h"
#include "UnMinusOperation.h"

DivOperation::DivOperation(std::unique_ptr<INode> top,
                           std::unique_ptr<INode> bottom)
    : Operation(GetOpInfo(Op::Div), std::move(top), std::move(bottom)) {}

std::unique_ptr<INode> DivOperation::Clone() const {
  return std::make_unique<DivOperation>(Dividend()->Clone(),
                                        Divider()->Clone());
}

PrintSize DivOperation::Render(Canvas* canvas,
                               PrintBox print_box,
                               bool dry_run,
                               RenderBehaviour render_behaviour) const {
  auto minus_behaviour = render_behaviour.TakeMinus();

  bool has_front_minus =
      (minus_behaviour == MinusBehaviour::Force) ||
      (minus_behaviour == MinusBehaviour::Relax && HasFrontMinus());

  render_behaviour.SetMunus(MinusBehaviour::Ommit);

  PrintSize prefix_size = {};
  if (has_front_minus) {
    auto un_minus_size =
        canvas->PrintAt(print_box, GetOpInfo(Op::UnMinus)->name,
                        render_behaviour.GetSubSuper(), dry_run);
    print_box = print_box.ShrinkLeft(un_minus_size.width + 1);
    prefix_size = un_minus_size.GrowWidth({1, 1, 0}, true);
  }

  auto lh_size = dry_run ? Dividend()->Render(canvas, PrintBox::Infinite(),
                                              dry_run, render_behaviour)
                         : Dividend()->LastPrintSize();
  auto rh_size = dry_run ? Divider()->Render(canvas, PrintBox::Infinite(),
                                             dry_run, render_behaviour)
                         : Divider()->LastPrintSize();
  if (dry_run) {
    assert(lh_size == Dividend()->LastPrintSize());
    assert(rh_size == Divider()->LastPrintSize());
  }
  // Render divider
  auto div_size = canvas->RenderDivider(
      print_box, std::max(lh_size.width, rh_size.width), dry_run);

  if (!dry_run) {
    {
      // Render top
      PrintBox lh_box(print_box);
      lh_box.x = lh_box.x + (div_size.width - lh_size.width) / 2;
      lh_box.height = print_box.base_line - div_size.base_line;
      lh_box.base_line = lh_box.height - (lh_size.height - lh_size.base_line);
      auto lh_size2 =
          Dividend()->Render(canvas, lh_box, dry_run, render_behaviour);
      assert(lh_size2 == lh_size);
      assert(lh_size == Dividend()->LastPrintSize());
    }
    {
      // Render bottom
      PrintBox rh_box(print_box);
      rh_box.x = rh_box.x + (div_size.width - rh_size.width) / 2;
      rh_box.y = print_box.base_line + (div_size.height - div_size.base_line);
      rh_box.base_line = rh_box.y + rh_size.base_line;
      auto rh_size2 =
          Divider()->Render(canvas, rh_box, dry_run, render_behaviour);
      assert(rh_size2 == rh_size);
      assert(rh_size == Divider()->LastPrintSize());
    }
  }

  lh_size = lh_size.GrowDown(div_size, true).GrowDown(rh_size, false);
  return print_size_ = prefix_size.GrowWidth(lh_size, true);
}

bool DivOperation::HasFrontMinus() const {
  bool lh_minus = Dividend()->HasFrontMinus();
  bool rh_minus = Divider()->HasFrontMinus();
  return lh_minus ^ rh_minus;
}

std::optional<CanonicMult> DivOperation::GetCanonicMult() {
  if (Constant* bottom_const = Divider()->AsConstant()) {
    CanonicMult result = INodeHelper::GetCanonicMult(operands_[0]);
    result.b *= bottom_const->Value();
    return result;
  }
  return std::nullopt;
}

std::optional<CanonicPow> DivOperation::GetCanonicPow() {
  CanonicPow dividend = INodeHelper::GetCanonicPow(operands_[0]);
  CanonicPow divider = INodeHelper::GetCanonicPow(operands_[1]);
  for (auto& pow_info : divider.base_nodes)
    pow_info.exp_up *= -1.0;
  dividend.Merge(std::move(divider));
  return dividend;
}

void DivOperation::SimplifyUnMinus(HotToken token,
                                   std::unique_ptr<INode>* new_node) {
  Operation::SimplifyUnMinus({&token}, nullptr);

  bool is_positve = true;
  for (auto& node : operands_) {
    if (auto* un_minus = INodeHelper::AsUnMinus(node.get())) {
      is_positve = !is_positve;
      node = INodeHelper::Negate(std::move(node));
    }
  }
  if (!is_positve) {
    *new_node = INodeHelper::MakeUnMinus(
        INodeHelper::MakeDiv(std::move(operands_[0]), std::move(operands_[1])));
  }
}

void DivOperation::SimplifyDivDiv(HotToken token) {
  Operation::SimplifyDivDiv({&token});

  auto* top = INodeHelper::AsDiv(Dividend());
  auto* bottom = INodeHelper::AsDiv(Divider());
  if (!top && !bottom) {
    return;
  }

  std::vector<std::unique_ptr<INode>> new_top;
  std::vector<std::unique_ptr<INode>> new_bottom;
  if (top) {
    ExctractNodesWithOp(Op::Mult, std::move(top->operands_[0]), &new_top);
    ExctractNodesWithOp(Op::Mult, std::move(top->operands_[1]), &new_bottom);
  } else {
    ExctractNodesWithOp(Op::Mult, std::move(operands_[0]), &new_top);
  }
  if (bottom) {
    ExctractNodesWithOp(Op::Mult, std::move(bottom->operands_[1]), &new_top);
    ExctractNodesWithOp(Op::Mult, std::move(bottom->operands_[0]), &new_bottom);
  } else {
    ExctractNodesWithOp(Op::Mult, std::move(operands_[1]), &new_bottom);
  }

  operands_[0] = INodeHelper::MakeMultIfNeeded(std::move(new_top));
  operands_[1] = INodeHelper::MakeMultIfNeeded(std::move(new_bottom));
}

void DivOperation::SimplifyConsts(HotToken token,
                                  std::unique_ptr<INode>* new_node) {
  Operation::SimplifyConsts({&token}, new_node);
  if (*new_node)
    return;

  if (Constant* top = INodeHelper::AsConstant(operands_[0].get())) {
    if (top->Value() == 0.0) {
      *new_node = std::move(operands_[0]);
      return;
    }
  }
  if (Constant* bottom = INodeHelper::AsConstant(operands_[1].get())) {
    if (bottom->Value() == 1.0) {
      *new_node = std::move(operands_[0]);
      return;
    }
    if (bottom->Value() == -1.0) {
      *new_node = INodeHelper::MakeUnMinus(std::move(operands_[0]));
      return;
    }
  }
}

void DivOperation::SimplifyTheSame(HotToken token,
                                   std::unique_ptr<INode>* new_node) {
  Operation::SimplifyTheSame({&token}, nullptr);

  SimplifyCanonicConstants(token, new_node);
  if (*new_node)
    return;

  SimplifyMultipliers(token, new_node);
}

void DivOperation::SimplifyCanonicConstants(HotToken& token,
                                            std::unique_ptr<INode>* new_node) {
  CanonicPow canonic_top = INodeHelper::GetCanonicPow(operands_[0]);
  CanonicPow canonic_bottom = INodeHelper::GetCanonicPow(operands_[1]);
  for (auto& node_info : canonic_bottom.base_nodes)
    node_info.exp_up *= -1.0;
  std::vector<std::unique_ptr<INode>> new_top_nodes;
  std::vector<std::unique_ptr<INode>> new_bottom_nodes;
  bool is_combined = MergeCanonicToPow(token, canonic_top, canonic_bottom,
                                       &new_top_nodes, &new_bottom_nodes);
  if (!is_combined)
    return;
  auto new_top = INodeHelper::MakeMultIfNeeded(std::move(new_top_nodes));
  if (new_bottom_nodes.empty()) {
    *new_node = std::move(new_top);
    return;
  }
  operands_[0] = std::move(new_top);
  operands_[1] = INodeHelper::MakeMultIfNeeded(std::move(new_bottom_nodes));
}

void DivOperation::SimplifyMultipliers(HotToken& token,
                                       std::unique_ptr<INode>* new_node) {
  auto* as_plus = INodeHelper::AsPlus(Dividend());
  if (!as_plus)
    return;

  auto divider_multipliers = ExtractMultipliers(Divider());
  for (size_t i = 0; i < as_plus->OperandsCount(); ++i) {
    auto multipliers = ExtractMultipliers(as_plus->Operand(i));
    divider_multipliers = TakeEqualNodes(&divider_multipliers, &multipliers);
    if (divider_multipliers.empty())
      return;
  }

  for (size_t i = 0; i < as_plus->OperandsCount(); ++i) {
    auto multipliers = ExtractMultipliers(as_plus->Operand(i));
    multipliers = RemoveEqualNodes(divider_multipliers, &multipliers);
    as_plus->SetOperand(i,
                        INodeHelper::MakeMultIfNeeded(std::move(multipliers)));
  }

  auto new_divider = ExtractMultipliers(Divider());
  new_divider = RemoveEqualNodes(divider_multipliers, &new_divider);
  SetOperand(OperandIndex::DividerIndex,
             INodeHelper::MakeMultIfNeeded(std::move(new_divider)));
}
