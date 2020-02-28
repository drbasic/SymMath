#pragma once

#include <memory>
#include <vector>

#include "INodeImpl.h"

class AbstractSequence : public INodeImpl {
 public:
  AbstractSequence();
  AbstractSequence(std::vector<std::unique_ptr<INode>> values);

  // INodeImpl interface
  PrintSize LastPrintSize() const override { return print_size_; }
  bool HasFrontMinus() const override { return false; }
  bool CheckCircular(const INodeImpl* other) const override;
  void SimplifyImpl(HotToken token, std::unique_ptr<INode>* new_node) override;
  void OpenBracketsImpl(HotToken token,
                        std::unique_ptr<INode>* new_node) override;
  void ConvertToComplexImpl(HotToken token,
                            std::unique_ptr<INode>* new_node) override;

  size_t Size() const { return values_.size(); }
  std::unique_ptr<INode> TakeValue(size_t indx);
  const INode* Value(size_t indx) const { return values_[indx].get(); }
  void Add(std::unique_ptr<INode> rh);

 protected:
  enum class PrintDirection {
    Horizontal,
    Vertical,
  };
  bool IsEqual(const AbstractSequence* rh) const;
  std::unique_ptr<AbstractSequence> SymCalc(
      std::unique_ptr<AbstractSequence> result,
      SymCalcSettings settings) const;

  PrintSize Render(PrintDirection direction,
                   Canvas* canvas,
                   PrintBox print_box,
                   bool dry_run,
                   RenderBehaviour render_behaviour) const;

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

  mutable PrintSize print_size_;
  mutable PrintSize values_print_size_;
  std::vector<std::unique_ptr<INode>> values_;
};
