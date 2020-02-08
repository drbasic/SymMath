#pragma once
#include <memory>
#include <string>
#include <vector>

#include "INode.h"

class Operation : public INode {
 public:
  Operation(const OpInfo* op_info, std::unique_ptr<INode> lh);
  Operation(const OpInfo* op_info,
            std::unique_ptr<INode> lh,
            std::unique_ptr<INode> rh);
  Operation(const OpInfo* op_info,
            std::vector<std::unique_ptr<INode>> operands);

  std::unique_ptr<INode> SymCalc() const override;

 protected:
  std::string PrintImpl(bool ommit_front_minus) const override;
  int Priority() const override;
  bool HasFrontMinus() const override;
  bool CheckCircular(const INode* other) const override;
  bool SimplifyImpl(std::unique_ptr<INode>* new_node) override;
  bool IsUnMinus() const override;
  Operation* AsUnMinus() override;
  std::vector<std::unique_ptr<INode>> TakeOperands(Op op) override;

 private:
  void CheckIntegrity() const;
  bool SimplifyUnMinus(std::unique_ptr<INode>* new_node);
  bool SimplifyChain();
  bool SimplifyConsts(std::unique_ptr<INode>* new_node);
  void ConvertToPlus();

  std::string PrintUnMinus(bool ommit_front_minus) const;
  std::string PrintMinusPlusMultDiv() const;
  std::string PrintOperand(const INode* node, bool with_op) const;
  std::unique_ptr<INode> CalcUnMinus() const;
  std::unique_ptr<INode> CalcMinusPlusMultDiv() const;

  const OpInfo* op_info_;
  std::vector<std::unique_ptr<INode>> operands_;
};
