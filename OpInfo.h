#pragma once
#include <memory>
#include <string_view>
#include <vector>

class INode;

enum class Op : int {
  UnMinus = 0,
  Minus,
  Plus,
  Mult,
  Div,
  Pow,
  Sin,
  Cos,
};

struct OpInfo {
  Op op;
  int priority;
  std::wstring_view name;
  using CalcF = std::unique_ptr<INode> (*)(
      const OpInfo* op,
      const std::vector<std::unique_ptr<INode>>& operands);
  using PrintF =
      std::string (*)(const OpInfo* op,
                      const std::vector<std::unique_ptr<INode>>& operands);
  using TrivialF = double (*)(double lh, double rh);

  TrivialF trivial_f;
  bool is_transitive = true;
  int operands_count = -1;
  CalcF calc_f;
  PrintF print_f;
};

const OpInfo* GetOpInfo(Op op);
