#ifndef DATA_MATRIX_SPARSEMATRIX_H
#define DATA_MATRIX_SPARSEMATRIX_H

#include <omp.h>

#include "row.h"

namespace BuildOrderOptimizer::Data::Matrix {

template <class T>
class SparseMatrix {
  public:
    SparseMatrix() : _cols(0) {}
    SparseMatrix(unsigned cols) : _cols(cols) {}
    SparseMatrix(unsigned rows, unsigned cols) : _data(contiguous<Row<T>>(rows)), _cols(cols) {}
    SparseMatrix(SparseMatrix const &m) : _data(m._data), _cols(m._cols) {}

    Row<T> const &
    operator[](unsigned i) const {
        return _data[i];
    }

    Row<T> &
    operator[](unsigned i) {
        return _data[i];
    }

    unsigned
    rows() const {
        return _data.size();
    }

    unsigned
    columns() const {
        return _cols;
    }

    void
    addRow(Row<T> const &r) {
        _data.push_back(r);
    }

    void
    append(SparseMatrix const &m) {
        _data.insert(_data.end(), m._data.begin(), m._data.end());
    }

    SparseMatrix
    gaussian() const {
        SparseMatrix gauss(*this);

        unsigned pivot_row = 0;
        unsigned offset = 0;
        T pivot;

        for (auto current_row = 0; current_row < gauss.rows(); current_row++) {
            if (current_row + offset == _cols) {
                break;
            }

            pivot_row = gauss.select_pivot(current_row, offset);

            // swap rows
            if (pivot_row > current_row) {
                std::swap(gauss[current_row], gauss[pivot_row]);
            }

            // eliminate
            pivot = gauss[current_row].get(current_row + offset);
            if (pivot != 0) {
                gauss.normalize_row(current_row, offset, pivot);
                gauss.eliminate_all(current_row, offset);
            } else {
                offset++;
                current_row--;
            }
        }

        return gauss;
    }

  protected:
    unsigned
    select_pivot(unsigned index, unsigned offset) const {
        auto column = index + offset;
        auto row = index;
        T max = abs(_data[index].get(column));

        for (auto j = index + 1; j < _data.size(); j++) {
            T value = abs(_data[j].get(column));

            if (value > max) {
                max = value;
                row = j;
            }
        }

        return row;
    }

    void
    normalize_row(unsigned index, unsigned offset, T normalizer) {
        for (auto j = 0; j < _cols; j++) {
            auto current = ret[i].get(j);
            _data[index].set(j, current / normalizer);
        }
    }

    void
    eliminate_row(unsigned pivot_row, unsigned offset, unsigned current_row) {
        auto normalizer = _data[current_row].get(pivot_row + offset);

        if (normalizer == 0) {
            return;
        }

        for (auto i = 0; i < _cols; i++) {
            auto current_element = _data[current_row].get(i);
            auto pivot_element = _data[pivot_row].get(i);

            _data[current_row].set(i, current_element - normalizer * pivot_element);
        }
    }

    void
    eliminate_all(unsigned pivot_row, unsigned offset) {
#pragma omp parallel for default(shared) schedule(guided)
        for (auto current_row = 0; current_row < _data.size(); current_row++) {
            if (pivot_row != current_row) {
                eliminate_row(pivot_row, offset, current_row);
            }
        }
    }

  private:
    contiguous<Row<T>> _data;
    unsigned _cols;
};

} // namespace BuildOrderOptimizer::Data::Matrix
#endif
