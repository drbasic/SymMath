#include "Canvas.h"

#include <algorithm>
#include <cassert>

#include "Brackets.h"
#include "RenderBehaviour.h"

namespace {

enum BracketsParts {
  SmallLeft,
  SmallRight,
  // parts of brackets
  TopLeft,
  TopRight,
  StrightLeft,
  StrightRight,
  BottomLeft,
  BottomRight,
  MiddleLeft,
  MiddleRight,
  // additional part of sqrts
  AdditionalBottomLeft,
  Last,
};
const wchar_t kRoundBrackets[BracketsParts::Last + 1] = L"()╭╮││╰╯││ ";
const wchar_t kSquareBrackets[BracketsParts::Last + 1] = L"[]┌┐││└┘││ ";
const wchar_t kFigureBrackets[BracketsParts::Last + 1] = L"{}╭╮││╰╯╮╭ ";
const wchar_t kStrightBrackets[BracketsParts::Last + 1] = L"││││││││││ ";
const wchar_t kSqrtBrackets[BracketsParts::Last + 1] = L"??┌┐│ ╯ │ ╰";
const wchar_t kDivider = L'─';

// ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghklmnopqrstuvwyz";
const wchar_t kN[] = L".0123456789+-∙=()aeix";
//ᴬᴮCᴰᴱᴴᴵᴶᴷᴸᴹᴺᴼᴾᴿᵀᵁᵂᵇᶜᵈⁱᵏᵐⁿᵖᵗᵘᵛⁿʰʳʷʸˡˢ";
const wchar_t kU[] = L"˙⁰¹²³⁴⁵⁶⁷⁸⁹⁺⁻·⁼⁽⁾ᵃᵉⁱˣ";
//ₒₓₔᵣᵤᵥ";
const wchar_t kD[] = L"ͺ₀₁₂₃₄₅₆₇₈₉₊₋․₌₍₎ₐₑᵢₓ";

wchar_t GetSubScript(wchar_t sym) {
  assert(std::size(kN) == std::size(kU));
  for (size_t i = 0; i < std::size(kN); ++i) {
    if (kN[i] == sym)
      return kU[i];
  }
  return sym;
}

wchar_t GetSuperScript(wchar_t sym) {
  assert(std::size(kN) == std::size(kD));
  for (size_t i = 0; i < std::size(kN); ++i) {
    if (kN[i] == sym)
      return kD[i];
  }
  return sym;
}
}  // namespace

//=============================================================================
PrintBox::PrintBox() = default;

PrintBox::PrintBox(uint32_t x,
                   uint32_t y,
                   uint32_t width,
                   uint32_t height,
                   uint32_t base_line)
    : x(x), y(y), width(width), height(height), base_line(base_line) {}

PrintBox::PrintBox(const PrintSize& print_size, uint32_t x, uint32_t y)
    : x(x),
      y(y),
      width(print_size.width),
      height(print_size.height),
      base_line(print_size.base_line + y) {}

PrintBox::PrintBox(const PrintBox& rh) = default;

PrintBox::PrintBox(PrintBox&& rh) noexcept = default;

PrintBox& PrintBox::operator=(const PrintBox& rh) = default;

PrintBox& PrintBox::operator=(PrintBox&& rh) noexcept = default;

PrintBox PrintBox ::Infinite() {
  return PrintBox(0, 0, 1000, 10000, 0);
}

PrintBox PrintBox::ShrinkTop(uint32_t delta_height) const {
  PrintBox result(*this);
  assert(result.height >= delta_height);
  result.y += delta_height;
  result.height -= delta_height;
  return result;
}

PrintBox PrintBox::ShrinkLeft(uint32_t delta_width) const {
  PrintBox result(*this);
  assert(result.width >= delta_width);
  result.x += delta_width;
  result.width -= delta_width;
  return result;
}

PrintSize PrintBox::Size() const {
  return {width, height, base_line};
}

PrintSize::PrintSize(uint32_t width, uint32_t height, uint32_t base_line)
    : width(width), height(height), base_line(base_line) {}

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
  uint32_t new_size = (print_size.width + 1) * print_size.height;
  data_.resize(static_cast<size_t>(new_size));
  std::fill(std::begin(data_), std::end(data_), ' ');
  for (uint32_t row = 0; row < print_size.height; ++row) {
    data_[GetIndex(print_size.width - 1, row) + 1] = '\n';
  }
}

const std::wstring& Canvas::ToString() const {
  assert(print_size_ != PrintSize{});
  return data_;
}

PrintSize Canvas::PrintAt(const PrintBox& print_box,
                          std::string_view str,
                          SubSuperBehaviour sub_super_behaviour,
                          bool dry_run) {
  return PrintAt(print_box, std::wstring(str.begin(), str.end()),
                 sub_super_behaviour, dry_run);
}

PrintSize Canvas::PrintAt(const PrintBox& print_box,
                          std::wstring_view str,
                          SubSuperBehaviour sub_super_behaviour,
                          bool dry_run) {
  if (!dry_run) {
    assert(print_size_ != PrintSize{});
    assert(print_box.x + str.size() <= print_size_.width);
    size_t indx = GetIndex(print_box.x, print_box.base_line);
    auto data_start = std::begin(data_) + indx;
    for (size_t i = 0; i < str.size(); ++i)
      assert(data_start[i] == ' ');
    if (sub_super_behaviour == SubSuperBehaviour::Normal) {
      std::copy(std::begin(str), std::end(str), data_start);
    } else {
      for (size_t i = 0; i < str.size(); ++i) {
        data_start[i] = sub_super_behaviour == SubSuperBehaviour::Subscript
                            ? GetSubScript(str[i])
                            : GetSuperScript(str[i]);
      }
    }
  }
  return {static_cast<uint32_t>(str.size()), 1, 0};
}

PrintSize Canvas::RenderBrackets(PrintBox print_box,
                                 BracketType bracket_type,
                                 const PrintSize& inner_size,
                                 bool dry_run,
                                 PrintBox* inner_print_box) {
  assert(inner_print_box);
  // Render Left bracket
  PrintSize left_br_size = RenderBracket(print_box, BracketSide::Left,
                                         bracket_type, inner_size, dry_run);
  print_box = print_box.ShrinkLeft(left_br_size.width);

  // Calculate PrintSize for inner value with spaces
  PrintBox inner_size_with_spaces{inner_size, print_box.x,
                                  print_box.base_line - left_br_size.base_line};
  inner_size_with_spaces.width += 2;
  {
    // Render top bracket
    inner_size_with_spaces.base_line = inner_size_with_spaces.y;
    RenderBracket(inner_size_with_spaces, BracketSide::Top, bracket_type,
                  inner_size_with_spaces.Size(), dry_run);
  }

  // Calculate PrintBox for inner value
  *inner_print_box = inner_size_with_spaces;
  inner_print_box->x += 1;
  inner_print_box->width = inner_size.width;
  inner_print_box->height = inner_size.height;
  uint32_t delta_y = (bracket_type == BracketType::Sqrt)
                         ? 1
                         : (left_br_size.height - inner_size.height) / 2;
  inner_print_box->y += delta_y;
  inner_print_box->base_line = inner_print_box->y + inner_size.base_line;

  // Render right bracket
  print_box = print_box.ShrinkLeft(inner_size_with_spaces.width);
  PrintSize right_br_size = RenderBracket(print_box, BracketSide::Right,
                                          bracket_type, inner_size, dry_run);

  return left_br_size.GrowWidth(inner_size_with_spaces.Size(), false)
      .GrowWidth(right_br_size, false);
}

PrintSize Canvas::RenderBracket(const PrintBox& print_box,
                                BracketSide side,
                                BracketType bracket_type,
                                const PrintSize& inner_size,
                                bool dry_run) {
  if (side == BracketSide::Left || side == BracketSide::Right)
    return RenderBracketLR(print_box, side, bracket_type, inner_size.height,
                           dry_run);
  return RenderBracketT(print_box, side, bracket_type, inner_size.width,
                        dry_run);
}

PrintSize Canvas::RenderBracketLR(const PrintBox& print_box,
                                  BracketSide side,
                                  BracketType bracket_type,
                                  uint32_t height,
                                  bool dry_run) {
  assert(side == BracketSide::Left || side == BracketSide::Right);
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
    case BracketType::Stright:
      brackets = kStrightBrackets;
      break;
    case BracketType::Sqrt:
      brackets = kSqrtBrackets;
      break;
  }
  assert(brackets);

  if (height == 1 && bracket_type != BracketType::Sqrt) {
    if (!dry_run) {
      uint32_t y = print_box.base_line - height / 2;
      data_[GetIndex(print_box.x, y)] =
          (side == BracketSide::Left ? brackets[BracketsParts::SmallLeft]
                                     : brackets[BracketsParts::SmallRight]);
    }
    return {1, 1, 0};
  }
  uint32_t width = 1;
  uint32_t native_base_line = height / 2;
  if (bracket_type == BracketType::Sqrt) {
    height += 1;
    native_base_line += 1;
    width += side == BracketSide::Left ? 1 : 0;
  } else if (bracket_type != BracketType::Stright) {
    height += 2;
    native_base_line += 1;
  }

  if (!dry_run) {
    uint32_t y = print_box.base_line - native_base_line;
    for (uint32_t i = 0; i < height; ++i) {
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
        s = side == BracketSide::Left ? brackets[BracketsParts::StrightLeft]
                                      : brackets[BracketsParts::StrightRight];
      }
      data_[GetIndex(print_box.x + width - 1, y + i)] = s;
    }
    if (bracket_type == BracketType::Sqrt && side == BracketSide::Left) {
      data_[GetIndex(print_box.x + width - 2, y + height - 1)] =
          brackets[BracketsParts::AdditionalBottomLeft];
    }
  }
  return {width, height, native_base_line};
}

PrintSize Canvas::RenderBracketT(const PrintBox& print_box,
                                 BracketSide side,
                                 BracketType bracket_type,
                                 uint32_t width,
                                 bool dry_run) {
  assert(side == BracketSide::Top);
  if (bracket_type != BracketType::Sqrt)
    return {};
  return RenderDivider(print_box, width, dry_run);
}

PrintSize Canvas::RenderDivider(const PrintBox& print_box,
                                uint32_t width,
                                bool dry_run) {
  return PrintAt(print_box, std::wstring(static_cast<size_t>(width), kDivider),
                 SubSuperBehaviour::Normal, dry_run);
}

void Canvas::SetDryRun(bool dry_run) {
  dry_run_ = dry_run;
}

size_t Canvas::GetIndex(uint32_t x, uint32_t y) const {
  assert(print_size_ != PrintSize{});
  assert(x < print_size_.width);
  assert(y < print_size_.height);
  uint32_t indx = y * (print_size_.width + 1) + x;
  return static_cast<size_t>(indx);
}
