#ifndef DATA_MATRIX_ELEMENT_H
#define DATA_MATRIX_ELEMENT_H

#include <algorithm>

namespace BuildOrderOptimizer::Data::Matrix {

template <class T>
struct Element {
    unsigned index;
    T value;

    Element() : index(0), value(T()) {}
    Element(unsigned i) : index(i), value(T()) {}
    Element(unsigned i, T v) : index(i), value(v) {}
    Element(Element const &e) : index(e.index), value(e.value) {}

    Element &
    operator=(Element r) {
        std::swap(index, r.index);
        std::swap(value, r.value);
        return *this;
    }

    bool
    operator<(Element const &r) const {
        return index < r.index;
    }

    bool
    operator<(unsigned r) const {
        return index < r;
    }
};

} // namespace BuildOrderOptimizer::Data::Matrix
#endif
