#include "RenderBehaviour.h"

#include <cassert>

MinusBehaviour RenderBehaviour::TakeMinus() {
  auto result = minus_behaviour_;
  minus_behaviour_ = MinusBehaviour::Relax;
  return result;
}

void RenderBehaviour::SetMunus(MinusBehaviour minus_behaviour) {
  assert(minus_behaviour == MinusBehaviour::Force ||
         minus_behaviour == MinusBehaviour::Ommit);
  minus_behaviour_ = minus_behaviour;
}

BracketsBehaviour RenderBehaviour::TakeBrackets() {
  auto result = brackets_behaviour_;
  brackets_behaviour_ = BracketsBehaviour::Relax;
  return result;
}

void RenderBehaviour::SetBrackets(BracketsBehaviour brackets_behaviour) {
  assert(brackets_behaviour == BracketsBehaviour::Force ||
         brackets_behaviour == BracketsBehaviour::Ommit);
  brackets_behaviour_ = brackets_behaviour;
}
