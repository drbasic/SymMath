#include "HotToken.h"

#include <cassert>
#include <memory>

#include "Operation.h"

ScopedParamsCounter::ScopedParamsCounter(HotToken* token,
                                         const Operation* operation)
    : token_(token),
      operation_(operation),
      params_count_(operation->OperandsCount()) {}

ScopedParamsCounter::~ScopedParamsCounter() {
  if (params_count_ != operation_->OperandsCount())
    token_->SetChanged();
}

HotToken::HotToken(HotToken* parent) : parent_(parent) {
  parent->Disarm();
  generation_and_armed_ = -(parent->Generation() + 1);
}

HotToken::~HotToken() {
  if (IsArmed())
    assert(false);
  if (parent_) {
    parent_->children_count_ += children_count_ + 1;
    parent_->changes_count_ += changes_count_;
  }
}

void HotToken::SetChanged() {
  ++changes_count_;
}

ScopedParamsCounter HotToken::CountParamsChanged(const Operation* operation) {
  return ScopedParamsCounter(this, operation);
}

void HotToken::Disarm() {
  if (generation_and_armed_ < 0)
    generation_and_armed_ = -generation_and_armed_;
}

bool HotToken::IsArmed() const {
  return generation_and_armed_ < 0;
}

int32_t HotToken::Generation() const {
  return (generation_and_armed_ < 0) ? -generation_and_armed_
                                     : generation_and_armed_;
}