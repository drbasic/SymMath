#pragma once

#include <memory>
#include <string>
#include <vector>

struct PrintSize {
  size_t width = 0;
  size_t height = 0;
  bool operator==(const PrintSize& rh) const;
  bool operator!=(const PrintSize& rh) const;
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
  std::string ToString() const;

  void PrintAt(const Position& pos, const std::string& str);
  enum class Bracket {
    Left, Right
  };
  void RenderBracket(const Position& pos, Bracket br, size_t height);

  void SetDryRun(bool dry_run);
private:
  size_t GetIndex(const Position& pos) const;

  bool dry_run_ = false;
  PrintSize print_size_;
  std::vector<char> data_;
};