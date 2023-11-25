#ifndef ARCHIVERSH
#define ARCHIVERSH

#include <build-order/optimizers/optimizer.h>
#include <build-order/solution.h>

namespace BuildOrderOptimizer::Archivers {

class Archiver {
  protected:
    Population _data;
    const Optimizers::Optimizer *_optimizer;
    unsigned _capacity;

  public:
    Archiver() : _optimizer(nullptr), _capacity(0) {}

    virtual void filter(Population &) const = 0;

    void
    setOptimizer(Optimizers::Optimizer *o) {
        _optimizer = o;
    }

    void
    setCapacity(unsigned c) {
        _capacity = c;
    }

    Population
    operator()() const {
        return _data;
    }

    Solution
    operator[](unsigned i) const {
        return _data[i];
    }

    unsigned
    size() const {
        return _data.size();
    }

    Population::const_iterator
    begin() const {
        return _data.begin();
    }

    Population::const_iterator
    end() const {
        return _data.end();
    }

    virtual void
    insert(Solution a) {
        for (unsigned i = 0; i < size(); i++)
            if (_data[i] == a)
                return;

        _data.push_back(a);

        filter(_data);

        if (_data.size() > _capacity)
            _data.erase(_data.begin() + _capacity, _data.end());
    }

    void
    unsafeInsert(Solution a) {
        _data.push_back(a);
    }

    void
    unsafeFilter() {
        filter(_data);
    }
};

} // namespace BuildOrderOptimizer::Archivers

#endif
