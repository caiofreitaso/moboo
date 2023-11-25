#ifndef CREATION_PARAMETERS_H
#define CREATION_PARAMETERS_H

#include <build-order/random/rng.h>

namespace BuildOrderOptimizer::Creation {

struct Parameters {
    template <class T>
    struct Range {
        T minimum;
        T maximum;
    };

    Range<double> objective;
    Range<double> restriction;
    Range<double> deltaO;
    Range<double> deltaR;

    inline double
    getObjective() {
        return Random::drng(objective.minimum, objective.maximum);
    }

    inline double
    getRestriction() {
        return Random::drng(restriction.minimum, restriction.maximum);
    }

    inline double
    getDeltaO() {
        return Random::drng(deltaO.minimum, deltaO.maximum);
    }

    inline double
    getDeltaR() {
        return Random::drng(deltaR.minimum, deltaR.maximum);
    }
};

extern Parameters CREATION;
extern Parameters FIX;

} // namespace BuildOrderOptimizer::Creation

#endif
