#pragma once
#include <vector>
#include <map>
#include <utility>
#include "VarInfo.h"

typedef std::pair<int, int> MatrixIndex;
constexpr size_t DENSE_MATRIX_THRESHOLD = 10000;

struct MatrixValue {
  std::map<MatrixIndex, VarInfo> sparseValues;
  std::vector<VarInfo> denseValues;
  std::vector<int> dimensions;
  size_t totalSize = 0;
  bool isSparse = false;

  void configureStorage(const std::vector<int>& dims) {
    dimensions = dims;
    totalSize = 1;
    for (int d : dims) totalSize *= d;

    if (totalSize < DENSE_MATRIX_THRESHOLD) {
      isSparse = false;
      denseValues.resize(totalSize);
    } else {
      isSparse = true;
      sparseValues.clear();
    }
  }

  size_t flattenIndex(const MatrixIndex& index) const {
    if (dimensions.size() != 2)
      throw std::runtime_error("Only 2D matrices supported in flattenIndex()");
    return index.first * dimensions[1] + index.second;
  }

  VarInfo get(const MatrixIndex& idx) const {
    if (isSparse) {
      auto it = sparseValues.find(idx);
      return it != sparseValues.end() ? it->second : VarInfo{0.0, false};
    } else {
      size_t flat = flattenIndex(idx);
      if (flat >= denseValues.size()) throw std::out_of_range("Index out of bounds");
      return denseValues[flat];
    }
  }

  void set(const MatrixIndex& idx, const VarInfo& value) {
    if (isSparse) {
      if (value.numericValue != 0.0 || value.isString)
        sparseValues[idx] = value;
      else
        sparseValues.erase(idx);
    } else {
      size_t flat = flattenIndex(idx);
      if (flat >= denseValues.size()) throw std::out_of_range("Index out of bounds");
      denseValues[flat] = value;
    }
  }
};
