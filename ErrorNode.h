#pragma once

#include "INode.h"
#include "INodeImpl.h"

class ErrorNode : public INodeImpl {
 public:
  ErrorNode(std::wstring error);

  // INode implementation
  CompareResult Compare(const INode* rh) const override;
  std::unique_ptr<INode> Clone() const override;
  std::unique_ptr<INode> SymCalc(SymCalcSettings settings) const override;

  // INodeImpl interface
  NodeType GetNodeType() const override { return NodeType::ErrorNode; }
  PrintSize Render(Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const override;
  PrintSize LastPrintSize() const override;
  int Priority() const override { return 0; }
  bool HasFrontMinus() const override { return false; }
  bool CheckCircular(const INodeImpl* other) const override { return false; }

  void SimplifyImpl(HotToken token, std::unique_ptr<INode>* new_node) override;
  void OpenBracketsImpl(HotToken token,
                        std::unique_ptr<INode>* new_node) override;
  void ConvertToComplexImpl(HotToken token,
                            std::unique_ptr<INode>* new_node) override;

 private:
  mutable PrintSize print_size_;
  std::wstring error_;
};
