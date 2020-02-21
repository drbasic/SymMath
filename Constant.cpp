#include "Constant.h"

#include <sstream>

Constant::Constant(double val) : value_(val) {}

PrintSize Constant::Render(Canvas* canvas,
                           PrintBox print_box,
                           bool dry_run,
                           RenderBehaviour render_behaviour) const {
  bool has_front_minus = HasFrontMinus();
  auto minus_behaviour = render_behaviour.TakeMinus();
  double for_print =
      ((has_front_minus && minus_behaviour == MinusBehaviour::Ommit) ||
       (!has_front_minus && minus_behaviour == MinusBehaviour::Force))
          ? -value_
          : value_;
  std::stringstream ss;
  ss << for_print;
  return print_size_ = canvas->PrintAt(print_box, ss.str(),
                                       render_behaviour.GetSubSuper(), dry_run);
}

PrintSize Constant::LastPrintSize() const {
  return print_size_;
}

bool Constant::HasFrontMinus() const {
  return value_ < 0;
}

bool Constant::IsEqual(const INode* rh) const {
  const Constant* rh_const = rh->AsNodeImpl()->AsConstant();
  return rh_const && (Value() == rh_const->Value());
}

std::unique_ptr<INode> Constant::SymCalc() const {
  return std::make_unique<Constant>(value_);
}

std::unique_ptr<INode> Constant::Clone() const {
  return std::make_unique<Constant>(value_);
}
