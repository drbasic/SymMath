#include "Canvas.h"

#include <algorithm>
#include <cassert>

#include "Brackets.h"

namespace {

enum BracketsParts {
  Left,
  Right,
  TopLeft,
  TopRight,
  Stright,
  BottomLeft,
  BottomRight,
  MiddleLeft,
  MiddleRight,
  Last,
};
const wchar_t kRoundBrackets[BracketsParts::Last + 1] = L"()╭╮│╰╯││";
const wchar_t kSquareBrackets[BracketsParts::Last + 1] = L"[]┌┐│└┘││";
const wchar_t kFigureBrackets[BracketsParts::Last + 1] = L"{}╭╮│╰╯╮╭";
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

PrintBox PrintBox::ShrinkTop(size_t delta_height) const {
  PrintBox result(*this);
  assert(result.height >= delta_height);
  result.y += delta_height;
  result.height -= delta_height;
  return result;
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

PrintSize PrintSize::GrowWidth(const PrintSize& other,
                               bool allign_base_line) const {
  PrintSize result(*this);
  if (other == PrintSize())
    return result;
  if (!allign_base_line) {
    result.height = std::max(result.height, other.height);
    result.width += other.width;
    return result;
  }
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

PrintSize Canvas::RenderBrackets(PrintBox print_box,
                                 BracketType bracket_type,
                                 PrintSize inner_size,
                                 bool dry_run,
                                 PrintBox* inner_print_box) {
  assert(inner_print_box);
  // Render Left bracket
  PrintSize left_br_size = RenderBracket(
      print_box, BracketSide::Left, bracket_type, inner_size.height, dry_run);
  print_box = print_box.ShrinkLeft(left_br_size.width);

  // Calculate PrintBox for inner value
  *inner_print_box = print_box;
  inner_print_box->y = print_box.base_line - left_br_size.height / 2 +
                       (left_br_size.height - inner_size.height) / 2;
  inner_print_box->height = inner_print_box->y + inner_size.height;
  inner_print_box->width = inner_size.width;
  inner_print_box->base_line = inner_print_box->y + inner_size.base_line;
  print_box = print_box.ShrinkLeft(inner_size.width);

  // Render right bracket
  PrintSize right_br_size = RenderBracket(
      print_box, BracketSide::Right, bracket_type, inner_size.height, dry_run);
  return left_br_size.GrowWidth(inner_size, false)
      .GrowWidth(right_br_size, false);
}

PrintSize Canvas::RenderBracket(const PrintBox& print_box,
                                BracketSide side,
                                BracketType bracket_type,
                                size_t height,
                                bool dry_run) {
  const wchar_t* brackets = nullptr;
  switch (bracket_type) {
    case BracketType::Round:
      brackets = kRoundBrackets;
      break;
    case BracketType::Square:
      brackets = kSquareBrackets;
      break;
    case BracketType::Fugure:
      brackets = kFigureBrackets;
      break;
  }
  if (height == 1) {
    if (!dry_run) {
      size_t y = print_box.base_line - height / 2;
      data_[GetIndex(print_box.x, y)] =
          (side == BracketSide::Left ? brackets[BracketsParts::Left]
                                     : brackets[BracketsParts::Right]);
    }
    return {1, 1, 0};
  }

  height += 2;

  if (!dry_run) {
    size_t y = print_box.base_line - height / 2;
    for (size_t i = 0; i < height; ++i) {
      wchar_t s = '?';
      if (i == 0) {
        s = side == BracketSide::Left ? brackets[BracketsParts::TopLeft]
                                      : brackets[BracketsParts::TopRight];
      } else if (i == height - 1) {
        s = side == BracketSide::Left ? brackets[BracketsParts::BottomLeft]
                                      : brackets[BracketsParts::BottomRight];
      } else if (i == height / 2) {
        s = side == BracketSide::Left ? brackets[BracketsParts::MiddleLeft]
                                      : brackets[BracketsParts::MiddleRight];
      } else {
        s = kSquareBrackets[BracketsParts::Stright];
      }
      data_[GetIndex(print_box.x + (side == BracketSide::Left ? 0 : 1),
                     y + i)] = s;
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
