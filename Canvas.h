#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

enum class BracketType;
enum class SubSuperBehaviour;

struct PrintSize {
  PrintSize() {}
  PrintSize(size_t width, size_t heigh, size_t base_line);
  bool operator==(const PrintSize& rh) const;
  bool operator!=(const PrintSize& rh) const;

  PrintSize GrowWidth(const PrintSize& other, bool allign_base_line) const;
  PrintSize GrowDown(const PrintSize& other, bool move_base_line) const;

  size_t width = 0;
  size_t height = 0;
  size_t base_line = 0;
};

struct PrintBox {
  PrintBox();
  PrintBox(size_t x, size_t y, size_t width, size_t height, size_t base_line);
  PrintBox(size_t x, size_t y, const PrintSize& print_size);
  PrintBox(const PrintBox& rh);
  PrintBox(PrintBox&& rh) noexcept;

  PrintBox& operator=(const PrintBox& rh);
  PrintBox& operator=(PrintBox&& rh) noexcept;

  static PrintBox Infinite();

  PrintBox ShrinkTop(size_t delta_height) const;
  PrintBox ShrinkLeft(size_t delta_width) const;

  size_t x = 0;
  size_t y = 0;
  size_t width = 0;
  size_t height = 0;
  size_t base_line = 0;
};

class Canvas {
 public:
  void Resize(const PrintSize& print_size);
  const std::wstring& ToString() const;

  PrintSize PrintAt(const PrintBox& print_box,
                    std::string_view str,
                    SubSuperBehaviour sub_super_behaviour,
                    bool dry_run);
  PrintSize PrintAt(const PrintBox& print_box,
                    std::wstring_view str,
                    SubSuperBehaviour sub_super_behaviour,
                    bool dry_run);

  PrintSize RenderBrackets(PrintBox print_box,
                           BracketType bracket_type,
                           PrintSize inner_size,
                           bool dry_run,
                           PrintBox* inner_print_box);

  PrintSize RenderDivider(const PrintBox& print_box,
                          size_t width,
                          bool dry_run);

  void SetDryRun(bool dry_run);

 private:
  enum class BracketSide {
    Left,
    Right,
    Top,
  };
  size_t GetIndex(size_t x, size_t y) const;
  PrintSize RenderBracket(const PrintBox& print_box,
                          BracketSide side,
                          BracketType bracket_type,
                          const PrintSize& inner_size,
                          bool dry_run);

  PrintSize RenderBracketLR(const PrintBox& print_box,
                            BracketSide side,
                            BracketType bracket_type,
                            size_t height,
                            bool dry_run);
  PrintSize RenderBracketT(const PrintBox& print_box,
                           BracketSide side,
                           BracketType bracket_type,
                           size_t width,
                           bool dry_run);

  bool dry_run_ = false;
  PrintSize print_size_;
  std::wstring data_;
};
