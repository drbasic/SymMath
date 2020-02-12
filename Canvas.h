#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

struct PrintSize {
  size_t width = 0;
  size_t height = 0;
  size_t base_line = 0;
  bool operator==(const PrintSize& rh) const;
  bool operator!=(const PrintSize& rh) const;

  void Grow(size_t new_base_line, size_t new_height);
};

struct Position {
  size_t x = 0;
  size_t y = 0;
};

enum MinusBehavior {
  Relax,
  Ommit,
  Force,
  OmmitMinusAndBrackets,
};

class Canvas {
 public:
  void Resize(const PrintSize& print_size);
  std::wstring ToString() const;

  void PrintAt(const Position& pos, std::string_view str);
  enum class Bracket { Left, Right };
  void RenderBracket(const Position& pos, Bracket br, size_t height);

  void SetDryRun(bool dry_run);

 private:
  size_t GetIndex(const Position& pos) const;

  bool dry_run_ = false;
  PrintSize print_size_;
  std::vector<wchar_t> data_;
};