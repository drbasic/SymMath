#include "Sequence.h"

#include "INodeHelper.h"

Sequence::Sequence() {}

bool Sequence::IsEqual(const INode* rh) const {
  auto as_seq = INodeHelper::AsSequence(rh);
  if (!as_seq)
    return false;
  return AbstractSequence::IsEqualSequence(as_seq);
}

std::unique_ptr<INode> Sequence::Clone() const {
  return AbstractSequence::Clone(std::make_unique<Sequence>());
}

std::unique_ptr<INode> Sequence::SymCalc(SymCalcSettings settings) const {
  return AbstractSequence::SymCalc(std::make_unique<Sequence>(), settings);
}

PrintSize Sequence::Render(Canvas* canvas,
                           PrintBox print_box,
                           bool dry_run,
                           RenderBehaviour render_behaviour) const {
  render_behaviour.TakeMinus();
  render_behaviour.TakeBrackets();
  return AbstractSequence::Render(PrintDirection::Horizontal, canvas, print_box,
                                  dry_run, render_behaviour);
}
