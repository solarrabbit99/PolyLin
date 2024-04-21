#pragma once
#include <unordered_set>

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
  PEEK_BACK
};

typedef int value_type;
typedef int time_type;
typedef unsigned int proc_type;
typedef unsigned int id_type;

#define MIN_TIME INT32_MIN
#define MAX_TIME INT32_MAX

struct Operation {
  id_type id;
  proc_type proc;
  Method method;
  value_type value;
  time_type startTime;
  time_type endTime;

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
};

template <>
struct std::hash<Operation> {
  std::size_t operator()(const Operation& o) const {
    return std::hash<id_type>()(o.id);
  }
};

typedef std::unordered_set<Operation> History;