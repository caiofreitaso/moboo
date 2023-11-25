#ifndef DECISION_ORIGIN_H
#define DECISION_ORIGIN_H

#include "decision-maker.h"

namespace BuildOrderOptimizer::Decision {

struct OriginDecisionMaker : public DecisionMaker {
    virtual double
    value(Data::Contiguous<double> v) const {
        double d = 0;
        for (unsigned i = 0; i < v.size(); i++) {
            d += v[i] * v[i];
        }
        return sqrt(d);
    }
};

} // namespace BuildOrderOptimizer::Decision

#endif
