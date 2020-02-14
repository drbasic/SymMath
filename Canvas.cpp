#include "Canvas.h"

#include <algorithm>
#include <cassert>

namespace {
enum BracketsParts {
  Left,
  Right,
  TopLeft,
  Middle,
  TopRight,
  BottomLeft,
  BottomRight,
  Last,
};
const wchar_t kSquareBrackets[BracketsParts::Last + 1] = L"[]┌│┐└┘";
const wchar_t kDivider = L'─';
}  // namespace

//=============================================================================
PrintBox::PrintBox() = default;

PrintBox::PrintBox(size_t x,
                   size_t y,
                   size_t width,
                   size_t height,
                   size_t base_line)
    : x(x), y(y), width(width), height(height), base_line(base_line) {}

PrintBox::PrintBox(size_t x, size_t y, const PrintSize& print_size)
    : x(x),
      y(y),
      width(print_size.width),
      height(print_size.height),
      base_line(print_size.base_line) {}

PrintBox::PrintBox(const PrintBox& rh) = default;

PrintBox::PrintBox(PrintBox&& rh) = default;

PrintBox& PrintBox::operator=(const PrintBox& rh) = default;

PrintBox& PrintBox::operator=(PrintBox&& rh) = default;

PrintBox PrintBox ::Infinite() {
  return PrintBox(0, 0, 1000, 10000, 0);
}

PrintBox PrintBox::ShrinkLeft(size_t delta_width) const {
  PrintBox result(*this);
  assert(result.width >= delta_width);
  result.x += delta_width;
  result.width -= delta_width;
  return result;
}

//=============================================================================
bool PrintSize::operator==(const PrintSize& rh) const {
  return width == rh.width && height == rh.height && base_line == rh.base_line;
}

bool PrintSize::operator!=(const PrintSize& rh) const {
  return !((*this) == rh);
}

PrintSize PrintSize::GrowWidth(const PrintSize& other) const {
  PrintSize result(*this);
  if (other == PrintSize())
    return result;

  assert(other.height > other.base_line);
  if (other.base_line > result.base_line) {
    result.height += other.base_line - result.base_line;
    result.base_line = other.base_line;
  }
  result.height = std::max(result.height,
                           result.base_line - other.base_line + other.height);
  result.width += other.width;
  return result;
}

PrintSize PrintSize::GrowDown(const PrintSize& other,
                              bool move_base_line) const {
  PrintSize result(*this);
  if (move_base_line)
    result.base_line = result.height + other.base_line;
  result.height += other.height;
  result.width = std::max(result.width, other.width);
  return result;
}
//=============================================================================

void Canvas::Resize(const PrintSize& print_size) {
  print_size_ = print_size;
  size_t new_size = (print_size.width + 1) * print_size.height;
  data_.resize(new_size);
  std::fill(std::begin(data_), std::end(data_), ' ');
  for (size_t row = 0; row < print_size.height; ++row) {
    data_[GetIndex(print_size.width - 1, row) + 1] = '\n';
  }
}

const std::wstring& Canvas::ToString() const {
  assert(print_size_ != PrintSize{});
  return data_;
}

PrintSize Canvas::PrintAt(const PrintBox& print_box,
                          std::string_view str,
                          bool dry_run) {
  return PrintAt(print_box, std::wstring(str.begin(), str.end()), dry_run);
}

PrintSize Canvas::PrintAt(const PrintBox& print_box,
                          std::wstring_view str,
                          bool dry_run) {
  if (!dry_run) {
    assert(print_size_ != PrintSize{});
    assert(print_box.x + str.size() <= print_size_.width);
    size_t indx = GetIndex(print_box.x, print_box.base_line);
    for (size_t i = 0; i < str.size(); ++i)
      assert(data_[indx + i] == ' ');
    std::copy(std::begin(str), std::end(str), std::begin(data_) + indx);
  }
  return {str.size(), 1, 0};
}

PrintSize Canvas::RenderBracket(const PrintBox& print_box,
                                Bracket br,
                                size_t height,
                                bool dry_run) {
  if (height == 1) {
    if (!dry_run) {
      data_[GetIndex(print_box.x, print_box.y)] =
          br == Bracket::Left ? kSquareBrackets[BracketsParts::Left]
                              : kSquareBrackets[BracketsParts::Right];
    }
    return {1, 1, 0};
  }
  height += 2;
  if (!dry_run) {
    for (size_t i = 0; i < height; ++i) {
      wchar_t s = '?';
      if (i == 0) {
        s = br == Bracket::Left ? kSquareBrackets[BracketsParts::TopLeft]
                                : kSquareBrackets[BracketsParts::TopRight];
      } else if (i == height - 1) {
        s = br == Bracket::Left ? kSquareBrackets[BracketsParts::BottomLeft]
                                : kSquareBrackets[BracketsParts::BottomRight];
      } else {
        s = kSquareBrackets[BracketsParts::Middle];
      }
      data_[GetIndex(print_box.x + (br == Bracket::Left ? 0 : 1),
                     print_box.y + i)] = s;
    }
  }
  return {2, height, height / 2};
}

PrintSize Canvas::RenderDivider(const PrintBox& print_box,
                                size_t width,
                                bool dry_run) {
  return PrintAt(print_box, std::wstring(width, kDivider), dry_run);
}

void Canvas::SetDryRun(bool dry_run) {
  dry_run_ = dry_run;
}

size_t Canvas::GetIndex(size_t x, size_t y) const {
  assert(print_size_ != PrintSize{});
  assert(x < print_size_.width);
  assert(y < print_size_.height);
  size_t indx = y * (print_size_.width + 1) + x;
  return indx;
}
