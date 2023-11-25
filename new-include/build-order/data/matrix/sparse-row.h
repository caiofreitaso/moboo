#ifndef DATA_MATRIX_SPARSEROW_H
#define DATA_MATRIX_SPARSEROW_H

#include <algorithm>
#include <iterator>

#include "row.h"

namespace BuildOrderOptimizer::Data::Matrix {

template <class T>
struct SparseRow : public Row<T> {
    virtual T
    get(unsigned i) const {
        auto f = this->find(i);

        if (f->index == i) {
            return f->value;
        }

        return SparseRow::default_value;
    }

    virtual void
    set(unsigned index, T v) {
        auto f = this->find(index);

        if (v != SparseRow::default_value) {
            if (f == this->row.end()) {
                this->row.insert(f, Element<T>(index, v));
            } else if (f->index == index) {
                f->value = v;
            } else {
                this->row.insert(f, Element<T>(index, v));
            }
        } else if (f != this->row.end() && f->index == index) {
            this->row.erase(f);
        }
    }
};

} // namespace BuildOrderOptimizer::Data::Matrix
#endif
