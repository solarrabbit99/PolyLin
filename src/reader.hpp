#pragma once
#include <fstream>
#include <sstream>
#include <unordered_map>

#include "definitions.hpp"
#include "util.hpp"

namespace polylin {

class HistoryReader {
 public:
  History readFile(const std::string& path) {
    std::ifstream f(path);
    std::string line;
    History hist;
    while (std::getline(f, line)) {
      std::stringstream ss{line};
      line = trim(line);
      if (line.empty() || line[0] == '#' ||
          std::count(line.begin(), line.end(), ' ') != 4)
        continue;

      proc_type proc;
      std::string methodStr;
      value_type value;
      time_type startTime, endTime;

      ss >> proc >> methodStr >> value >> startTime >> endTime;

      hist.emplace(proc, getMethodFromStr(methodStr), value, startTime,
                   endTime);
    }
    return hist;
  }
};

}  // namespace polylin