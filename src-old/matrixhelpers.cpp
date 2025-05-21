#include <iostream>
#include "matrixvalue.h"

void printMatrix(const std::string& name, const MatrixValue& mat) {
  if (mat.dimensions.size() != 2)
    throw std::runtime_error("PRINT only supports 2D matrices");

  int rows = mat.dimensions[0];
  int cols = mat.dimensions[1];

  std::cout << name << " =\n";
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      VarInfo v = mat.get({i, j});
      std::cout << v.numericValue << " ";
    }
    std::cout << "\n";
  }
}
