#pragma once
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "definitions.hpp"
#include "util.hpp"

namespace polylin {

template <typename value_type>
class HistoryReader {
 public:
  HistoryReader(const std::string& path) : path(path) {}

  History<value_type> getHist() {
    std::ifstream f(path);
    std::string line;
    History<value_type> hist;
    while (std::getline(f, line)) {
      std::stringstream ss{line};
      line = trim(line);
      if (line.empty() || line[0] == '#') continue;

      std::string methodStr;
      value_type value;
      time_type startTime, endTime;

      ss >> methodStr >> value >> startTime >> endTime;

      hist.emplace(getMethodFromStr(methodStr), value, startTime, endTime);
    }
    return hist;
  }

  History<value_type> getExtHist() {
    std::ifstream f(path);
    std::string line;
    History<value_type> hist;
    while (std::getline(f, line)) {
      std::stringstream ss{line};
      line = trim(line);
      if (line.empty() || line[0] == '#') continue;

      std::string methodStr;
      value_type value;
      bool retVal;
      time_type startTime, endTime;

      ss >> methodStr >> value >> retVal >> startTime >> endTime;

      hist.emplace(getMethodFromStr(methodStr), value, startTime, endTime,
                   retVal);
    }
    return hist;
  }

  std::string getHistTypeStr() {
    std::ifstream f(path);
    std::string line;
    if (!std::getline(f, line) || line[0] != '#') return "";

    return trim(line.substr(1));
  }

 private:
  const std::string path;
};

}  // namespace polylin