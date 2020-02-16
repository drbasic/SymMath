#include "DivOperation.h"

#include <algorithm>
#include <cassert>

#include "Constant.h"
#include "INodeHelper.h"
#include "OpInfo.h"

DivOperation::DivOperation(std::unique_ptr<INode> top,
                           std::unique_ptr<INode> bottom)
    : Operation(GetOpInfo(Op::Div), std::move(top), std::move(bottom)) {}

std::unique_ptr<INode> DivOperation::Clone() const {
  return std::make_unique<DivOperation>(Operand(0)->Clone(),
                                        Operand(1)->Clone());
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
        canvas->PrintAt(print_box, GetOpInfo(Op::UnMinus)->name, dry_run);
    print_box = print_box.ShrinkLeft(un_minus_size.width + 1);
    prefix_size = un_minus_size.GrowWidth({1, 1, 0}, true);
  }

  auto lh_size = dry_run ? Operand(0)->Render(canvas, PrintBox::Infinite(),
                                              dry_run, render_behaviour)
                         : Operand(0)->LastPrintSize();
  auto rh_size = dry_run ? Operand(1)->Render(canvas, PrintBox::Infinite(),
                                              dry_run, render_behaviour)
                         : Operand(1)->LastPrintSize();

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
          Operand(0)->Render(canvas, lh_box, dry_run, render_behaviour);
      assert(lh_size2 == lh_size);
    }
    {
      // Render bottom
      PrintBox rh_box(print_box);
      rh_box.x = rh_box.x + (div_size.width - rh_size.width) / 2;
      rh_box.y = print_box.base_line + (div_size.height - div_size.base_line);
      rh_box.base_line = rh_box.y + rh_size.base_line;
      auto rh_size2 =
          Operand(1)->Render(canvas, rh_box, dry_run, render_behaviour);
      assert(rh_size2 == rh_size);
    }
  }

  lh_size = lh_size.GrowDown(div_size, true).GrowDown(rh_size, false);
  return print_size_ = prefix_size.GrowWidth(lh_size, true);
}

std::optional<CanonicMult> DivOperation::GetCanonic() {
  if (Constant* rh = Operand(1)->AsConstant()) {
    CanonicMult result;
    result.b = rh->Value();
    INodeHelper::MergeCanonic(&operands_[0], &result);
    return result;
  }
  return std::nullopt;
}

bool DivOperation::HasFrontMinus() const {
  bool lh_minus = Operand(0)->HasFrontMinus();
  bool rh_minus = Operand(1)->HasFrontMinus();
  return lh_minus ^ rh_minus;
}
