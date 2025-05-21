#pragma once
#include <string>

struct VarInfo {
  double numericValue = 0.0;
  std::string stringValue;
  bool isString = false;

  VarInfo() = default;
  VarInfo(const std::string& str) : stringValue(str), isString(true) {}
  VarInfo(double val) : numericValue(val), isString(false) {}
};
