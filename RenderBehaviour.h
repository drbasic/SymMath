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

enum class SubSuperBehaviour {
  Normal,
  Subscript,
  Superscript,
};

enum class VariableBehaviour {
  Relax,
  NameOnly,
  ValueOnly,
};

class RenderBehaviour {
 public:
  MinusBehaviour TakeMinus();
  void SetMunus(MinusBehaviour minus_behaviour);

  BracketsBehaviour TakeBrackets();
  void SetBrackets(BracketsBehaviour brackets_behaviour);

  SubSuperBehaviour GetSubSuper() const { return sub_super_behaviour_; }
  void SetSubSuper(SubSuperBehaviour sub_super_behaviour);

  VariableBehaviour GetVariable() const { return variable_behaviour_; }
  void SetVariable(VariableBehaviour variable_behaviour);

 private:
  MinusBehaviour minus_behaviour_ = MinusBehaviour::Relax;
  BracketsBehaviour brackets_behaviour_ = BracketsBehaviour::Relax;
  SubSuperBehaviour sub_super_behaviour_ = SubSuperBehaviour::Normal;
  VariableBehaviour variable_behaviour_ = VariableBehaviour::Relax;
};