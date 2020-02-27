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
  Sqrt,
  Sin,
  Cos,
  Log,
  Equal,
  Diff,
};

struct OpInfo {
  using TrivialF = double (*)(double lh, double rh);
  using CalcF = std::unique_ptr<INode> (*)(
      const OpInfo* op,
      std::vector<std::unique_ptr<INode>>* operands);

  Op op;
  int priority = 0;
  std::wstring_view name;
  TrivialF trivial_f = nullptr;
  bool is_transitive = true;
  int operands_count = -1;
  CalcF calc_f = nullptr;
};

const OpInfo* GetOpInfo(Op op);
