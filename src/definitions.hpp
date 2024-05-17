#pragma once
#include <limits>
#include <stdexcept>
#include <string>
#include <unordered_set>

namespace polylin {

enum Method {
  PUSH,
  POP,
  PEEK,
  ENQ,
  DEQ,
  PUSH_FRONT,
  POP_FRONT,
  PEEK_FRONT,
  PUSH_BACK,
  POP_BACK,
  PEEK_BACK,
  INSERT,
  POLL,
};

inline Method getMethodFromStr(std::string str) {
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
  if (str == "deq") return Method::DEQ;
  if (str == "insert") return Method::INSERT;
  if (str == "poll")
    return Method::POLL;
  else
    throw std::invalid_argument("Unknown method: " + str);
}

typedef int value_type;
typedef int time_type;
typedef unsigned int proc_type;
typedef unsigned int id_type;

#define EMPTY_VALUE -1
#define MIN_TIME std::numeric_limits<time_type>::lowest()
#define MAX_TIME std::numeric_limits<time_type>::max()

struct Operation {
  id_type id;
  proc_type proc;
  Method method;
  value_type value;
  time_type startTime;
  time_type endTime;

  Operation() = delete;

  Operation(proc_type proc, Method method, value_type value,
            time_type startTime, time_type endTime)
      : method(method), value(value), startTime(startTime), endTime(endTime) {
    static id_type globId = 0;
    id = ++globId;
  };

  Operation(const Operation& o)
      : id(o.id),
        method(o.method),
        value(o.value),
        startTime(o.startTime),
        endTime(o.endTime){};

  Operation(const Operation&& o)
      : id(o.id),
        method(o.method),
        value(o.value),
        startTime(o.startTime),
        endTime(o.endTime){};

  Operation& operator=(const Operation& o) {
    id = o.id;
    method = o.method;
    value = o.value;
    startTime = o.startTime;
    endTime = o.endTime;
    return *this;
  }
};

inline bool operator<(const Operation& a, const Operation& b) {
  return a.id < b.id;
}

inline bool operator==(const Operation& a, const Operation& b) {
  return a.id == b.id;
}

typedef std::unordered_set<Operation> History;

}  // namespace polylin

template <>
struct std::hash<polylin::Operation> {
  std::size_t operator()(const polylin::Operation& o) const {
    return std::hash<polylin::id_type>()(o.id);
  }
};