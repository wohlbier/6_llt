#ifndef TYPES_HH
#define TYPES_HH

typedef long Index_t;
typedef long Scalar_t;
typedef std::vector<Index_t> IndexArray_t;
typedef std::vector<std::tuple<Index_t,Scalar_t>> Row_t;
typedef Row_t * pRow_t;
typedef pRow_t * ppRow_t;

static inline Index_t n_map(Index_t i) { return i % NODELETS(); }
static inline Index_t r_map(Index_t i) { return i / NODELETS(); }

/*
 * Overrides default new to always allocate replicated storage for instances
 * of this class. repl_new is intended to be used as a parent class for
 * distributed data structure types.
 */
class repl_new
{
public:
    // Overrides default new to always allocate replicated storage for
    // instances of this class
    static void *
    operator new(std::size_t sz)
    {
        return mw_mallocrepl(sz);
    }

    // Overrides default delete to safely free replicated storage
    static void
    operator delete(void * ptr)
    {
        mw_free(ptr);
    }
};

class Matrix_t : public repl_new
{
public:
    static Matrix_t * create(Index_t nrows)
    {
        return new Matrix_t(nrows);
    }

    Matrix_t() = delete;
    Matrix_t(const Matrix_t &) = delete;
    Matrix_t & operator=(const Matrix_t &) = delete;
    Matrix_t(Matrix_t &&) = delete;
    Matrix_t & operator=(Matrix_t &&) = delete;

    void build(IndexArray_t::iterator i_it,
               IndexArray_t::iterator j_it,
               IndexArray_t::iterator v_it,
               Index_t nedges)
    {
        for (Index_t ix = 0; ix < nedges; ++ix)
        {
            cilk_migrate_hint(row_addr(*i_it));
            cilk_spawn setElement(*i_it, *j_it, *v_it);
            cilk_sync;
            ++i_it; ++j_it; ++v_it; // increment iterators
        }
    }

    Index_t nrows() { return nrows_; }
    Index_t nrows() const { return nrows_; }

    pRow_t getrow(Index_t i) { return rows_[n_map(i)] + r_map(i); }
    pRow_t getrow(Index_t i) const { return rows_[n_map(i)] + r_map(i); }

    Index_t * row_addr(Index_t i)
    {
        // dereferencing causes migrations
        return (Index_t *)(rows_ + n_map(i));
    }

private:
    Matrix_t(Index_t nrows) : nrows_(nrows)
    {
        nrows_per_nodelet_ = r_map(nrows_) + n_map(nrows_);
        rows_ = (ppRow_t)mw_malloc2d(NODELETS(),
                                     nrows_per_nodelet_ * sizeof(Row_t));

        // replicate the class across nodelets
        for (Index_t i = 1; i < NODELETS(); ++i)
        {
            memcpy(mw_get_nth(this, i), mw_get_nth(this, 0), sizeof(*this));
        }

        // local mallocs on each nodelet
        for (Index_t i = 0; i < NODELETS(); ++i)
        {
            cilk_migrate_hint(rows_ + i);
            cilk_spawn allocateRows(i);
        }
        cilk_sync;
    }

    // localalloc a single row
    void allocateRows(Index_t i)
    {
        for (Index_t row_idx= 0; row_idx < nrows_per_nodelet_; ++row_idx)
        {
            new(rows_[i] + row_idx) Row_t();
        }
    }

    void setElement(Index_t irow, Index_t icol, Index_t const &val)
    {
    	pRow_t r = rows_[n_map(irow)] + r_map(irow);

        if (r->empty()) // empty row
        {
            r->push_back(std::make_tuple(icol, val));
        }
        else // insert into row
        {
            Row_t::iterator it = r->begin();
            while (std::get<0>(*it) < icol and it != r->end())
            {
                ++it;
            }
            if (it == r->end())
            {
                r->push_back(std::make_tuple(icol, val));
            }
            else
            {
                it = r->insert(it, std::make_tuple(icol, val));
            }
        }
    }

    Index_t nrows_;
    Index_t nrows_per_nodelet_;
    ppRow_t rows_;
};
typedef Matrix_t * pMatrix_t;

#endif // TYPES_HH
