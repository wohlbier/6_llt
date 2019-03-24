#ifndef ALGEBRA_HH
#define ALGEBRA_HH

#include <memoryweb.h>
#include "types.hh"

//Scalar_t dot(pRow_t a, pRow_t b)
bool dot(Scalar_t & ans, Row_t const & a, Row_t const & b) // no semiring
{
    bool result = false;
//    Row_t::iterator ait = a->begin();
//    Row_t::iterator bit = b->begin();
//
//    Scalar_t result = 0;
//
//    while (ait != a->end() && bit != b->end())
//    {
//        Index_t a_idx = std::get<0>(*ait);
//        Index_t b_idx = std::get<0>(*bit);
//
//        if (a_idx == b_idx)
//        {
//            result += std::get<1>(*ait) * std::get<1>(*bit);
//            ++ait;
//            ++bit;
//        }
//        else if (a_idx < b_idx)
//        {
//            ++ait;
//        }
//        else
//        {
//            ++bit;
//        }
//    }
//
    return result;
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
    Scalar_t ** tmp
        = (Scalar_t **)mw_malloc2d(NODELETS(),
                                   A.nrows_per_nodelet()*sizeof(Scalar_t));
    
    for (Index_t irow = 0; irow < A.nrows(); ++irow)
    {
        // address of Row_t to use in migration hints
        pRow_t prow = A.row_addr(irow);
        pRow_t crow = C.row_addr(irow);
        for (Index_t icol = 0; icol < B.nrows(); ++icol)
        {
            pRow_t pcol = B.row_addr(icol); // using icol of B^T is row addr
//           cilk_migrate_hint(prow);
//           Scalar_t val = cilk_spawn dot(prow, pcol); // should return mask
//           if (val != 0)
//           {
//               C_row.push_back(std::make_tuple(icol, val));
//           }
//           //cilk_sync;
        }
//       cilk_sync; // race?
//       C.setRow(irow, C_row);
//       C_row.clear();
    }
}

#endif // ALGEBRA_HH
