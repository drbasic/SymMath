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
}  // namespace

bool PrintSize::operator==(const PrintSize& rh) const {
  return width == rh.width && height == rh.height && base_line == rh.base_line;
}

bool PrintSize::operator!=(const PrintSize& rh) const {
  return !((*this) == rh);
}

void PrintSize::GrowRight(const PrintSize& other) {
  if (other == PrintSize())
    return;
  assert(other.height > other.base_line);
  if (other.base_line > base_line) {
    height += other.base_line - base_line;
    base_line = other.base_line;
  }
  height = std::max(height, base_line - other.base_line + other.height);
  width += other.width;
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

std::wstring Canvas::ToString() const {
  assert(print_size_ != PrintSize{});
  if (data_.empty())
    return std::wstring();
  return std::wstring(std::begin(data_), std::end(data_));
}

PrintSize Canvas::PrintAt(const Position& pos,
                          std::string_view str,
                          bool dry_run) {
  if (!dry_run) {
    assert(print_size_ != PrintSize{});
    assert(pos.x + str.size() <= print_size_.width);
    size_t indx = GetIndex(pos);
    for (size_t i = 0; i < str.size(); ++i)
      assert(data_[indx + i] == ' ');
    std::copy(std::begin(str), std::end(str), std::begin(data_) + indx);
  }
  return {str.size(), 1, 0};
}

PrintSize Canvas::RenderBracket(const Position& pos,
                                Bracket br,
                                size_t height,
                                bool dry_run) {
  if (height == 1) {
    if (!dry_run) {
      data_[GetIndex(pos)] = br == Bracket::Left
                                 ? kSquareBrackets[BracketsParts::Left]
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
      data_[GetIndex({pos.x + (br == Bracket::Left ? 0 : 1), pos.y + i})] = s;
    }
  }
  return {2, height, height / 2};
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
