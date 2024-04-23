#include <string>

#include "definitions.hpp"

std::string trim(const std::string& str) {
  size_t start = str.find_first_not_of(' ');
  if (start == std::string::npos) return "";
  size_t end = str.find_last_not_of(' ');
  if (end == std::string::npos) return "";
  return str.substr(start, end - start + 1);
}

Method getMethodFromStr(std::string str) {
  if (str == "push") return Method::PUSH;
  if (str == "pop") return Method::POP;
  if (str == "peek") return Method::PEEK;
  if (str == "push_front") return Method::PUSH_FRONT;
  if (str == "pop_front") return Method::POP_FRONT;
  if (str == "peek_front") return Method::PEEK_FRONT;
  if (str == "push_back") return Method::PUSH_BACK;
  if (str == "pop_back") return Method::POP_BACK;
  if (str == "peek_back") return Method::PEEK_BACK;
  if (str == "enq") return Method::ENQ;
  if (str == "deq")
    return Method::DEQ;
  else
    throw std::invalid_argument("Unknown method: " + str);
}