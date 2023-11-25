#ifndef ARCHIVERS_ADAPTIVE_GRID_H
#define ARCHIVERS_ADAPTIVE_GRID_H

#include <build-order/optimizers/objective.h>

#include <cassert>
#include <cstdlib>
#include <iostream>

#include "archiver.h"

namespace BuildOrderOptimizer::Archivers {

class AdaptativeGrid_Archiver : public Archiver {
    unsigned _gridLevels;
    Data::Contiguous<unsigned> _ub;
    Data::Contiguous<unsigned> _lb;
    Data::Contiguous<unsigned> _uev;
    Data::Contiguous<unsigned> _region;
    Data::Contiguous<bool> obj;
    Data::Contiguous<unsigned> _min;
    Data::Contiguous<unsigned> _max;

    Data::Contiguous<unsigned> cmin(Data::Contiguous<Data::Contiguous<unsigned>>, unsigned) const;
    Data::Contiguous<unsigned> cmax(Data::Contiguous<Data::Contiguous<unsigned>>, unsigned) const;

    void fixBounds(Data::Contiguous<Data::Contiguous<unsigned>>, unsigned);
    unsigned getRegionIndex(Data::Contiguous<unsigned>) const;

  public:
    AdaptativeGrid_Archiver(unsigned c, const Optimizers::Optimizer *o, unsigned g = 5)
        : _gridLevels(g), _uev(c, 0), _region(c, 0) {
        _capacity = c;
        _data.reserve(c + 1);
        _optimizer = o;

        obj.reserve(o->numberObjectives());
        obj.push_back(true);

        for (auto &type : _optimizer->objectives) {
            for (auto &objective : type.row) {
                obj.push_back(objective.value == Optimizers::MINIMIZE);
            }
        }
    }

    virtual void
    filter(Population &pop) const {}

    virtual void insert(Solution a);
};

} // namespace BuildOrderOptimizer::Archivers

#endif
