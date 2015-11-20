
from collections import defaultdict
import struct

from cvxpy.utilities import power_tools

from epsilon import dcp
from epsilon import error
from epsilon import expression
from epsilon.expression_pb2 import Expression, Curvature

class TransformError(error.ExpressionError):
    pass

def fp_expr(expr):
    return struct.pack("q", hash(expr.SerializeToString())).encode("hex")

def validate_args(expr, count):
    if len(expr.arg) != count:
        raise TransformError(
            "invalid args %d != %d" % (len(expr.args), count),
            expr)

def only_arg(expr):
    validate_args(expr, 1)
    return expr.arg[0]

def dim(expr, index=None):
    if len(expr.size.dim) != 2:
        raise ExpressioneError("wrong number of dimensions", expr)
    if index is None:
        return expr.size.dim[0]*expr.size.dim[1]
    else:
        return expr.size.dim[index]

def epi(f_expr, t_expr):
    """An expression for an epigraph constraint.

    The constraint depends on the curvature of f:
      - f convex,  I(f(x) <= t)
      - f concave, I(f(x) >= t)
      - f affine,  I(f(x) == t)
    """
    f_curvature = dcp.get_curvature(f_expr)

    if f_curvature.curvature_type == Curvature.CONVEX:
        return expression.leq_constraint(f_expr, t_expr)
    elif f_curvature.curvature_type == Curvature.CONCAVE:
        return expression.leq_constraint(negate(f_expr), negate(t_expr))
    elif f_curvature.curvature_type == Curvature.AFFINE:
        return expression.eq_constraint(f_expr, t_expr);

    raise TransformError("Unknown curvature", f_expr)

def epi_var(expr, name, size=None):
    if size is None:
        size = expr.size.dim
    name += ":" + fp_expr(expr)
    return expression.variable(size[0], size[1], name)

def epi_transform(f_expr, name):
    t_expr = epi_var(f_expr, name)
    epi_f_expr = epi(f_expr, t_expr)
    return t_expr, epi_f_expr

def create_lu_var(expr):
    return lin_utils.create_var((dim(expr, 0), dim(expr, 1)))

# gm()/gm_constrs() translated from cvxpy.utilities.power_tools.gm_constrs()
def gm(t, x, y):
    return expression.soc_elemwise_constraint(
        expression.add(x, y),
        expression.add(x, expression.negate(y)),
        expression.multiply(expression.scalar_constant(2), t))

def gm_constrs(t_expr, x_exprs, p):
    assert power_tools.is_weight(p)
    w = power_tools.dyad_completion(p)
    tree = power_tools.decompose(w)

    gm_vars = 0
    def create_gm_var():
        var = epi_var(t_expr, "gm_var_%d" % gm_vars)
        gm_vars += 1
        return var

    d = defaultdict(create_gm_var)
    d[w] = t_expr
    if len(x_exprs) < len(w):
        x_exprs += [t_expr]
    assert len(x_exprs) == len(w)

    for i, (p, v) in enumerate(zip(w, x_exprs)):
        if p > 0:
            tmp = [0]*len(w)
            tmp[i] = 1
            d[tuple(tmp)] = v

    constraints = []
    for elem, children in tree.items():
        if 1 not in elem:
            constraints += [gm(d[elem], d[children[0]], d[children[1]])]

    return constraints
