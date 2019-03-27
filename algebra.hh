#ifndef ALGEBRA_HH
#define ALGEBRA_HH

#include <memoryweb.h>
#include "types.hh"

//void set_val(Scalar_t ** a, Index_t irow, Scalar_t val)
//{
//    a[n_map(irow)][r_map(irow)] = val;
//}
//
//void set_array(Scalar_t ** a, Index_t nvals, Scalar_t val)
//{
//    for (Index_t irow = 0; irow < nvals; ++irow)
//    {
//        cilk_migrate_hint(a + n_map(irow));
//        cilk_spawn set_val(a, irow, val);
//    }
//    cilk_sync;
//}

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

//void mask_dot_push(Scalar_t ** T_val, Index_t irow, Index_t icol,
void mask_dot_push(Index_t irow, Index_t icol,
                   pRow_t pMrow, pRow_t pCrow, pRow_t pArow, pRow_t pBcol)
{
    // return for empty row or column
    if (!pArow || !pBcol) return;

    // [n_map(icol)][r_map(icol)] maps into T_val like irow

    Scalar_t ans;

//    if (dot(T_val[n_map(irow)][r_map(irow)], pArow, pBcol)
//        && index_exists(pMrow,icol))
    if (dot(ans, pArow, pBcol) && index_exists(pMrow,icol))
    {
        //std::cerr << "ans: " << ans << std::endl;
        // the right value is in T_val.
        pCrow->push_back(std::make_tuple(icol, ans));
    }

//    Index_t sum = 0;
//    Index_t N = 4096;
//    for (long i = 0; i < N; ++i) {
//        sum += (long)pCrow;
//    }

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
//    Scalar_t ** T_val
//        = (Scalar_t **)mw_malloc2d(NODELETS(),
//                                   A.nrows_per_nodelet()*sizeof(Scalar_t));
//    set_array(T_val, A.nrows(), 0);

    for (Index_t icol = 0; icol < B.nrows(); ++icol)
    {
        pRow_t pBcol = B.row_addr(icol); // using icol of B^T is row addr
        for (Index_t irow = 0; irow < A.nrows(); ++irow)
        {
            // address of Row_t to use in migration hints
            pRow_t pCrow = C.row_addr(irow);
            pRow_t pMrow = M.row_addr(irow);
            pRow_t pArow = A.row_addr(irow);

            //std::cout << "irow,pCrow: " << irow << ", " << pCrow << std::endl;
            //std::cout << "irow,pMrow: " << irow << ", " << pMrow << std::endl;
            //std::cout << "irow,pArow: " << irow << ", " << pArow << std::endl;

            // want the thread to run on the nodelet of the row of C
            //cilk_migrate_hint(pCrow);
            //cilk_spawn mask_dot_push(T_val, irow, icol,
            //                         pMrow, pCrow, pArow, pBcol);
            //cilk_spawn mask_dot_push(irow, icol,
            mask_dot_push(irow, icol,
                                     pMrow, pCrow, pArow, pBcol);
        }
//        cilk_sync;
//        set_array(T_val, A.nrows(), 0);
    }
}

Scalar_t reduce(Matrix_t & A)
{
    Scalar_t sum = 0;
    for (Index_t irow = 0; irow < A.nrows(); ++irow)
    {
        pRow_t pArow = A.row_addr(irow);
        Row_t::iterator ait = pArow->begin();
//        std::cout << "(irow,size): ("
//                  << irow << ", " << pArow->size() << ")" << std::endl;
        while (ait != pArow->end())
        {
            sum += std::get<1>(*ait);
        }
    }
    return sum;
}


#endif // ALGEBRA_HH
