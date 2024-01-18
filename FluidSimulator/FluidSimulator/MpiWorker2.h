#pragma once
#include "physics.h"

struct MpiWorkerRange {
    unsigned int start;
    unsigned int end;
};

class MpiWorker {
public:
    void Run();

private:
    Physics physics;
};
