#pragma once
#include <string>

class Exception {
 public:
  Exception(std::string err);
  std::string err_;
};
