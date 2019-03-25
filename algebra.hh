#ifndef ALGEBRA_HH
#define ALGEBRA_HH

#include <memoryweb.h>
#include "types.hh"

void set_val(Scalar_t ** a, Index_t irow, Scalar_t val)
{
    a[n_map(irow)][r_map(irow)] = val;
}

void set_array(Scalar_t ** a, Index_t nvals, Scalar_t val)
{
    for (Index_t irow = 0; irow < nvals; ++irow)
    {
        cilk_migrate_hint(a + n_map(irow));
        cilk_spawn set_val(a, irow, val);
    }
    cilk_sync;
}

bool index_exists(pRow_t a, Index_t i)
{
    bool result = false;
    Row_t::iterator ait = a->begin();
    while (ait != a->end())
    {
        if (i == std::get<0>(*ait))
        {
            result = true;
        }
    }
    return result;
}

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
            ++ait;
            ++bit;
            result = true;
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

void mask_dot_push(Scalar_t ** T_val, Index_t icol,
                   pRow_t pMrow, pRow_t pCrow, pRow_t pArow, pRow_t pBcol)
{
    // do the masking
    // if M[icol] nonexistent, return
    if (!index_exists(pMrow,icol)) return;

    // [n_map(icol)][r_map(icol)] maps into T_val like irow

    if (dot(T_val[n_map(icol)][r_map(icol)], pArow, pBcol))
    {
        // the right value is in T_val, just assert it.
        assert(T_val[n_map(icol)][r_map(icol)]);
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

    // create temporary storage for one row worth of results
    Scalar_t ** T_val
        = (Scalar_t **)mw_malloc2d(NODELETS(),
                                   A.nrows_per_nodelet()*sizeof(Scalar_t));
    set_array(T_val, A.nrows(), 0);
    
    for (Index_t irow = 0; irow < A.nrows(); ++irow)
    {
        // address of Row_t to use in migration hints
        pRow_t pCrow = C.row_addr(irow);
        pRow_t pMrow = M.row_addr(irow);
        pRow_t pArow = A.row_addr(irow);
        for (Index_t icol = 0; icol < B.nrows(); ++icol)
        {
            pRow_t pBcol = B.row_addr(icol); // using icol of B^T is row addr

            cilk_migrate_hint(pCrow);
            cilk_spawn mask_dot_push(T_val, icol, pMrow, pCrow, pArow, pBcol);
        }
        cilk_sync;
        set_array(T_val, A.nrows(), 0);
    }
}

#endif // ALGEBRA_HH
