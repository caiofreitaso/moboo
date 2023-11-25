#ifndef RULES_RELATIONTYPE_H
#define RULES_RELATIONTYPE_H

namespace BuildOrderOptimizer::Rules {

enum RelationType
{
    RT_COST,
    RT_CONSUME,
    RT_MAXIMUM,
    RT_BORROW,
    RT_PREREQUISITE
};

} // namespace BuildOrderOptimizer::Rules

#endif
