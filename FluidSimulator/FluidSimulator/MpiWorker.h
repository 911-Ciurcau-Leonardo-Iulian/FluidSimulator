#pragma once

class MpiWorker {
public:
    int me;

    MpiWorker(int me) : me(me) {}
    void run();
};
