#include "Exception.h"

Exception::Exception(std::string err) : err_(std::move(err)) {}
