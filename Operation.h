#pragma once
#include <memory>
#include <string>
#include <vector>

#include "INode.h"

struct ConanicMultDiv {
  double a = 1;
  double b = 1;
  std::vector<std::unique_ptr<INode>*> nodes;
};

class Operation : public INode {
 public:
  Operation(const OpInfo* op_info, std::unique_ptr<INode> lh);
  Operation(const OpInfo* op_info,
            std::unique_ptr<INode> lh,
            std::unique_ptr<INode> rh);
  Operation(const OpInfo* op_info,
            std::vector<std::unique_ptr<INode>> operands);

  std::unique_ptr<INode> SymCalc() const override;
  std::unique_ptr<INode> Clone() const override;

 protected:
  std::string PrintImpl(bool ommit_front_minus) const override;
  int Priority() const override;
  bool HasFrontMinus() const override;
  bool CheckCircular(const INode* other) const override;
  bool IsEqual(const INode* rh) const override;
  bool IsUnMinus() const override;
  Operation* AsUnMinus() override;
  Operation* AsOperation() override;
  const Operation* AsOperation() const override;
  std::vector<std::unique_ptr<INode>> TakeOperands(Op op) override;

  bool Combine(Op op,
               const INode* node1,
               const INode* node2,
               std::unique_ptr<INode>* new_node1,
               std::unique_ptr<INode>* new_node2) const override;

  bool SimplifyImpl(std::unique_ptr<INode>* new_node) override;

 private:
  void CheckIntegrity() const;
  bool SimplifyUnMinus(std::unique_ptr<INode>* new_node);
  bool SimplifyChain();
  bool SimplifySame(std::unique_ptr<INode>* new_node);
  bool IsAllOperandsConst() const;
  bool SimplifyConsts(std::unique_ptr<INode>* new_node);
  bool NeedConvertToChain() const;
  void ConvertToPlus();
  void ConvertToPlus(std::vector<std::unique_ptr<INode>>* add_nodes,
                     std::vector<std::unique_ptr<INode>>* sub_nodes);
  ConanicMultDiv GetConanic(std::unique_ptr<INode>* node);
  ConanicMultDiv GetConanicMult();
  ConanicMultDiv GetConanicDiv();
  ConanicMultDiv GetConanicUnMinus();
  void RemoveEmptyOperands();

  std::string PrintUnMinus(bool ommit_front_minus) const;
  std::string PrintMinusPlusMultDiv() const;
  std::string PrintOperand(const INode* node, bool with_op) const;
  std::unique_ptr<INode> CalcUnMinus() const;
  std::unique_ptr<INode> CalcMinusPlusMultDiv() const;

  const OpInfo* op_info_;
  std::vector<std::unique_ptr<INode>> operands_;
};
