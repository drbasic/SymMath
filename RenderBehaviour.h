#pragma once

enum class MinusBehaviour {
  Relax,
  Ommit,
  Force,
};

enum class BracketsBehaviour {
  Relax,
  Ommit,
  Force,
};

class RenderBehaviour {
 public:
  MinusBehaviour TakeMinus();
  void SetMunus(MinusBehaviour minus_behaviour);

  BracketsBehaviour TakeBrackets();
  void SetBrackets(BracketsBehaviour brackets_behaviour);

 private:
  MinusBehaviour minus_behaviour_ = MinusBehaviour::Relax;
  BracketsBehaviour brackets_behaviour_ = BracketsBehaviour::Relax;
};