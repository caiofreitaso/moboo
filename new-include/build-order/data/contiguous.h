#ifndef DATA_CONTIGUOUS_H
#define DATA_CONTIGUOUS_H

#ifndef USECONTIGUOUS_CUSTOM

#include <vector>

namespace BuildOrderOptimizer::Data {

template <class K>
using Contiguous = std::vector<K>;

#else
#include <algorithm>

namespace BuildOrder::Data {
template <class K>
class Contiguous {
    K *data;
    unsigned _sz;
    unsigned alloc;

  public:
    typedef K *iterator;
    typedef const K *const_iterator;

    Contiguous() : data(0), _sz(0), alloc(0) {}
    Contiguous(unsigned s) : data(new K[s]), _sz(s), alloc(s) {}

    Contiguous(unsigned s, K const &v) : data(new K[s]), _sz(s), alloc(s) {
        for (unsigned i = 0; i < _sz; i++) {
            data[i] = v;
        }
    }

    Contiguous(Contiguous const &v) : data(new K[v._sz]), _sz(v._sz), alloc(v._sz) {
        for (unsigned i = 0; i < _sz; i++) {
            data[i] = v.data[i];
        }
    }
    ~Contiguous() {
        for (unsigned i = 0; i < alloc; i++) {
            (&data[i])->~K();
        }
        delete[] data;
    }

    iterator
    begin() {
        return data;
    }
    iterator
    end() {
        return data + _sz;
    }

    const_iterator
    begin() const {
        return data;
    }
    const_iterator
    end() const {
        return data + _sz;
    }

    Contiguous &
    operator=(Contiguous const &c) {
        if (c._sz > alloc) {
            delete[] data;
            data = new K[c._sz];
            alloc = c._sz;
        }
        _sz = c._sz;
        for (unsigned i = 0; i < _sz; i++)
            data[i] = c.data[i];
    }

    void
    reserve(unsigned new_alloc) {
        if (new_alloc > alloc) {
            alloc = new_alloc;

            K *new_data = new K[new_alloc];
            for (unsigned i = 0; i < _sz; i++)
                new_data[i] = data[i];

            delete[] data;
            data = new_data;
        }
    }

    void
    resize(unsigned new_size, K const &v = K()) {
        if (alloc < new_size) {
            alloc = new_size;
            K *new_data = new K[new_size];
            unsigned cmp = _sz > new_size ? new_size : _sz;
            for (unsigned i = 0; i < cmp; i++)
                new_data[i] = data[i];
            for (unsigned i = cmp; i < new_size; i++)
                new_data[i] = v;

            delete[] data;
            data = new_data;
        }
        _sz = new_size;
    }

    void
    push_back(K v) {
        _sz++;
        if (_sz > alloc) {
            alloc++;
            K *new_data = new K[_sz];
            for (unsigned i = 0; i < _sz - 1; i++)
                new_data[i] = data[i];
            delete[] data;
            data = new_data;
        }
        data[_sz - 1] = v;
    }

    void
    insert(iterator it, K const &v) {
        unsigned pos = it - data;
        _sz++;

        if (_sz > alloc) {
            alloc = _sz;
            K *new_data = new K[_sz];

            for (unsigned i = 0; i < pos; i++)
                new_data[i] = data[i];

            new_data[pos] = v;

            for (unsigned i = pos + 1; i < _sz; i++)
                new_data[i] = data[i - 1];

            delete[] data;
            data = new_data;
        } else {
            for (unsigned i = _sz - 1; i > pos; i++)
                data[i] = data[i - 1];
            data[pos] = v;
        }
    }

    void
    insert(iterator it, iterator b, iterator e) {
        unsigned pos = it - data;
        unsigned sz = e - b;

        if (_sz + sz > alloc) {
            K *new_data = new K[_sz + sz];

            for (unsigned i = 0; i < pos; i++)
                new_data[i] = data[i];

            for (auto i = b; i != e; i++, pos++)
                new_data[pos] = *i;

            for (unsigned i = pos; i < _sz - 1; i++)
                new_data[i + sz] = data[i];

            delete[] data;
            data = new_data;
            alloc = _sz + sz;
        } else {
            for (unsigned i = _sz - 1; i > pos + sz; i++)
                data[i] = data[i - 1];
            for (auto i = b; i != e; i++, pos++)
                data[pos] = *i;
        }

        _sz += sz;
    }

    void
    clear() {
        delete[] data;
        _sz = 0;
        alloc = 0;
    }
    void
    pop_back() {
        _sz--;
    }

    void
    erase(iterator it) {
        erase(it, it + 1);
    }

    void
    erase(iterator b, iterator e) {
        unsigned sz = e - b;

        std::rotate(b, e, end());
        _sz -= sz;
    }

    K &
    front() {
        return data[0];
    }

    K &
    back() {
        return data[_sz - 1];
    }

    bool
    empty() const {
        return _sz == 0;
    }

    unsigned
    size() const {
        return _sz;
    }

    unsigned
    capacity() const {
        return alloc;
    }

    K &
    operator[](unsigned i) {
        return data[i];
    }

    K const &
    operator[](unsigned i) const {
        return data[i];
    }

    bool
    operator==(Contiguous const &c) const {
        if (c._sz != _sz)
            return false;
        for (unsigned i = 0; i < _sz; i++)
            if (!(data[i] == c[i]))
                return false;
        return true;
    }

    bool
    operator!=(Contiguous const &c) const {
        return !(*this == c);
    }
};
#endif

template <class T>
void
pop(Contiguous<T> &v, unsigned i = 0) {
    v.erase(v.begin() + i);
}

} // namespace BuildOrder::Data
#endif
