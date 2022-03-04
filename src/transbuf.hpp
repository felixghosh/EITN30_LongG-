#ifndef TRANSBUF
#define TRANSBUF

#include "ip.hpp"
#include <deque>
#include <pthread.h>

class TransBuf {
public:
    std::deque<Frame*> queue;
    pthread_mutex_t mutex;

    void append(Frame* frame);
    Frame* getFirst();
    TransBuf();
    bool isEmpty();
    int size();
    void peekFrontSize();
    void printSizeAll();
};

#endif