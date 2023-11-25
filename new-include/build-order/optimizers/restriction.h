#ifndef OPTIMIZERS_RESTRICTION_H
#define OPTIMIZERS_RESTRICTION_H

namespace BuildOrderOptimizer::Optimizers {

struct Restriction {
    unsigned less_than;
    unsigned greater_than;

    constexpr Restriction() : less_than(-1), greater_than(0) {}
    constexpr Restriction(unsigned lt, unsigned gt) : less_than(lt), greater_than(gt) {}

    constexpr operator bool() const { return less_than > 0 || greater_than < -1; }

    constexpr bool
    operator==(Restriction r) const {
        return less_than == r.less_than && greater_than == r.greater_than;
    }

    constexpr bool
    operator!=(Restriction r) const {
        return !(*this == r);
    }
};

} // namespace BuildOrderOptimizer::Optimizers
#endif
