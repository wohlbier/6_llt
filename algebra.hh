#ifndef ALGEBRA_HH
#define ALGEBRA_HH

#include <memoryweb.h>
#include "types.hh"
#include "utils.hh"

static inline
bool index_exists(pRow_t r, Index_t icol)
{
    bool result = false;
    Row_t::iterator rit = r->begin();
    while (rit != r->end())
    {
        if (icol == std::get<0>(*rit))
        {
            result = true;
            break;
        }
        ++rit;
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
                   pMatrix_t M, pMatrix_t C, pMatrix_t A, pMatrix_t B)
{
    // return for empty row or column
    if (!A->getrow(irow) || !B->getrow(icol)) return;
    // mask
    if (!index_exists(M->getrow(irow), icol)) return;

    Scalar_t ans;
    if (dot(ans, A->getrow(irow), B->getrow(icol)))
    {
        C->getrow(irow)->push_back(std::make_tuple(icol, ans));
    }
}

static inline
void ABT_Mask_NoAccum_kernel(
    pMatrix_t C,               // output matrix
    pMatrix_t const M,         // mask matrix
    // SemiringT,               // semiring
    pMatrix_t const A,         // Input matrix 1
    pMatrix_t const B,         // Input matrix 2
    bool replace_flag = false)  // put the answer in place?
{
    // making use of the fact we know that B equals L^T

    for (Index_t icol = 0; icol < B->nrows(); ++icol)
    {
        //std::cout << "icol: " << icol << " of " << B.nrows() << std::endl;
        for (Index_t irow = 0; irow < A->nrows(); ++irow)
        {
            // want the thread to run on the nodelet of the row of C
            cilk_migrate_hint(C->nodelet_addr(irow));
            cilk_spawn mask_dot_push(irow, icol, M, C, A, B);
        }
        cilk_sync;
    }
}

Scalar_t reduce(pMatrix_t A)
{
    Scalar_t sum = 0;
    for (Index_t irow = 0; irow < A->nrows(); ++irow)
    {
        pRow_t pArow = A->getrow(irow);
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
