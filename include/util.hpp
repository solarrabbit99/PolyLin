#pragma once
#include <string>

#include "definitions.hpp"

namespace polylin {

std::string trim(const std::string& str) {
  size_t start = str.find_first_not_of(' ');
  if (start == std::string::npos) return "";
  size_t end = str.find_last_not_of(' ');
  if (end == std::string::npos) return "";
  return str.substr(start, end - start + 1);
}

}  // namespace polylin