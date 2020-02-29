#pragma once

#include <memory>
#include <vector>

#include "INodeImpl.h"

class AbstractSequence : public INodeImpl {
 public:
  AbstractSequence();
  AbstractSequence(std::vector<std::unique_ptr<INode>> values);

  // INode interface
  bool IsEqual(const INode* rh) const;
  CompareResult Compare(const INode* rh) const override;

  // INodeImpl interface
  PrintSize LastPrintSize() const override { return print_size_; }
  bool HasFrontMinus() const override { return false; }
  bool CheckCircular(const INodeImpl* other) const override;
  AbstractSequence* AsAbstractSequence() override { return this; }
  const AbstractSequence* AsAbstractSequence() const override { return this; }
  void SimplifyImpl(HotToken token, std::unique_ptr<INode>* new_node) override;
  void OpenBracketsImpl(HotToken token,
                        std::unique_ptr<INode>* new_node) override;
  void ConvertToComplexImpl(HotToken token,
                            std::unique_ptr<INode>* new_node) override;

  size_t Size() const { return values_.size(); }
  std::unique_ptr<INode> TakeValue(size_t indx);
  const INode* Value(size_t indx) const { return values_[indx].get(); }
  void AddValue(std::unique_ptr<INode> rh);
  void SetValue(size_t indx, std::unique_ptr<INode> node);
  void Unfold();

 protected:
  enum class PrintDirection {
    Horizontal,
    Vertical,
  };
  std::unique_ptr<AbstractSequence> Clone(
      std::unique_ptr<AbstractSequence> result) const;

  std::unique_ptr<AbstractSequence> SymCalc(
      std::unique_ptr<AbstractSequence> result,
      SymCalcSettings settings) const;

  PrintSize Render(PrintDirection direction,
                   Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const;

  std::vector<std::unique_ptr<INode>> values_;

 private:
  PrintSize RenderAllValues(PrintDirection direction,
                            Canvas* canvas,
                            PrintBox print_box,
                            bool dry_run,
                            RenderBehaviour render_behaviour) const;
  PrintSize RenderAllValuesH(Canvas* canvas,
                             PrintBox print_box,
                             bool dry_run,
                             RenderBehaviour render_behaviour) const;
  PrintSize RenderAllValuesV(Canvas* canvas,
                             PrintBox print_box,
                             bool dry_run,
                             RenderBehaviour render_behaviour) const;
  PrintSize RenderValue(const INode* value,
                        Canvas* canvas,
                        PrintBox print_box,
                        bool dry_run,
                        RenderBehaviour render_behaviour,
                        bool with_comma) const;

  void DoUnfold(std::vector<std::unique_ptr<INode>>* result);

  mutable PrintSize print_size_;
  mutable PrintSize values_print_size_;
};
