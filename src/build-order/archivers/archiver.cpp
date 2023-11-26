#include <build-order/archivers/archiver.h>

void
BuildOrderOptimizer::Archivers::Archiver::setProblem(Optimizers::Problem *p) {
    _problem = p;
}

void
BuildOrderOptimizer::Archivers::Archiver::setCapacity(unsigned c) {
    _capacity = c;
}

BuildOrderOptimizer::Population
BuildOrderOptimizer::Archivers::Archiver::operator()() const {
    return _data;
}

BuildOrderOptimizer::Solution
BuildOrderOptimizer::Archivers::Archiver::operator[](unsigned i) const {
    return _data[i];
}

unsigned
BuildOrderOptimizer::Archivers::Archiver::size() const {
    return _data.size();
}

BuildOrderOptimizer::Population::const_iterator
BuildOrderOptimizer::Archivers::Archiver::begin() const {
    return _data.begin();
}

BuildOrderOptimizer::Population::const_iterator
BuildOrderOptimizer::Archivers::Archiver::end() const {
    return _data.end();
}

void
BuildOrderOptimizer::Archivers::Archiver::insert(Solution a) {
    for (unsigned i = 0; i < size(); i++)
        if (_data[i] == a)
            return;

    _data.push_back(a);

    filter(_data);

    if (_data.size() > _capacity)
        _data.erase(_data.begin() + _capacity, _data.end());
}

void
BuildOrderOptimizer::Archivers::Archiver::unsafeInsert(Solution a) {
    _data.push_back(a);
}

void
BuildOrderOptimizer::Archivers::Archiver::unsafeFilter() {
    filter(_data);
}
