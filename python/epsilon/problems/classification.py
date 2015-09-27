
import cvxpy as cp
import numpy as np
import scipy.sparse as sp

def create_dense(m, n):
    np.random.seed(0)
    if n < 100:
        p = 1
    else:
        p = 100./n

    np.random.seed(0)
    A = np.random.randn(m, n)
    A = A*sp.diags([1 / np.sqrt(np.sum(A**2, 0))], [0])

    x0 = sp.rand(n, 1, p)
    x0.data = np.random.randn(x0.nnz)
    b = np.sign(A*x0 + np.sqrt(0.1)*np.random.randn(m,1))

    return A, b

# TODO(mwytock): Make it so expression tree doesnt scale with m
def logistic_loss(x):
    return sum(cp.log_sum_exp(cp.vstack(0, -x[i])) for i in xrange(x.size[0]))

def hinge_loss(x):
    return cp.sum_entries(cp.max_elemwise(0, 1-x))
