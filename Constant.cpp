#include "Constant.h"

#include <sstream>

Constant::Constant(double val) : value_(val) {}

Constant::Constant(bool val) : bool_value_(val), value_(val ? 1.0 : 0.0) {}

Constant::Constant(double val, std::wstring name)
    : value_(val), name_(std::move(name)) {}

PrintSize Constant::Render(Canvas* canvas,
                           PrintBox print_box,
                           bool dry_run,
                           RenderBehaviour render_behaviour) const {
  bool has_front_minus = HasFrontMinus();
  auto minus_behaviour = render_behaviour.TakeMinus();

  std::wstringstream ss;
  if (!bool_value_ && name_.empty()) {
    double for_print =
        ((has_front_minus && minus_behaviour == MinusBehaviour::Ommit) ||
         (!has_front_minus && minus_behaviour == MinusBehaviour::Force))
            ? -value_
            : value_;
    ss << for_print;
  } else if (!name_.empty()) {
    ss << name_;
  } else {
    ss << ((*bool_value_) ? L"True" : L"False");
  }
  return print_size_ = canvas->PrintAt(print_box, ss.str(),
                                       render_behaviour.GetSubSuper(), dry_run);
}

PrintSize Constant::LastPrintSize() const {
  return print_size_;
}

bool Constant::HasFrontMinus() const {
  return name_.empty() && !bool_value_ && value_ < 0;
}

void Constant::SimplifyImpl(HotToken token, std::unique_ptr<INode>* new_node) {
  token.Disarm();
}

void Constant::OpenBracketsImpl(HotToken token,
                                std::unique_ptr<INode>* new_node) {
  token.Disarm();
}

void Constant::ConvertToComplexImpl(HotToken token,
                                    std::unique_ptr<INode>* new_node) {
  token.Disarm();
}

CompareResult Constant::Compare(const INode* rh) const {
  auto result = CompareType(rh);
  if (result != CompareResult::Equal)
    return result;
  const Constant* rh_const = rh->AsNodeImpl()->AsConstant();
  result = CompareTrivial(Name(), rh_const->Name());
  if (result != CompareResult::Equal)
    return result;
  if (bool_value_) {
    result = CompareTrivial(*bool_value_, *rh_const->bool_value_);
    if (result != CompareResult::Equal)
      return result;
  }
  result = CompareTrivial(Value(), rh_const->Value());
  return result;
}

std::unique_ptr<INode> Constant::SymCalc(SymCalcSettings settings) const {
  return Clone();
}

std::unique_ptr<INode> Constant::Clone() const {
  if (!name_.empty())
    return std::make_unique<Constant>(value_, name_);
  if (bool_value_)
    return std::make_unique<Constant>(*bool_value_);
  return std::make_unique<Constant>(value_);
}
