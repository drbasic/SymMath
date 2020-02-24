#include "LogOperation.h"
#define _USE_MATH_DEFINES
#include <math.h>

#include <sstream>

#include "Constant.h"
#include "INodeHelper.h"

double TrivialLogCalc(double lh, double rh) {
  if (lh == M_E)
    return log(rh);
  if (lh == 2.0)
    return log2(rh);
  if (lh == 10.0)
    return log10(rh);
  return log(rh) / log(lh);
}

//=============================================================================
LogOperation::LogOperation(std::unique_ptr<INode> base,
                           std::unique_ptr<INode> value)
    : Operation(GetOpInfo(Op::Ln), std::move(base), std::move(value)) {}

std::unique_ptr<INode> LogOperation::Clone() const {
  return INodeHelper::MakeLog(Operand(0)->Clone(), Operand(1)->Clone());
}

PrintSize LogOperation::Render(Canvas* canvas,
                               PrintBox print_box,
                               bool dry_run,
                               RenderBehaviour render_behaviour) const {
  // Render "log".
  auto log_size = canvas->PrintAt(print_box, op_info_->name,
                                  render_behaviour.GetSubSuper(), dry_run);
  print_box = print_box.ShrinkLeft(log_size.width);

  {  // Render base.
    auto base_render_behaviour = render_behaviour;
    base_render_behaviour.SetSubSuper(SubSuperBehaviour::Subscript);
    auto base_print_box = print_box;
    base_print_box.y = print_box.base_line - log_size.base_line + log_size.height;
    base_print_box.base_line = base_print_box.y + base_print_size_.base_line;
    base_print_size_ =
        RenderBase(canvas, base_print_box, dry_run, base_render_behaviour);
    log_size = log_size.GrowWidth(base_print_size_, false)
                   .GrowDown(base_print_size_, false);
    print_box = print_box.ShrinkLeft(base_print_size_.width);
  }

  {
    // Render value.
    auto value_render_behaviour = render_behaviour;
    value_render_behaviour.SetBrackets(BracketsBehaviour::Force);
    auto value_size = RenderOperand(Operand(1), canvas, print_box, dry_run,
                                    value_render_behaviour, false);
    log_size = log_size.GrowWidth(value_size, true);
  }
  return print_size_ = log_size;
}

INodeImpl* LogOperation::Base() {
  return Operand(0);
}

const INodeImpl* LogOperation::Base() const {
  return Operand(0);
}

PrintSize LogOperation::RenderBase(Canvas* canvas,
                                   PrintBox print_box,
                                   bool dry_run,
                                   RenderBehaviour render_behaviour) const {
  const auto* as_const = Base()->AsConstant();
  if (as_const) {
    std::stringstream ss;
    if (as_const->Value() == M_E)
      ss << "e";
    else
      ss << as_const->Value();
    return canvas->PrintAt(print_box, ss.str(), render_behaviour.GetSubSuper(),
                           dry_run);
  }

  return RenderOperand(Base(), canvas, PrintBox::Infinite(), dry_run,
                       render_behaviour, false);
}
