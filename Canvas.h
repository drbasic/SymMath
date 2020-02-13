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

  void GrowRight(const PrintSize& other);
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

  PrintSize PrintAt(const Position& pos, std::string_view str, bool dry_run);
  enum class Bracket { Left, Right };
  PrintSize RenderBracket(const Position& pos,
                          Bracket br,
                          size_t height,
                          bool dry_run);

  void SetDryRun(bool dry_run);

 private:
  size_t GetIndex(const Position& pos) const;

  bool dry_run_ = false;
  PrintSize print_size_;
  std::vector<wchar_t> data_;
};