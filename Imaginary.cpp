#include "Imaginary.h"

#include <cassert>

Imaginary::Imaginary() {}

CompareResult Imaginary::Compare(const INode* rh) const {
  auto result = CompareType(rh);
  if (result != CompareResult::Equal)
    return result;
  const Imaginary* rh_imaginary = rh->AsNodeImpl()->AsImaginary();
  assert(rh_imaginary);
  return result;
}

std::unique_ptr<INode> Imaginary::Clone() const {
  return std::make_unique<Imaginary>();
}

std::unique_ptr<INode> Imaginary::SymCalc(SymCalcSettings settings) const {
  return std::make_unique<Imaginary>();
}

PrintSize Imaginary::Render(Canvas* canvas,
                            PrintBox print_box,
                            bool dry_run,
                            RenderBehaviour render_behaviour) const {
  return print_size_ = canvas->PrintAt(print_box, "i",
                                       render_behaviour.GetSubSuper(), dry_run);
}

PrintSize Imaginary::LastPrintSize() const {
  return print_size_;
}

void Imaginary::SimplifyImpl(HotToken token, std::unique_ptr<INode>* new_node) {
  token.Disarm();
}

void Imaginary::OpenBracketsImpl(HotToken token,
                                 std::unique_ptr<INode>* new_node) {
  token.Disarm();
}

void Imaginary::ConvertToComplexImpl(HotToken token,
                                     std::unique_ptr<INode>* new_node) {
  token.Disarm();
}