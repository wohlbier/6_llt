/*
  MM = MEMORY MAP
  Memory Reference Map. Entry (i,j) is the number of references
  made by threads on nodelet i to nodelet j. i == j for local references.
  Sum of column j is number of references on nodelet j, excluding remotes.
  Sum of row i, excluding i == j, is number of migrations away from nodelet i.

  Each row in the memory map represents the source nodelet on which a memory
  instruction was encountered and the column represents the destination nodelet
  on which the memory instruction was ultimately executed. Values on the
  diagonal represent local memory accesses and non-diagonal values show the
  number of memory accesses that required a migration to another nodelet to
  complete.

  RM = REMOTES MAP
  Remotes Reference Map. Counts number of remote operations from
  threads on nodelet i to nodelet j.

  Each row in the remotes map is a source nodelet and each column is a
  destination nodelet for a remote memory operation. Note that the diagonal
  values are always zero because a remote update to a local value will be
  counted as a local memory operation. In this example, all the values are zero
  showing that no remote memory operations occurred.
 */

#include <tuple>
#include <vector>

#include <memoryweb.h>

typedef long Index_t;
typedef long Scalar_t;
typedef std::vector<std::tuple<Index_t,Scalar_t>> Row_t;
typedef Row_t * pRow_t;

Index_t r_map(Index_t i) { return i / NODELETS(); } // slow running index
Index_t n_map(Index_t i) { return i % NODELETS(); } // fast running index

int main(int argc, char* argv[])
{
    starttiming();

    Index_t nrows = 16;
    Index_t nrows_per_nodelet = nrows + nrows % NODELETS();

    // NB: r is a "view 2" pointer to a Row_t
    //     r + 2 is an address on nodelet 2.
    //     r[2] will migrate to node 2 and read the value at address r + 2

    // MM: 124 memory references on nodelet 0.
    // RM: 0+1 on all j nodelets (0,j) j=1...7
    Row_t ** r
        = (Row_t **)mw_malloc2d(NODELETS(),
                                nrows_per_nodelet * sizeof(Row_t));

    for (Index_t row_idx = 0; row_idx < nrows; ++row_idx)
    {
        size_t nid(n_map(row_idx));
        size_t rid(r_map(row_idx));

        // r_repl[nid][rid] is a Row_t
        // r_repl[nid] + rid is address of Row_t's
        // &r_repl[nid][rid] = r_repl[nid] + rid

        // migrations to do placement new on other nodelets
        pRow_t rowPtr = new(&r[nid][rid]) Row_t();
    }

    // Push stuff onto nodelet 0 vector
    pRow_t rowPtr = &r[n_map(0)][r_map(0)];
    rowPtr->push_back(std::make_tuple(0,1));
    rowPtr->push_back(std::make_tuple(7,1));
    rowPtr->push_back(std::make_tuple(12,1));
    rowPtr->push_back(std::make_tuple(14,1));

    // this is an address on nodlet 7
    rowPtr = &r[n_map(15)][r_map(15)];

    // including this line completely changes the migration pattern
    // E. Hein guesses that this is stack spillage.
    rowPtr->push_back(std::make_tuple(1,1));

    return 0;
}
