#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

enum class Bracket {
  Left,
  Right,
};

enum class BracketType;

enum class MinusBehavior {
  Relax,
  Ommit,
  Force,
  OmmitMinusAndBrackets,
};

enum class BracketsBehavior {
  Relax,
  Ommit,
  Force,
};

struct PrintSize {
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
  PrintBox(PrintBox&& rh);

  PrintBox& operator=(const PrintBox& rh);
  PrintBox& operator=(PrintBox&& rh);

  static PrintBox Infinite();

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
                    bool dry_run);
  PrintSize PrintAt(const PrintBox& print_box,
                    std::wstring_view str,
                    bool dry_run);

  PrintSize RenderBracket(const PrintBox& print_box,
                          Bracket br,
                          BracketType bracket_type,
                          size_t height,
                          bool dry_run);

  PrintSize RenderDivider(const PrintBox& print_box,
                          size_t width,
                          bool dry_run);

  void SetDryRun(bool dry_run);

 private:
  size_t GetIndex(size_t x, size_t y) const;

  bool dry_run_ = false;
  PrintSize print_size_;
  std::wstring data_;
};
