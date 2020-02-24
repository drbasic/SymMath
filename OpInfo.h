#pragma once
#include <memory>
#include <string_view>
#include <vector>

class INode;
class Operation;

enum class Op : int {
  UnMinus = 0,
  Minus,
  Plus,
  Mult,
  VectorMult,
  Div,
  Pow,
  Sin,
  Cos,
  Ln,
  Equal,
  Diff,
};

struct OpInfo {
  Op op;
  int priority;
  std::wstring_view name;
  using TrivialF = double (*)(double lh, double rh);
  using CalcF = std::unique_ptr<INode> (*)(
      const OpInfo* op,
      std::vector<std::unique_ptr<INode>>* operands);

  TrivialF trivial_f;
  bool is_transitive = true;
  int operands_count = -1;
  CalcF calc_f;
};

const OpInfo* GetOpInfo(Op op);
