#include "PowOperation.h"

#include <algorithm>
#include <cassert>
#include <cmath>

#include "Constant.h"
#include "INodeHelper.h"
#include "Imaginary.h"
#include "MultOperation.h"
#include "OpInfo.h"
#include "ValueHelpers.h"

namespace {
constexpr size_t kMaxPowUnfold = 10;
}

PowOperation::PowOperation(std::unique_ptr<INode> lh, std::unique_ptr<INode> rh)
    : Operation(GetOpInfo(Op::Pow), std::move(lh), std::move(rh)) {}

std::unique_ptr<INode> PowOperation::MakeIfNeeded(std::unique_ptr<INode> base,
                                                  double exp) {
  auto pow = INodeHelper::MakePow(std::move(base), INodeHelper::MakeConst(exp));

  std::unique_ptr<INode> result;
  pow->SimplifyConsts({}, &result);
  if (!result)
    result = std::move(pow);

  return result;
}

std::unique_ptr<INode> PowOperation::Clone() const {
  return std::make_unique<PowOperation>(operands_[0]->Clone(),
                                        operands_[1]->Clone());
}

PrintSize PowOperation::Render(Canvas* canvas,
                               PrintBox print_box,
                               bool dry_run,
                               RenderBehaviour render_behaviour) const {
  auto base_render_behaviour = render_behaviour;
  render_behaviour.TakeMinus();
  auto* is_base_pow = INodeHelper::AsPow(Base());
  auto* is_exp_pow = INodeHelper::AsPow(Exp());
  if (Base()->Priority() < Priority() || is_base_pow)
    base_render_behaviour.SetBrackets(BracketsBehaviour::Force);
  auto exp_render_behaviour = render_behaviour;
  exp_render_behaviour.SetBrackets(is_exp_pow ? BracketsBehaviour::Force
                                              : BracketsBehaviour::Ommit);
  exp_render_behaviour.SetSubSuper(SubSuperBehaviour::Superscript);

  base_print_size_ = dry_run
                         ? RenderOperand(Base(), canvas, PrintBox::Infinite(),
                                         dry_run, base_render_behaviour, false)
                         : base_print_size_;

  pow_print_size_ = dry_run
                        ? RenderOperand(Exp(), canvas, PrintBox::Infinite(),
                                        dry_run, exp_render_behaviour, false)
                        : pow_print_size_;

  if (!dry_run) {
    {
      // Render base
      PrintBox base_box = print_box.ShrinkTop(pow_print_size_.height);
      auto base_size2 = RenderOperand(Base(), canvas, base_box, dry_run,
                                      base_render_behaviour, false);
      assert(base_size2 == base_print_size_);
    }
    {
      // Render exp
      PrintBox exp_box = print_box.ShrinkLeft(base_print_size_.width);
      exp_box.height = pow_print_size_.height;
      exp_box.y = print_box.base_line - base_print_size_.base_line -
                  pow_print_size_.height;
      exp_box.base_line = exp_box.y + pow_print_size_.base_line;
      auto exp_size2 = RenderOperand(Exp(), canvas, exp_box, dry_run,
                                     exp_render_behaviour, false);
      assert(exp_size2 == pow_print_size_);
    }
  }
  auto total_size = pow_print_size_.GrowDown(base_print_size_, true);
  total_size.width = pow_print_size_.width + base_print_size_.width;
  total_size.base_line = pow_print_size_.height + base_print_size_.base_line;
  return print_size_ = total_size;
}

void PowOperation::OpenBracketsImpl(HotToken token,
                                    std::unique_ptr<INode>* new_node) {
  Operation::OpenBracketsImpl({&token}, nullptr);

  auto* as_const = INodeHelper::AsConstant(Exp());
  if (!as_const || as_const->Value() > kMaxPowUnfold)
    return;
  auto exp = as_const->Value();
  double int_part;
  if (std::modf(exp, &int_part) != 0.0)
    return;
  int int_exp = static_cast<int>(exp);
  if (int_exp == 0) {
    *new_node = INodeHelper::MakeConst(1.0);
    return;
  }
  bool negative_exp = false;
  if (int_exp < 0) {
    int_exp = -int_exp;
    negative_exp = true;
  }

  operands_.resize(1);
  operands_.reserve(int_exp);
  for (int i = 1; i < int_exp; ++i) {
    operands_.push_back(Operand(0)->Clone());
  }
  std::unique_ptr<INode> new_temp_node;
  {
    auto temp_node = INodeHelper::MakeMultIfNeeded(std::move(operands_));
    temp_node->AsNodeImpl()->OpenBracketsImpl({&token}, &new_temp_node);
    if (!new_temp_node)
      new_temp_node = std::move(temp_node);
    if (negative_exp)
      new_temp_node = INodeHelper::Negate(std::move(new_temp_node));
  }
  *new_node = std::move(new_temp_node);
}

std::optional<CanonicPow> PowOperation::GetCanonicPow() {
  auto* exp_const = INodeHelper::AsConstant(operands_[1].get());
  if (!exp_const || exp_const->IsNamed())
    return std::nullopt;
  CanonicPow result = INodeHelper::GetCanonicPow(operands_[0]);
  for (auto& node_info : result.base_nodes)
    node_info.exp_up *= exp_const->Value();
  return result;
}

void PowOperation::SimplifyConsts(HotToken token,
                                  std::unique_ptr<INode>* new_node) {
  Operation::SimplifyConsts({&token}, new_node);
  if (*new_node)
    return;

  SimplifyExp(token, new_node);
}

void PowOperation::SimplifyExp(HotToken& token,
                               std::unique_ptr<INode>* new_node) {
  if (auto* exp_const = Exp()->AsConstant()) {
    if (exp_const->Value() == 0.0) {
      token.SetChanged();
      *new_node = INodeHelper::MakeConst(1.0);
      return;
    }
    if (exp_const->Value() == 1.0) {
      token.SetChanged();
      *new_node = TakeOperand(BaseIndex);
      return;
    }
  }

  if (!Base()->IsEqual(Constants::Imag()))
    return;
  if (auto* exp_const = Exp()->AsConstant()) {
    double remains_exp = exp_const->Value();
    bool negate = false;
    bool simplified = false;
    if (remains_exp >= 4.0) {
      remains_exp = std::fmod(remains_exp, 4.0);
      simplified = true;
    }
    if (remains_exp >= 2.0) {
      remains_exp -= 2;
      negate = true;
      simplified = true;
    }
    if (simplified) {
      auto node = INodeHelper::MakePowIfNeeded(Imag(), remains_exp);
      if (negate)
        node = INodeHelper::Negate(std::move(node));
      token.SetChanged();
      *new_node = std::move(node);
      return;
    }
  }
}
