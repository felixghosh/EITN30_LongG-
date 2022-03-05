#ifndef TRANSBUF
#define TRANSBUF

#include "ip.hpp"
#include <queue>
#include <pthread.h>

class TransBuf {
public:
    std::queue<Frame*> queue;
    pthread_mutex_t mutex;

    void append(Frame* frame);
    Frame* getFirst();
    TransBuf();
    bool isEmpty();
    int size();
    void peekFrontSize();
    //void printSizeAll();
};

#endif