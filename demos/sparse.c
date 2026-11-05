#include "demos.h"

/* triplets kept sorted by (row, col); a zero value is not stored */
bool sparse_set(sparse_t *m, int row, int col, double value)
{
    size_t i = 0;
    while (i < m->n && (m->t[i].row < row || (m->t[i].row == row && m->t[i].col < col))) i++;
    if (i < m->n && m->t[i].row == row && m->t[i].col == col) {
        if (value != 0) { m->t[i].value = value; return true; }
        for (size_t j = i; j + 1 < m->n; j++) m->t[j] = m->t[j + 1];   /* remove */
        m->n--;
        return true;
    }
    if (value == 0) return true;
    if (m->n == sizeof m->t / sizeof m->t[0]) return false;
    for (size_t j = m->n; j > i; j--) m->t[j] = m->t[j - 1];
    m->t[i] = (triplet_t){ row, col, value };
    m->n++;
    return true;
}

double sparse_get(const sparse_t *m, int row, int col)
{
    for (size_t i = 0; i < m->n; i++)
        if (m->t[i].row == row && m->t[i].col == col) return m->t[i].value;
    return 0;
}

void sparse_mul_vec(const sparse_t *m, const double *x, double *y)
{
    for (int r = 0; r < m->rows; r++) y[r] = 0;
    for (size_t i = 0; i < m->n; i++) y[m->t[i].row] += m->t[i].value * x[m->t[i].col];
}
