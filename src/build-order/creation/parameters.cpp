#include <build-order/creation/parameters.h>

BuildOrderOptimizer::Creation::Parameters BuildOrderOptimizer::Creation::CREATION = {
    .objective =
        {
            .minimum = 2.0,
            .maximum = 4.0,
        },

    .restriction =
        {
            .minimum = 1.5,
            .maximum = 3.5,
        },

    .deltaO =
        {
            .minimum = 1.01,
            .maximum = 1.1,
        },

    .deltaR =
        {
            .minimum = 1.8,
            .maximum = 2.5,
        },
};

BuildOrderOptimizer::Creation::Parameters BuildOrderOptimizer::Creation::FIX = {
    .objective =
        {
            .minimum = 1.5,
            .maximum = 2.5,
        },

    .restriction =
        {
            .minimum = 2.6,
            .maximum = 3.5,
        },

    .deltaO =
        {
            .minimum = 1.1,
            .maximum = 1.3,
        },

    .deltaR =
        {
            .minimum = 1.4,
            .maximum = 2.0,
        },
};
