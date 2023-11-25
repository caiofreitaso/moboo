#ifndef ARCHIVERS_KNEE_H
#define ARCHIVERS_KNEE_H

#include "nsga2.h"

namespace BuildOrderOptimizer::Archivers {

class Knee_Archiver : public NSGA2_Archiver {
    static Data::Contiguous<Data::Contiguous<double>> _values;
    static unsigned _v_size;
    static unsigned _v_obj;

  public:
    Knee_Archiver(unsigned c, const Optimizers::Optimizer *o);

    static void knees(Data::Contiguous<Data::Contiguous<unsigned>> &, Data::Contiguous<Data::Contiguous<unsigned>> &,
                      Data::Contiguous<bool> const &);
};

} // namespace BuildOrderOptimizer::Archivers

#endif
