#ifndef ARCHIVERS_NONDOMINATED_H
#define ARCHIVERS_NONDOMINATED_H

#include "archiver.h"

namespace BuildOrderOptimizer::Archivers {

class NonDominated_Archiver : public Archiver {
  public:
    NonDominated_Archiver(unsigned c, const Optimizers::Problem *p) {
        _capacity = c;
        _data.reserve(c + 1);
        _problem = p;
    }

    virtual void
    filter(Population &pop) const {
        pop = _problem->nonDominated(pop);
    }
};

} // namespace BuildOrderOptimizer::Archivers

#endif
