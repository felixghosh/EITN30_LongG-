#include "transbuf.hpp"

TransBuf::TransBuf(){
    pthread_mutex_init(&mutex, NULL);
}

void TransBuf::append(Frame* frame){
    pthread_mutex_lock(&mutex);
    queue.push(frame);
    //printf("after push back: %d\n", queue.front()->size);
    pthread_mutex_unlock(&mutex);
}

Frame* TransBuf::getFirst(){
    pthread_mutex_lock(&mutex);
    if(queue.size() <= 0){
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    Frame* frame = queue.front();
    //printf("size in buf get first: %d\n", frame->size);
    queue.pop();
    pthread_mutex_unlock(&mutex);
    return frame;
}

bool TransBuf::isEmpty(){
    pthread_mutex_lock(&mutex);
    bool empty = queue.size() == 0;
    pthread_mutex_unlock(&mutex);
    return empty;
}

int TransBuf::size(){
    pthread_mutex_lock(&mutex);
    int size = queue.size();
    pthread_mutex_unlock(&mutex);
    return size;
}

void TransBuf::peekFrontSize(){
    pthread_mutex_lock(&mutex);
    printf("Peeking front size: %d", queue.front()->size);
    pthread_mutex_unlock(&mutex);
}

/*void TransBuf::printSizeAll() {
    pthread_mutex_lock(&mutex);
    for(int i = 0; i < queue.size(); i++) {
        printf("schlong size: %d\n", queue[i]->size);
    }
    pthread_mutex_unlock(&mutex);

}*/