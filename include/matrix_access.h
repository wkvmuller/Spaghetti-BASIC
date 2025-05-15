#ifndef MATRIX_ACCESS_H
#define MATRIX_ACCESS_H

#include "program_structure.h"
#include <iostream>

VarInfo getMatrixElement(const MatrixValue &matrix,
                         const std::vector<int> &indices) {
  VarInfo result;

  if (!matrix.isSparse) {
    size_t offset = 0;
    size_t multiplier = 1;
    for (int i = matrix.dimensions.size() - 1; i >= 0; --i) {
      offset += indices[i] * multiplier;
      multiplier *= matrix.dimensions[i];
    }
    if (offset < matrix.denseValues.size()) {
      return matrix.denseValues[offset];
    } else {
      std::cerr << "Out-of-bounds access in dense matrix.\n";
      return result;
    }
  }

  MatrixIndex idx{indices};
  auto it = matrix.sparseValues.find(idx);
  if (it != matrix.sparseValues.end()) {
    return it->second;
  }

  return result;
}

void setMatrixElement(MatrixValue &matrix, const std::vector<int> &indices,
                      const VarInfo &value) {
  if (!matrix.isSparse) {
    size_t offset = 0;
    size_t multiplier = 1;
    for (int i = matrix.dimensions.size() - 1; i >= 0; --i) {
      offset += indices[i] * multiplier;
      multiplier *= matrix.dimensions[i];
    }
    if (offset >= matrix.denseValues.size()) {
      std::cerr << "Out-of-bounds write in dense matrix.\n";
      return;
    }
    matrix.denseValues[offset] = value;
    return;
  }

  MatrixIndex idx{indices};
  matrix.sparseValues[idx] = value;
}

#endif // MATRIX_ACCESS_H
