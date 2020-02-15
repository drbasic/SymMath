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
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
  PrintSize LastPrintSize() const override;
  int Priority() const override;
  bool HasFrontMinus() const override;
  bool CheckCircular(const INode* other) const override;

  Operation* AsOperation() override;
  const Operation* AsOperation() const override;

  bool SimplifyImpl(std::unique_ptr<INode>* new_node) override;

  PrintSize RenderOperand(const INode* node,
                          Canvas* canvas,
                          PrintBox print_box,
                          bool dry_run,
                          RenderBehaviour render_behaviour,
                          bool with_op) const;

  friend class INodeHelper;

  bool IsUnMinus() const;

  void CheckIntegrity() const;
  bool SimplifyUnMinus(std::unique_ptr<INode>* new_node);
  bool SimplifyDivExtractUnMinus(std::unique_ptr<INode>* new_node);
  bool SimplifyDivDiv();
  bool SimplifyDivMul();
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

  PrintSize RenderUnMinus(Canvas* canvas,
                          PrintBox print_box,
                          bool dry_run,
                          RenderBehaviour render_behaviour) const;
  PrintSize RenderMinusPlusMult(Canvas* canvas,
                                PrintBox print_box,
                                bool dry_run,
                                RenderBehaviour render_behaviour) const;
  PrintSize RenderDiv(Canvas* canvas,
                      PrintBox print_box,
                      bool dry_run,
                      RenderBehaviour render_behaviour) const;

  std::unique_ptr<INode> CalcUnMinus() const;
  std::unique_ptr<INode> CalcMinusPlusMultDiv() const;

  const OpInfo* op_info_;
  mutable PrintSize print_size_;
  std::vector<std::unique_ptr<INode>> operands_;
};
