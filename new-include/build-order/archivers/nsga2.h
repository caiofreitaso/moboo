#ifndef ARCHIVER_NSGA2_H
#define ARCHIVER_NSGA2_H

#include "archiver.h"

namespace BuildOrderOptimizer::Archivers {

class NSGA2_Archiver : public Archiver {
  protected:
    static bool dominates(Data::Contiguous<unsigned> a, Data::Contiguous<unsigned> b, Data::Contiguous<bool> min);

    static void quicksort(Data::Contiguous<unsigned> &indexes, unsigned begin, unsigned end, unsigned objective,
                          Data::Contiguous<Data::Contiguous<unsigned>> &v);

    static void quicksort(Data::Contiguous<unsigned> &v, unsigned begin, unsigned end, Data::Contiguous<double> &d);

  public:
    void (*dist_func)(Data::Contiguous<Data::Contiguous<unsigned>> &, Data::Contiguous<Data::Contiguous<unsigned>> &,
                      Data::Contiguous<bool> const &);

    NSGA2_Archiver(unsigned c, const Optimizers::Problem *p);

    virtual void filter(Population &pop) const;

    static void crowding(Data::Contiguous<Data::Contiguous<unsigned>> &, Data::Contiguous<Data::Contiguous<unsigned>> &,
                         Data::Contiguous<bool> const &);
};

} // namespace BuildOrderOptimizer::Archivers

#endif
