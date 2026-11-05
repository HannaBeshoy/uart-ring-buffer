#include "demos.h"

minmax_t min_max_pairs(const int *a, size_t n)
{
    /* compare the two elements of each pair, then the smaller with min and the
       larger with max: 3 comparisons per 2 elements instead of 4 */
    minmax_t r = { a[0], a[0], 0 };
    size_t i = n % 2 ? 1 : 0;
    if (n % 2 == 0) {
        r.comparisons++;
        if (a[0] < a[1]) { r.min = a[0]; r.max = a[1]; } else { r.min = a[1]; r.max = a[0]; }
        i = 2;
    }
    for (; i + 1 < n; i += 2) {
        int lo = a[i], hi = a[i + 1];
        r.comparisons++;
        if (lo > hi) { int t = lo; lo = hi; hi = t; }
        r.comparisons++; if (lo < r.min) r.min = lo;
        r.comparisons++; if (hi > r.max) r.max = hi;
    }
    return r;
}

double horner(const double *coef, size_t degree, double x, long *mults)
{
    /* coef[0] is the highest power: p(x) = ((c0 x + c1) x + c2) ... */
    double y = coef[0];
    *mults = 0;
    for (size_t i = 1; i <= degree; i++) { y = y * x + coef[i]; (*mults)++; }
    return y;
}

unsigned long long fast_pow(unsigned long long base, unsigned e, long *mults)
{
    /* square-and-multiply: about 2*log2(e) multiplications instead of e-1 */
    unsigned long long r = 1;
    *mults = 0;
    while (e) {
        if (e & 1) { r *= base; (*mults)++; }
        e >>= 1;
        if (e) { base *= base; (*mults)++; }
    }
    return r;
}
