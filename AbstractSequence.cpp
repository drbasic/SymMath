#include "AbstractSequence.h"

#include <cassert>

#include "Brackets.h"

AbstractSequence::AbstractSequence() {}

AbstractSequence::AbstractSequence(std::vector<std::unique_ptr<INode>> values)
    : values_(std::move(values)) {}

PrintSize AbstractSequence::Render(PrintDirection direction,
                                   Canvas* canvas,
                                   PrintBox print_box,
                                   bool dry_run,
                                   RenderBehaviour render_behaviour) const {
  render_behaviour.TakeMinus();
  render_behaviour.TakeBrackets();

  if (dry_run) {
    values_print_size_ = RenderAllValues(direction, canvas, print_box, dry_run,
                                         render_behaviour);
  }

  PrintBox values_box;
  auto print_size = canvas->RenderBrackets(
      print_box,
      direction == PrintDirection::Vertical ? BracketType::Stright
                                            : BracketType::Round,
      values_print_size_, dry_run, &values_box);

  if (!dry_run) {
    auto values_print_size = RenderAllValues(direction, canvas, values_box,
                                             dry_run, render_behaviour);
    assert(values_print_size == values_print_size_);
    assert(print_size == print_size_);
  }

  return print_size_ = print_size;
}

bool AbstractSequence::CheckCircular(const INodeImpl* other) const {
  for (const auto& val : values_) {
    if (val->AsNodeImpl()->CheckCircular(other))
      return true;
  }
  return false;
}

void AbstractSequence::SimplifyImpl(HotToken token,
                                    std::unique_ptr<INode>* new_node) {
  for (auto& val : values_) {
    std::unique_ptr<INode> new_sub_node;
    val->AsNodeImpl()->SimplifyImpl({&token}, &new_sub_node);
    if (new_sub_node)
      val = std::move(new_sub_node);
  }
}

void AbstractSequence::OpenBracketsImpl(HotToken token,
                                        std::unique_ptr<INode>* new_node) {
  for (auto& val : values_) {
    std::unique_ptr<INode> temp_node;
    val->AsNodeImpl()->OpenBracketsImpl({&token}, &temp_node);
    if (temp_node)
      val = std::move(temp_node);
  }
}

void AbstractSequence::ConvertToComplexImpl(HotToken token,
                                            std::unique_ptr<INode>* new_node) {
  for (auto& node : values_) {
    std::unique_ptr<INode> temp_node;
    node->AsNodeImpl()->ConvertToComplexImpl({&token}, &temp_node);
    if (temp_node)
      node = std::move(temp_node);
  }
}

std::unique_ptr<INode> AbstractSequence::TakeValue(size_t indx) {
  return std::move(values_[indx]);
}

void AbstractSequence::AddValue(std::unique_ptr<INode> rh) {
  values_.push_back(std::move(rh));
}

void AbstractSequence::SetValue(size_t indx, std::unique_ptr<INode> node)
{
  assert(indx < values_.size());
  values_[indx] = std::move(node);
}

void AbstractSequence::Unfold() {
  std::vector<std::unique_ptr<INode>> new_values;
  DoUnfold(&new_values);
  values_.swap(new_values);
}

std::unique_ptr<AbstractSequence> AbstractSequence::Clone(
    std::unique_ptr<AbstractSequence> result) const {
  result->values_.reserve(Size());
  for (size_t i = 0; i < Size(); ++i)
    result->AddValue(Value(i)->Clone());
  return result;
}

bool AbstractSequence::IsEqualSequence(const AbstractSequence* rh) const {
  if (!rh)
    return false;
  if (values_.size() != rh->values_.size())
    return false;
  for (size_t i = 0; i < values_.size(); ++i) {
    if (!values_[i]->IsEqual(rh->values_[i].get()))
      return false;
  }
  return true;
}

std::unique_ptr<AbstractSequence> AbstractSequence::SymCalc(
    std::unique_ptr<AbstractSequence> result,
    SymCalcSettings settings) const {
  result->values_.reserve(values_.size());
  for (const auto& val : values_)
    result->values_.push_back(val->SymCalc(settings));
  return result;
}

PrintSize AbstractSequence::RenderAllValues(
    PrintDirection direction,
    Canvas* canvas,
    PrintBox print_box,
    bool dry_run,
    RenderBehaviour render_behaviour) const {
  if (direction == PrintDirection::Vertical) {
    return RenderAllValuesV(canvas, std::move(print_box), dry_run,
                            render_behaviour);
  }
  return RenderAllValuesH(canvas, std::move(print_box), dry_run,
                          render_behaviour);
}

PrintSize AbstractSequence::RenderAllValuesH(
    Canvas* canvas,
    PrintBox print_box,
    bool dry_run,
    RenderBehaviour render_behaviour) const {
  PrintSize total_print_size = {};
  PrintBox operand_box{print_box};
  for (size_t i = 0; i < values_.size(); ++i) {
    bool with_op = i != 0;
    auto operand_size = RenderValue(values_[i].get(), canvas, operand_box,
                                    dry_run, render_behaviour, with_op);
    render_behaviour.TakeMinus();
    render_behaviour.TakeBrackets();
    operand_box = operand_box.ShrinkLeft(operand_size.width);
    total_print_size = total_print_size.GrowWidth(operand_size, true);
  }
  return total_print_size;
}

PrintSize AbstractSequence::RenderAllValuesV(
    Canvas* canvas,
    PrintBox print_box,
    bool dry_run,
    RenderBehaviour render_behaviour) const {
  PrintSize values_print_size;
  for (size_t i = 0; i < values_.size(); ++i) {
    if (!dry_run) {
      print_box.base_line =
          print_box.y + values_[i]->AsNodeImpl()->LastPrintSize().base_line;
    }
    auto operand_size = values_[i]->AsNodeImpl()->Render(
        canvas, print_box, dry_run, render_behaviour);

    print_box = print_box.ShrinkTop(operand_size.height);
    values_print_size = values_print_size.GrowDown(operand_size, false);
  }
  return values_print_size;
}

PrintSize AbstractSequence::RenderValue(const INode* value,
                                        Canvas* canvas,
                                        PrintBox print_box,
                                        bool dry_run,
                                        RenderBehaviour render_behaviour,
                                        bool with_comma) const {
  PrintSize total_value_size;
  if (with_comma) {
    auto comma_size = canvas->PrintAt(print_box, ", ",
                                      render_behaviour.GetSubSuper(), dry_run);
    total_value_size = total_value_size.GrowWidth(comma_size, true);
    print_box = print_box.ShrinkLeft(comma_size.width);
  }

  // Render value
  auto valaue_size =
      value->AsNodeImpl()->Render(canvas, print_box, dry_run, render_behaviour);
  return total_value_size.GrowWidth(valaue_size, true);
}

void AbstractSequence::DoUnfold(std::vector<std::unique_ptr<INode>>* result) {
  for (auto& value : values_) {
    if (auto* seq = value->AsNodeImpl()->AsAbstractSequence()) {
      seq->DoUnfold(result);
    } else {
      result->push_back(std::move(value));
    }
  }
}
