#include <build-order/decision/decision-maker.h>

unsigned
BuildOrderOptimizer::Decision::DecisionMaker::choose(Population p) const {
    return choose(normalize(p));
}

unsigned
BuildOrderOptimizer::Decision::DecisionMaker::choose(Data::Contiguous<Data::Contiguous<double>> vec) const {
    unsigned k = 0;
    double dist = value(vec[0]);

    for (unsigned i = 1; i < vec.size(); i++) {
        double d_i = value(vec[i]);

        if (d_i < dist) {
            dist = d_i;
            k = i;
        }
    }

    return k;
}

BuildOrderOptimizer::Data::Contiguous<BuildOrderOptimizer::Data::Contiguous<double>>
BuildOrderOptimizer::Decision::DecisionMaker::normalize(Population p) const {
    Data::Contiguous<Data::Contiguous<double>> vec = target->toDVector(p);
    Data::Contiguous<bool> obj = target->objectivesVector();

    for (unsigned j = 0; j < obj.size(); j++)
        if (obj[j])
            for (unsigned i = 0; i < vec.size(); i++)
                vec[i][j] = -vec[i][j];

    Data::Contiguous<double> min(vec[0]);
    Data::Contiguous<double> max(vec[0]);

    for (unsigned i = 1; i < vec.size(); i++)
        for (unsigned j = 0; j < obj.size(); j++)
            if (vec[i][j] < min[j])
                min[j] = vec[i][j];
    for (unsigned i = 1; i < vec.size(); i++)
        for (unsigned j = 0; j < obj.size(); j++)
            if (vec[i][j] > max[j])
                max[j] = vec[i][j];

    for (unsigned i = 0; i < vec.size(); i++)
        for (unsigned j = 0; j < obj.size(); j++)
            vec[i][j] = (vec[i][j] - min[j]) / (max[j] - min[j]);

    return vec;
}

BuildOrderOptimizer::Solution
BuildOrderOptimizer::Decision::DecisionMaker::choose(State::GameState i0, unsigned iterations) const {
    Population front = target->optimize(i0, iterations);
    return front[choose(front)];
}
