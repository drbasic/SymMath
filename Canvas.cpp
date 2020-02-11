#include "Canvas.h"

#include <algorithm>
#include <cassert>

bool PrintSize::operator==(const PrintSize& rh) const {
  return width == rh.width && height == rh.height && base_line == rh.base_line;
}

bool PrintSize::operator!=(const PrintSize& rh) const {
  return !((*this) == rh);
}

void PrintSize::Grow(size_t new_base_line, size_t new_height) {
  assert(new_height > new_base_line);
  if (new_base_line > base_line) {
    height += new_base_line - base_line;
    base_line = new_base_line;
  }
  height = std::max(height, base_line - new_base_line + new_height);
}
//=============================================================================

void Canvas::Resize(const PrintSize& print_size) {
  print_size_ = print_size;
  size_t new_size = (print_size.width + 1) * print_size.height;
  data_.resize(new_size);
  std::fill(std::begin(data_), std::end(data_), ' ');
  for (size_t row = 0; row < print_size.height; ++row) {
    data_[GetIndex({print_size.width - 1, row}) + 1] = '\n';
  }
}

std::string Canvas::ToString() const {
  assert(print_size_ != PrintSize{});
  if (data_.empty())
    return std::string();
  return std::string(std::begin(data_), std::end(data_));
}

void Canvas::PrintAt(const Position& pos, std::string_view str) {
  assert(print_size_ != PrintSize{});
  assert(!dry_run_);
  assert(pos.x + str.size() <= print_size_.width);
  size_t indx = GetIndex(pos);
  for (size_t i = 0; i < str.size(); ++i)
    assert(data_[indx + i] == ' ');
  std::copy(std::begin(str), std::end(str), std::begin(data_) + indx);
}

void Canvas::RenderBracket(const Position& pos, Bracket br, size_t height) {
  assert(print_size_ != PrintSize{});
  assert(!dry_run_);
  for (size_t i = 0; i < height; ++i)
    data_[GetIndex({pos.x, pos.y + i})] = br == Bracket::Left ? '(' : ')';
}

void Canvas::SetDryRun(bool dry_run) {
  dry_run_ = dry_run;
}

size_t Canvas::GetIndex(const Position& pos) const {
  assert(print_size_ != PrintSize{});
  assert(pos.x < print_size_.width);
  assert(pos.y < print_size_.height);
  size_t indx = pos.y * (print_size_.width + 1) + pos.x;
  return indx;
}