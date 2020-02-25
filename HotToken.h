#pragma once
#include <stdint.h>

class Operation;
class HotToken;

class ScopedParamsCounter {
 public:
  ~ScopedParamsCounter();

 private:
  friend class HotToken;
  ScopedParamsCounter(HotToken* token, const Operation* operation);

  HotToken* token_;
  const Operation* operation_;
  const size_t params_count_;
};

class HotToken {
 public:
  HotToken() {}
  HotToken(const HotToken&) = delete;
  HotToken(HotToken* parent);
  ~HotToken();
  void SetChanged();
  uint32_t GetChangesCount() { return changes_count_; }
  ScopedParamsCounter CountParamsChanged(const Operation* operation);

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