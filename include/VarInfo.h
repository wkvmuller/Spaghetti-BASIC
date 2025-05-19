#pragma once
#include <string>

struct VarInfo {
  double numericValue = 0.0;
  std::string stringValue;
  bool isString = false;

  VarInfo() = default;
  VarInfo(double val, bool isStr = false) : numericValue(val), isString(isStr) {}
  VarInfo(const std::string& str) : stringValue(str), isString(true) {}
};
