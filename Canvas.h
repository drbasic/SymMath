#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

enum class Bracket {
  Left,
  Right,
};

enum MinusBehavior {
  Relax,
  Ommit,
  Force,
  OmmitMinusAndBrackets,
};

struct PrintSize {
  size_t width = 0;
  size_t height = 0;
  size_t base_line = 0;
  bool operator==(const PrintSize& rh) const;
  bool operator!=(const PrintSize& rh) const;

  void GrowRight(const PrintSize& other);
};

struct PrintPosition {
  size_t x = 0;
  size_t y = 0;
};

class Canvas {
 public:
  void Resize(const PrintSize& print_size);
  std::wstring ToString() const;

  PrintSize PrintAt(const PrintPosition& print_pos,
                    std::string_view str,
                    bool dry_run);

  PrintSize RenderBracket(const PrintPosition& print_pos,
                          Bracket br,
                          size_t height,
                          bool dry_run);

  void SetDryRun(bool dry_run);

 private:
  size_t GetIndex(const PrintPosition& print_pos) const;

  bool dry_run_ = false;
  PrintSize print_size_;
  std::vector<wchar_t> data_;
};
