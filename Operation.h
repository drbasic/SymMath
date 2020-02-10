#pragma once
#include <memory>
#include <string>
#include <vector>

#include "INode.h"

struct CanonicMultDiv {
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

  static CanonicMultDiv GetCanonic(std::unique_ptr<INode>* node);

 protected:
  bool IsEqual(const INode* rh) const override;
  std::unique_ptr<INode> Clone() const override;
  PrintSize GetPrintSize(bool ommit_front_minus) const override;
  std::string PrintImpl(bool ommit_front_minus) const override;
  int Priority() const override;
  bool HasFrontMinus() const override;
  bool CheckCircular(const INode* other) const override;

  Operation* AsOperation() override;
  const Operation* AsOperation() const override;

  bool SimplifyImpl(std::unique_ptr<INode>* new_node) override;

 private:
  friend class INodeAcessor;
  bool IsUnMinus() const;

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

  CanonicMultDiv GetCanonicMult();
  CanonicMultDiv GetCanonicDiv();
  CanonicMultDiv GetCanonicUnMinus();
  bool ReduceFor(double val);

  std::vector<std::unique_ptr<INode>> TakeOperands(Op op);

  void RemoveEmptyOperands();

  PrintSize MeasureUnMinus(bool ommit_front_minus) const;
  std::string PrintUnMinus(bool ommit_front_minus) const;
  PrintSize MeasurMinusPlusMultDiv() const;
  std::string PrintMinusPlusMultDiv() const;
  PrintSize MeasureOperand(const INode* node, bool with_op) const;
  std::string PrintOperand(const INode* node, bool with_op) const;
  std::unique_ptr<INode> CalcUnMinus() const;
  std::unique_ptr<INode> CalcMinusPlusMultDiv() const;

  const OpInfo* op_info_;
  std::vector<std::unique_ptr<INode>> operands_;
};
