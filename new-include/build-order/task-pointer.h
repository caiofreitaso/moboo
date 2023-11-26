#ifndef TASK_POINTER_H
#define TASK_POINTER_H

#include <build-order/data/contiguous.h>

#include <algorithm>

namespace BuildOrderOptimizer {

struct TaskPointer {
    unsigned task;
    unsigned delay;

    TaskPointer() : task(0), delay(0) {}
    TaskPointer(TaskPointer const &pointer) : task(pointer.task), delay(pointer.delay) {}
    TaskPointer(unsigned task) : task(task), delay(0) {}
    TaskPointer(unsigned task, unsigned d) : task(task), delay(d) {}

    ~TaskPointer() {}

    TaskPointer &
    operator=(TaskPointer t) {
        std::swap(task, t.task);
        std::swap(delay, t.delay);

        return *this;
    }
};

typedef Data::Contiguous<TaskPointer> BuildOrder;

} // namespace BuildOrderOptimizer
#endif
