#ifndef ARCHIVERS_ARCHIVER_H
#define ARCHIVERS_ARCHIVER_H

#include <build-order/optimizers/problem.h>
#include <build-order/solution.h>

namespace BuildOrderOptimizer::Archivers {

class Archiver {
  protected:
    Population _data;
    const Optimizers::Problem *_problem;
    unsigned _capacity;

  public:
    Archiver() : _problem(nullptr), _capacity(0) {}

    virtual void filter(Population &) const = 0;

    void setProblem(Optimizers::Problem *p);
    void setCapacity(unsigned c);

    Population operator()() const;
    Solution operator[](unsigned i) const;

    unsigned size() const;

    Population::const_iterator begin() const;
    Population::const_iterator end() const;

    virtual void insert(Solution a);

    void unsafeInsert(Solution a);
    void unsafeFilter();
};

typedef Archiver *(*ArchiverFactory)(unsigned, Optimizers::Problem *);

} // namespace BuildOrderOptimizer::Archivers

#endif
