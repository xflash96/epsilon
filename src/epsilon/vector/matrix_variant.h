// Dynamic polymorphism for Eigen dense/sparse matrix types

#ifndef EPSILON_VECTOR_MATRIX_VARIANT_H
#define EPSILON_VECTOR_MATRIX_VARIANT_H

#include <glog/logging.h>

#include <Eigen/Dense>
#include <Eigen/SparseCore>

#include "epsilon/vector/vector_util.h"

class MatrixVariant {
 public:
  typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> DenseMatrix;
  typedef Eigen::SparseMatrix<double> SparseMatrix;
  typedef Eigen::DiagonalMatrix<double, Eigen::Dynamic> DiagonalMatrix;

  ~MatrixVariant() {
    VLOG(2) << "dtor";
    Destruct();
  }

  MatrixVariant(const DenseMatrix& dense) {
    VLOG(2) << "dense copy ctor " << this;
    type_ = DENSE;
    new (&dense_) DenseMatrix(dense);
  }

  MatrixVariant(DenseMatrix&& dense) {
    VLOG(2) << "dense move ctor " << this;
    type_ = DENSE;
    new (&dense_) DenseMatrix(dense);
  }

  MatrixVariant(const SparseMatrix& sparse) {
    VLOG(2) << "sparse copy ctor " << this;
    type_ = SPARSE;
    new (&sparse_) SparseMatrix(sparse);
  }

  MatrixVariant(const MatrixVariant& rhs) {
    VLOG(2) << "copy ctor";
    switch (rhs.type_) {
      case MatrixVariant::SPARSE:
        new (&sparse_) SparseMatrix(rhs.sparse_);
        break;
      case MatrixVariant::DENSE:
        new (&dense_) DenseMatrix(rhs.dense_);
        break;
      case MatrixVariant::DIAGONAL:
        new (&diagonal_) DiagonalMatrix(rhs.diagonal_);
        break;
      case MatrixVariant::SCALAR:
        new (&scalar_) ScalarMatrix(rhs.scalar_);
        break;
    }
    type_ = rhs.type_;
  }

  MatrixVariant(MatrixVariant&& rhs) {
    VLOG(2) << "move ctor " << this;
    switch (rhs.type_) {
      case MatrixVariant::SPARSE:
        new (&sparse_) SparseMatrix(std::move(rhs.sparse_));
        break;
      case MatrixVariant::DENSE:
        new (&dense_) DenseMatrix(std::move(rhs.dense_));
        break;
      case MatrixVariant::DIAGONAL:
        new (&diagonal_) DiagonalMatrix(std::move(rhs.diagonal_));
        break;
      case MatrixVariant::SCALAR:
        new (&scalar_) ScalarMatrix(std::move(rhs.scalar_));
        break;
    }
    type_ = rhs.type_;
  }

  MatrixVariant& operator=(const MatrixVariant& rhs) {
    VLOG(2) << "copy assignment " << this;
    MatrixVariant lhs(rhs);
    *this = std::move(lhs);
    return *this;
  }

  MatrixVariant& operator=(MatrixVariant&& rhs) {
    VLOG(2) << "move assignment " << this;
    Destruct();
    switch (rhs.type_) {
      case MatrixVariant::SPARSE:
        new (&sparse_) SparseMatrix(std::move(rhs.sparse_));
        break;
      case MatrixVariant::DENSE:
        new (&dense_) DenseMatrix(std::move(rhs.dense_));
        break;
      case MatrixVariant::DIAGONAL:
        new (&diagonal_) DiagonalMatrix(std::move(rhs.diagonal_));
        break;
      case MatrixVariant::SCALAR:
        new (&scalar_) ScalarMatrix(std::move(rhs.scalar_));
        break;
    }
    type_ = rhs.type_;
    return *this;
  }

  std::string DebugString() const;
  MatrixVariant transpose() const;
  MatrixVariant& operator+=(const MatrixVariant& rhs);
  MatrixVariant& operator*=(const MatrixVariant& rhs);

  int rows() const;
  int cols() const;
  DenseMatrix AsDense() const;

 private:
  void Destruct() {
    switch (type_) {
      case MatrixVariant::DENSE:
        dense_.~DenseMatrix();
        break;
      case MatrixVariant::SPARSE:
        sparse_.~SparseMatrix();
        break;
      case MatrixVariant::DIAGONAL:
        diagonal_.~DiagonalMatrix();
      case MatrixVariant::SCALAR:
        break;
    }
  }

  enum Type {
    DENSE,
    SPARSE,
    DIAGONAL,
    SCALAR,
  } type_;

  struct ScalarMatrix {
    int n;
    double alpha;
  };

  union {
    DenseMatrix dense_;
    SparseMatrix sparse_;
    DiagonalMatrix diagonal_;
    ScalarMatrix scalar_;
  };
};

MatrixVariant operator+(MatrixVariant lhs, const MatrixVariant& rhs);
MatrixVariant operator*(MatrixVariant lhs, const MatrixVariant& rhs);

#endif  // EPSILON_VECTOR_MATRIX_VARIANT_H
