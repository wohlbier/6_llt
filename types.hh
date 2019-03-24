#ifndef TYPES_HH
#define TYPES_HH

typedef long Index_t;
typedef long Scalar_t;
typedef std::vector<Index_t> IndexArray_t;
typedef std::vector<std::tuple<Index_t,Scalar_t>> Row_t;
typedef Row_t * pRow_t;

static inline
Index_t r_map(Index_t i) { return i / NODELETS(); } // slow running index
static inline
Index_t n_map(Index_t i) { return i % NODELETS(); } // fast running index

class Matrix_t
{
public:
    Matrix_t(Index_t nrows)
        : nrows_(nrows)
    {
        nrows_per_nodelet_ = nrows_ + nrows_ % NODELETS();

        // allocate Row_t's
        row_block_ = (Row_t **)mw_malloc2d(NODELETS(),
                                           nrows_per_nodelet_ * sizeof(Row_t));
        
        // placement new Row_t's
        for (Index_t irow = 0; irow < nrows_; ++irow)
        {
            size_t nid(n_map(irow));
            size_t rid(r_map(irow));

            // row_block_[nid][rid] is a Row_t
            // row_block_[nid] + rid is address of Row_t's
            // &row_block_[nid][rid] = row_block_[nid] + rid

            // migrations to do placement new on other nodelets
            pRow_t rowPtr = new(row_block_[nid] + rid) Row_t();
            assert(rowPtr);
        }
    }

    Index_t nrows() { return nrows_; }
    Index_t nrows() const { return nrows_; }

    Index_t nrows_per_nodelet() { return nrows_per_nodelet_; }
    Index_t nrows_per_nodelet() const { return nrows_per_nodelet_; }

    void build(IndexArray_t::iterator i_it,
               IndexArray_t::iterator j_it,
               IndexArray_t::iterator v_it,
               Index_t nedges)
    {
        for (Index_t ix = 0; ix < nedges; ++ix)
        {
            cilk_migrate_hint(row_block_ + n_map(*i_it));
            cilk_spawn setElement(*i_it, *j_it, *v_it);
            cilk_sync;
            ++i_it; ++j_it; ++v_it; // increment iterators
        }
        //cilk_sync; // races?
    }

    void setRow(Index_t irow, Row_t const & src) {
        size_t nid(n_map(irow));
        size_t rid(r_map(irow));
        Row_t r = row_block_[nid][rid];
        r.clear();
        r = src;
    }

    pRow_t row_addr(Index_t irow)
    { return row_block_[n_map(irow)] + r_map(irow); }

    pRow_t row_addr(Index_t irow) const
    { return row_block_[n_map(irow)] + r_map(irow); }

private:

    void setElement(Index_t irow, Index_t icol, Index_t const &val)
    {
        size_t nid(n_map(irow));
        size_t rid(r_map(irow));
        Row_t r = row_block_[nid][rid];

        if (r.empty()) // empty row
        {
            r.push_back(std::make_tuple(icol, val));
        }
        else // insert into row
        {
            Row_t::iterator it = r.begin();
            while (std::get<0>(*it) < icol and it != r.end())
            {
                it++;
            }
            if (it == r.end())
            {
                r.push_back(std::make_tuple(icol, val));
            }
            else
            {
                it = r.insert(it, std::make_tuple(icol, val));
            }
        }
    }

    Index_t nrows_;
    Index_t nrows_per_nodelet_;
    Row_t ** row_block_;
};
#endif // TYPES_HH
