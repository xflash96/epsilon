#include "epsilon/linear/kronecker_product_impl.h"
#include "epsilon/vector/vector_util.h"
#include "epsilon/util/time.h"

namespace linear_map {

LinearMap::DenseMatrix KroneckerProductImpl::AsDense() const {
  VLOG(1) << "Converting kron to dense (" << m() << " x " << n() << ")";

  DenseMatrix A = A_->AsDense();
  DenseMatrix B = B_->AsDense();
  DenseMatrix C(m(), n());

  for (int i = 0; i < A.rows(); i++) {
    for (int j = 0; j < A.cols(); j++) {
      C.block(i*B.rows(), j*B.cols(), B.rows(), B.cols()) = A(i,j)*B;
    }
  }

  return C;
}

LinearMap::SparseMatrix KroneckerProductImpl::AsSparse() const {
  VLOG(1) << "Converting kron to sparse (" << m() << " x " << n() << ")";

  DenseMatrix A = A_->AsDense();
  DenseMatrix B = B_->AsDense();
  SparseMatrix C(m(), n());

  {
    std::vector<Eigen::Triplet<double> > coeffs;
    for (int i = 0; i < A.rows(); i++) {
      for (int j = 0; j < A.cols(); j++) {
        if (A(i,j) == 0)
          continue;
        AppendBlockTriplets(A(i,j)*B, i*B.rows(), j*B.cols(), &coeffs);
      }
    }
    C.setFromTriplets(coeffs.begin(), coeffs.end());
  }

  return C;
}

LinearMapImpl::DenseVector KroneckerProductImpl::Apply(
    const LinearMapImpl::DenseVector& x) const {
  DenseMatrix X = ToMatrix(x, B_->n(), A_->n());
  return ToVector(
      A_->ApplyMatrix(
          B_->ApplyMatrix(X).transpose()).transpose());
}

bool KroneckerProductImpl::operator==(const LinearMapImpl& other) const {
  if (other.type() != KRONECKER_PRODUCT ||
      other.m() != m() ||
      other.n() != n())
    return false;

  auto const& K = static_cast<const KroneckerProductImpl&>(other);
  return K.A() == A() && K.B() == B();
}


}  // namespace linear_map
