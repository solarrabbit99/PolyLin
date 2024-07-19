#pragma once
#include <algorithm>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include "definitions.hpp"
#include "util.hpp"

namespace polylin {

class HistoryReader {
 public:
  virtual History getHist() = 0;
  virtual std::string getHistTypeStr() = 0;
};

class OperationHistoryReader {
 public:
  OperationHistoryReader(const std::string& path) : path(path) {}

  History getHist() {
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

  std::string getHistTypeStr() {
    std::ifstream f(path);
    std::string line;
    if (!std::getline(f, line) || line[0] != '#') return "";

    return trim(line.substr(1));
  }

 private:
  const std::string path;
};

// Used for reading histories provided in the format used in violin
class EventHistoryReader {
 public:
  EventHistoryReader(const std::string& path) : path(path) {}

  History getHist() {
    std::ifstream f(path);
    std::string line;
    History hist;
    std::unordered_map<proc_type, Operation> lastProcOp;
    time_type time = 0;
    while (std::getline(f, line)) {
      if (histType.empty()) {
        if (line.find("@object") != std::string::npos)
          // For some reason, violin adds prefix "atomic-" to the histType
          histType = line.substr(line.find("atomic-") + 7);
        continue;
      }

      if (line.empty() || line[0] == '#') continue;

      std::stringstream ss{line};
      std::string procStr, eventStr;

      ss >> procStr >> eventStr;

      proc_type proc = std::stoi(procStr.substr(1, procStr.size() - 2));

      if (lastProcOp.count(proc)) {
        lastProcOp.at(proc).endTime = time;

        std::string valueStr;
        ss >> valueStr;
        if (valueStr.size()) {
          lastProcOp.at(proc).value =
              valueStr == "empty" ? -1 : std::stoi(valueStr);
        }

        hist.emplace(std::move(lastProcOp.at(proc)));
        lastProcOp.erase(proc);
      } else {
        std::string opStr;
        ss >> opStr;

        std::string methodStr = opStr.substr(0, opStr.find('('));

        // Violin uses "add" and "remove" instead of "enq" and "deq"
        if (methodStr == "add") methodStr = "enq";
        if (methodStr == "remove") methodStr = "deq";

        lastProcOp.emplace(
            proc, Operation{proc, getMethodFromStr(methodStr), 0, time, 0});

        if (opStr.find('(') != std::string::npos) {
          std::string valueStr = opStr.substr(opStr.find('(') + 1);
          valueStr.pop_back();  // Remove trailing ')'
          lastProcOp.at(proc).value = std::stoi(valueStr);
        }
      }

      ++time;
    }
    return hist;
  }

  std::string getHistTypeStr() { return histType; }

 private:
  const std::string path;
  std::string histType;
};
}  // namespace polylin