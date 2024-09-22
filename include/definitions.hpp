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
  CONTAINS,
  REMOVE,
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
  if (str == "poll") return Method::POLL;
  if (str == "contains") return Method::CONTAINS;
  if (str == "remove")
    return Method::REMOVE;
  else
    throw std::invalid_argument("Unknown method: " + str);
}

// typedef int value_type;
typedef unsigned long long time_type;
typedef unsigned int proc_type;
typedef unsigned int id_type;

#define DEFAULT_VALUE_TYPE int
#define EMPTY_VALUE -1
#define MIN_TIME std::numeric_limits<time_type>::lowest()

template <typename value_type>
struct Operation {
  id_type id;
  Method method;
  value_type value;
  time_type startTime;
  time_type endTime;
  bool retVal;

  Operation() = delete;

  Operation(Method method, value_type value, time_type startTime,
            time_type endTime, bool retVal)
      : method(method),
        value(value),
        startTime(startTime),
        endTime(endTime),
        retVal(retVal) {
    static id_type globId = 0;
    id = ++globId;
  };

  Operation(Method method, value_type value, time_type startTime,
            time_type endTime)
      : Operation(method, value, startTime, endTime, true) {};

  Operation(const Operation& o)
      : id(o.id),
        method(o.method),
        value(o.value),
        startTime(o.startTime),
        endTime(o.endTime),
        retVal(o.retVal) {};

  Operation(const Operation&& o)
      : id(o.id),
        method(o.method),
        value(o.value),
        startTime(o.startTime),
        endTime(o.endTime),
        retVal(o.retVal) {};

  Operation& operator=(const Operation& o) {
    id = o.id;
    method = o.method;
    value = o.value;
    startTime = o.startTime;
    endTime = o.endTime;
    retVal = o.retVal;
    return *this;
  }

  Operation& operator=(const Operation&& o) {
    id = o.id;
    method = o.method;
    value = o.value;
    startTime = o.startTime;
    endTime = o.endTime;
    retVal = o.retVal;
    return *this;
  }
};

template <typename value_type>
inline bool operator<(const Operation<value_type>& a,
                      const Operation<value_type>& b) {
  return a.id < b.id;
}

template <typename value_type>
inline bool operator==(const Operation<value_type>& a,
                       const Operation<value_type>& b) {
  return a.id == b.id;
}

template <typename value_type>
inline bool operator!=(const Operation<value_type>& a,
                       const Operation<value_type>& b) {
  return !(a == b);
}

template <typename value_type>
using History = std::unordered_set<Operation<value_type>>;

}  // namespace polylin

template <typename value_type>
struct std::hash<polylin::Operation<value_type>> {
  std::size_t operator()(const polylin::Operation<value_type>& o) const {
    return std::hash<polylin::id_type>()(o.id);
  }
};