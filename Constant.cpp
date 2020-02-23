#include "Constant.h"

#include <sstream>

Constant::Constant(double val) : value_(val) {}

Constant::Constant(bool val) : bool_value_(val), value_(val ? 1.0 : 0.0) {}

PrintSize Constant::Render(Canvas* canvas,
                           PrintBox print_box,
                           bool dry_run,
                           RenderBehaviour render_behaviour) const {
  bool has_front_minus = HasFrontMinus();
  auto minus_behaviour = render_behaviour.TakeMinus();

  std::stringstream ss;
  if (!bool_value_) {
    double for_print =
        ((has_front_minus && minus_behaviour == MinusBehaviour::Ommit) ||
         (!has_front_minus && minus_behaviour == MinusBehaviour::Force))
            ? -value_
            : value_;
    ss << for_print;
  } else {
    ss << ((*bool_value_) ? "true" : "false");
  }
  return print_size_ = canvas->PrintAt(print_box, ss.str(),
                                       render_behaviour.GetSubSuper(), dry_run);
}

PrintSize Constant::LastPrintSize() const {
  return print_size_;
}

bool Constant::HasFrontMinus() const {
  return !bool_value_ && value_ < 0;
}

bool Constant::IsEqual(const INode* rh) const {
  const Constant* rh_const = rh->AsNodeImpl()->AsConstant();
  if (!rh_const)
    return false;
  if (bool_value_ != rh_const->bool_value_)
    return false;
  if (bool_value_)
    return (*bool_value_) == (*rh_const->bool_value_);
  return Value() == rh_const->Value();
}

std::unique_ptr<INode> Constant::SymCalc() const {
  return Clone();
}

std::unique_ptr<INode> Constant::Clone() const {
  if (bool_value_)
    return std::make_unique<Constant>(*bool_value_);
  return std::make_unique<Constant>(value_);
}
