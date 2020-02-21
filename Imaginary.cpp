#include "Imaginary.h"

Imaginary::Imaginary() {}

bool Imaginary::IsEqual(const INode* rh) const {
  const Imaginary* rh_img = rh->AsNodeImpl()->AsImaginary();
  return rh_img;
}

std::unique_ptr<INode> Imaginary::Clone() const {
  return std::make_unique<Imaginary>();
}

std::unique_ptr<INode> Imaginary::SymCalc() const {
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
