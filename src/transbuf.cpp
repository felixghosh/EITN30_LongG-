#include "transbuf.hpp"

TransBuf::TransBuf(){
    mutex = PTHREAD_MUTEX_INITIALIZER;
}

void TransBuf::append(Frame* frame){
    pthread_mutex_lock(&mutex);
    queue.push_back(frame);
    pthread_mutex_unlock(&mutex);
}

Frame* TransBuf::getFirst(){
    pthread_mutex_lock(&mutex);
    if(queue.size() <= 0){
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    Frame* frame = queue.front();
    queue.pop_front();
    pthread_mutex_unlock(&mutex);
    return frame;
}