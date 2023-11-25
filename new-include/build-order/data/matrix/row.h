#ifndef DATA_MATRIX_ROW_H
#define DATA_MATRIX_ROW_H

#include <build-order/data/contiguous.h>

#include <algorithm>
#include <iterator>

#include "element.h"

namespace BuildOrderOptimizer::Data::Matrix {

template <class T>
struct Row {
    const T default_value = T();

    typedef typename Contiguous<Element<T>>::iterator iterator;
    typedef typename Contiguous<Element<T>>::const_iterator const_iterator;

    Contiguous<Element<T>> row;

    Row &
    operator=(Row e) {
        std::swap(row, e.row);
        return *this;
    }

    iterator
    begin() {
        return row.begin();
    }

    iterator
    end() {
        return row.end();
    }

    const_iterator
    begin() const {
        return row.begin();
    }

    const_iterator
    end() const {
        return row.end();
    }

    iterator
    find(unsigned index) {
        return std::lower_bound(row.begin(), row.end(), index);
    }

    const_iterator
    find(unsigned index) const {
        return std::lower_bound(row.begin(), row.end(), index);
    }

    unsigned
    size() const {
        return row.size();
    }

    Element<T> &
    operator[](unsigned i) {
        return row[i];
    }

    Element<T> const &
    operator[](unsigned i) const {
        return row[i];
    }

    virtual T
    get(unsigned i) const {
        auto f = this->find(i);
        if (f == this->row.end())
            return default_value;
        if (f->index == i)
            return f->value;
        return default_value;
    }

    virtual void
    set(unsigned index, T v) {
        auto f = this->find(index);
        row.insert(f, Element<T>(index, v));
    }
};

} // namespace BuildOrderOptimizer::Data::Matrix
#endif
