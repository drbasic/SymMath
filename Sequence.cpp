#include "Sequence.h"

#include <algorithm>

#include "INodeHelper.h"

Sequence::Sequence() {}

std::unique_ptr<INode> Sequence::Clone() const {
  return AbstractSequence::DoClone(std::make_unique<Sequence>());
}

std::unique_ptr<INode> Sequence::SymCalc(SymCalcSettings settings) const {
  return AbstractSequence::DoSymCalc(std::make_unique<Sequence>(), settings);
}

PrintSize Sequence::Render(Canvas* canvas,
                           PrintBox print_box,
                           bool dry_run,
                           RenderBehaviour render_behaviour) const {
  render_behaviour.TakeMinus();
  render_behaviour.TakeBrackets();
  return AbstractSequence::DoRender(PrintDirection::Horizontal, canvas,
                                    print_box, dry_run, render_behaviour);
}

void Sequence::Unique() {
  auto less_cmp = [](const std::unique_ptr<INode>& lh,
                     const std::unique_ptr<INode>& rh) {
    return lh->Compare(rh.get()) == CompareResult::Less;
  };
  std::sort(values_.begin(), values_.end(), less_cmp);
  auto eq_cmp = [](const std::unique_ptr<INode>& lh,
                   const std::unique_ptr<INode>& rh) {
    return lh->Compare(rh.get()) == CompareResult::Equal;
  };
  auto it = std::unique(values_.begin(), values_.end(), eq_cmp);
  values_.erase(it, values_.end());
}