#ifndef ALGEBRA_HH
#define ALGEBRA_HH

#include <memoryweb.h>
#include "types.hh"

static inline
bool index_exists(pRow_t a, Index_t i)
{
    bool result = false;
    Row_t::iterator ait = a->begin();
    while (ait != a->end())
    {
        if (i == std::get<0>(*ait))
        {
            result = true;
            break;
        }
        ++ait;
    }
    return result;
}

static inline
bool dot(Scalar_t & ans, pRow_t a, pRow_t b) // no semiring
{
    bool result = false;
    Row_t::iterator ait = a->begin();
    Row_t::iterator bit = b->begin();

    ans = 0;
    while (ait != a->end() && bit != b->end())
    {
        Index_t a_idx = std::get<0>(*ait);
        Index_t b_idx = std::get<0>(*bit);

        if (a_idx == b_idx)
        {
            ans += std::get<1>(*ait) * std::get<1>(*bit);
            result = true;
            ++ait;
            ++bit;
        }
        else if (a_idx < b_idx)
        {
            ++ait;
        }
        else
        {
            ++bit;
        }
    }
    return result;
}

static inline
void mask_dot_push(Index_t irow, Index_t icol,
                   pRow_t pMrow, pRow_t pCrow, pRow_t pArow, pRow_t pBcol)
{
    // return for empty row or column
    if (!pArow || !pBcol) return;

    // [n_map(icol)][r_map(icol)] maps into T_val like irow

    Scalar_t ans;
    if (dot(ans, pArow, pBcol) && index_exists(pMrow,icol))
    {
        pCrow->push_back(std::make_tuple(icol, ans));
    }
}

void ABT_Mask_NoAccum_kernel(
    Matrix_t & C,               // output matrix
    Matrix_t const & M,         // mask matrix
    // SemiringT,               // semiring
    Matrix_t const & A,         // Input matrix 1
    Matrix_t const & B,         // Input matrix 2
    bool replace_flag = false)  // put the answer in place?
{
    // making use of the fact we know that B equals L^T

    for (Index_t icol = 0; icol < B.nrows(); ++icol)
    {
        pRow_t pBcol = B.row_addr(icol); // using icol of B^T is row addr
        for (Index_t irow = 0; irow < A.nrows(); ++irow)
        {
            // address of Row_t to use in migration hints
            pRow_t pCrow = C.row_addr(irow);
            pRow_t pMrow = M.row_addr(irow);
            pRow_t pArow = A.row_addr(irow);

            // want the thread to run on the nodelet of the row of C
            cilk_migrate_hint(pCrow);
            cilk_spawn mask_dot_push(irow, icol, pMrow, pCrow, pArow, pBcol);
        }
        cilk_sync;
    }
}

Scalar_t reduce(Matrix_t & A)
{
    Scalar_t sum = 0;
    for (Index_t irow = 0; irow < A.nrows(); ++irow)
    {
        pRow_t pArow = A.row_addr(irow);
        Row_t::iterator ait = pArow->begin();
        while (ait != pArow->end())
        {
            sum += std::get<1>(*ait);
            ++ait;
        }
    }
    return sum;
}

#endif // ALGEBRA_HH
