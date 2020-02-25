#pragma once
#include <stdint.h>

class HotToken {
 public:
  HotToken() {}
  HotToken(const HotToken&) = delete;
  HotToken(HotToken* parent);
  ~HotToken();
  void SetChanged();
  uint32_t GetChangesCount() { return changes_count_; }

 private:
  friend class Constant;
  friend class Imaginary;
  friend class Operation;
  friend class Variable;
  friend class ErrorNode;
  friend class HotTokenHelper;

  void Disarm();
  bool IsArmed() const;
  int32_t Generation() const;

  HotToken* parent_ = nullptr;
  int32_t generation_and_armed_ = -1;
  uint32_t children_count_ = 0;
  uint32_t changes_count_ = 0;
};