// === Eigen‐based SVD and Eigen Decompositions ===  
// Append this to the end of ExtendedMATOPS.hpp

#ifndef EXTENDED_MATOPS_EIGEN_HPP
#define EXTENDED_MATOPS_EIGEN_HPP

#include <Eigen/Dense>

namespace MATOPS {

// Compute the Singular Value Decomposition of any (dense) matrix `mat`.
// U   : Left singular vectors (size Rows×Rows)
// S   : Singular values (size min(Rows,Cols)×1)
// V   : Right singular vectors (size Cols×Cols)
template<typename Scalar, int Rows, int Cols>
void computeSVD(
    const Eigen::Matrix<Scalar, Rows, Cols>& mat,
    Eigen::Matrix<Scalar, Rows, Rows>&       U,
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1>& S,
    Eigen::Matrix<Scalar, Cols, Cols>&       V)
{
    Eigen::JacobiSVD<Eigen::Matrix<Scalar, Rows, Cols>> svd(
        mat, Eigen::ComputeFullU | Eigen::ComputeFullV);
    U = svd.matrixU();
    S = svd.singularValues();
    V = svd.matrixV();
}

// Compute the Eigenvalue decomposition of any square matrix `mat`.
// eigenvalues   : Real parts of eigenvalues (size Dim×1)
// eigenvectors  : Corresponding real eigenvectors (size Dim×Dim)
template<typename Scalar, int Dim>
void computeEigen(
    const Eigen::Matrix<Scalar, Dim, Dim>& mat,
    Eigen::Matrix<Scalar, Dim, 1>&        eigenvalues,
    Eigen::Matrix<Scalar, Dim, Dim>&      eigenvectors)
{
    Eigen::EigenSolver<Eigen::Matrix<Scalar, Dim, Dim>> solver(mat);
    eigenvalues  = solver.eigenvalues().real();
    eigenvectors = solver.eigenvectors().real();
}

} // namespace MATOPS

#endif // EXTENDED_MATOPS_EIGEN_HPP

} // namespace ExtendedMATOPS
