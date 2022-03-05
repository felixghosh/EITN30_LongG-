#include <ctime>       // time()
#include <cstring>
#include <iostream>    // cin, cout, endl
#include <string>      // string, getline()
#include <time.h>      // CLOCK_MONOTONIC_RAW, timespec, clock_gettime()
#include <RF24/RF24.h> // RF24, RF24_PA_LOW, delay()
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "ip.hpp"
#include "tun.hpp"
#include "transbuf.hpp"
#include <queue>
#include <list>


using namespace std;

void* sender(void* p_radio);
void* receiver(void* p_radio);
void* readTun(void* arg);
void* writeTun(void* arg);

RF24 radioSend(17, 0);
RF24 radioReceive(27,60);


char payload[32];

TransBuf *transBuf = new TransBuf;

void master(RF24 radio);
void slave(RF24 radio);

// custom defined timer for evaluating transmission time in microseconds
struct timespec startTimer, endTimer;
uint32_t getMicros(); // prototype to get ellapsed time in microseconds

void print_queue(std::queue<Frame*> q)
{
  while (!q.empty())
  {
    std::cout << q.front()->toString() << " ";
    q.pop();
    printf("size of queue: %d\n", q.size());
  }
  std::cout << std::endl;
}

int main(int argc, char** argv) {
    /*queue<Frame*> testq;
    
    char data[28];
    memset(data, 1, 28);
    uint16_t size = 28;
    uint16_t id = 12;
    uint16_t num;
    num = 827;
    Frame test1(data, size, id, num, true);
    num += 1;
    Frame test2(data, size, id, num, true);
    num += 1;
    Frame test3(data, size, id, num, true);
    testq.push(&test1);
    cout << testq.front()->toString() << endl;
    testq.push(&test2);
    cout << testq.front()->toString() << endl;
    testq.push(&test3);
    cout << testq.front()->toString() << endl;
    for(int i = 0; i < 3; i++){
        cout << testq.front()->toString() << endl;
        testq.pop();
    }*/
    //printf("test1: %d\n", test.size);
    //transBuf.append(&test);
    //Frame* f = transBuf.getFirst();
    //printf("test2: %d\n", f->size);

    /*char* c = test.serialize();

    //dumpHex(c, " ", 32);

    Frame test2(c);*/

    pthread_t send, receive, read_tun_thread, write_tun_thread;

    //Setup radiso
    RF24 *p_radio_send = (RF24*) malloc(sizeof(RF24));
    RF24 *p_radio_receive = (RF24*) malloc(sizeof(RF24));
    *p_radio_send = radioSend;
    *p_radio_receive = radioReceive;

    //Setup tun interface
    setup_tun("10.0.0.2");

    //Create threads
    //pthread_create(&send, NULL, sender, p_radio_send);
    //pthread_create(&receive, NULL, receiver, p_radio_receive);
    pthread_create(&read_tun_thread, NULL, readTun, NULL);
    //pthread_create(&write_tun_thread, NULL, writeTun, NULL);
    
    //wait for threads to join
    void *ret;
    if(pthread_join(read_tun_thread, &ret) != 0){
        perror("pthread_create() error");
        exit(3);
    } 
    if(pthread_join(send, &ret) != 0){
        perror("pthread_create() error");
        exit(3);
    }
    else if(pthread_join(receive, &ret) != 0){
        perror("pthread_create() error");
        exit(3);
    }
    else return 0;
}

void* readTun(void* arg){
    char readbuf[1024];
    char reconstructed[1024];
    memset(readbuf, 0, 1024);
    memset(reconstructed, 0, 1024);
    while(true){
        int x = read_tun(readbuf, sizeof readbuf);
        
        std::string sep = " ";
        if(((readbuf[0] & 0xF0) >> 4) == 4){           //Check for ipv4 packet
        
            printf("data read!\n");
            dumpHex(readbuf, sep, x);
            //TransBuf* t = &transBuf;
            //fragment_packet(readbuf, x, transBuf);

            uint16_t num;
            bool end = false;
            char* data;
            memset(data, 0, 28);
            int nbrPack = x % 28 == 0 ? x/28 : x / 28+1;
            uint16_t id = rand() % 16384;
            uint16_t i;
            printf("len: %d\n", x);
            for(num = 0; num < nbrPack; num++){
                data = new char[28];
                for(i = 0; i < 28; i++){
                    if(num*28 + i >= x-1){
                        end = true; 
                    }
                    data[i] = readbuf[i+(28*num)];
                }
                //printf("ASIGNING SIZE: %d\n", i);
                Frame* f = new Frame(data, i + 0, id + 0, num + 0, end + 0);
                printf("address %d\n", f);
                //Frame f2(data, i + 0, id + 0, num + 10, end + 0);
                //std::cout << f.toString() << std::endl;
                dumpHex(f->data, " ", i);
                transBuf->queue.push(f);
                //std::cout << transBuf->queue.front()->toString() << std::endl;
                /*transBuf->queue.push(&f2);
                std::cout << transBuf->queue.front()->toString() << std::endl;*/
            }
            /*for(int i = 0; i < 3; i++){
                cout << transBuf->queue.front()->toString() << endl;
                //cout << transBuf->queue.back()->toString() << endl;
                transBuf->queue.pop();
            }*/
            //print_queue(transBuf->queue);


            //printf("pointer after fragment: %d\n", transBuf);
            //t->printSizeAll();
            //printf("packet fragmented!\n");
            
            std::list<Frame> frames;
            transBuf->peekFrontSize();
            int len = transBuf->size();
            
            for(int i = 0; i < len; i++){
                //dumpHex((*transBuf.queue.front()).data, " ", 28);//frames.front().data);
                //printf("size: %d\n", transBuf->size());
                Frame* f = transBuf->queue.front();//transBuf->getFirst();
                transBuf->queue.pop();
                std::cout << f->toString() << std::endl;
                //printf("Framesize: %d\n", f->getSize());
                dumpHex(f->data, " ", 28);
                frames.push_front(*f);
                //dumpHex(frames.front().data, " ", frames.front().size);
            }
            strcpy(reconstructed, reassemble_packet(frames, len));
            printf("packet reassembled!\n");
            dumpHex(reconstructed, sep, x);
        }
        //print_queue(transBuf.queue);
    }
    pthread_exit(NULL);
}

void* writeTun(void* arg){
    char writebuf[1024];
    while(true){
        memset(writebuf, 0, 1024);
        strcpy(writebuf, "TJA");
        write_tun(writebuf, sizeof writebuf);
        //printf("NUUUU");
        usleep(1000000);
    }
    
    pthread_exit(NULL);
}

void* receiver(void* p_radio){
    RF24 radio = *((RF24 *) p_radio);
    free(p_radio);

    // perform hardware check
    if (!radio.begin()) {
        cout << "radio hardware is not responding!!" << endl;
        return 0; // quit now
    }

    radio.setChannel(70);
    // to use different addresses on a pair of radios, we need a variable to
    // uniquely identify which address this radio will use to transmit
    bool radioNumber = 0; // 0 uses address[0] to transmit, 1 uses address[1] to transmit

    // Let these addresses be used for the pair
    uint8_t address[2][6] = {"1Node", "2Node"};
    // It is very helpful to think of an address as a path instead of as
    // an identifying device destination

    radio.setPayloadSize(sizeof(payload));

    radio.setPALevel(RF24_PA_LOW);

    radio.openWritingPipe(address[radioNumber]);

    radio.openReadingPipe(1, address[!radioNumber]);

    slave(radio);

    pthread_exit(NULL);

}

void* sender(void* p_radio){
    RF24 radio = *((RF24 *) p_radio);
    free(p_radio);

    // perform hardware check
    if (!radio.begin()) {
        cout << "radio hardware is not responding!!" << endl;
        return 0; // quit now
    }

    radio.setChannel(120);
    // to use different addresses on a pair of radios, we need a variable to
    // uniquely identify which address this radio will use to transmit
    bool radioNumber = 1; // 0 uses address[0] to transmit, 1 uses address[1] to transmit

    // Let these addresses be used for the pair
    uint8_t address[2][6] = {"1Node", "2Node"};
    // It is very helpful to think of an address as a path instead of as
    // an identifying device destination;

    radio.setPayloadSize(sizeof(payload));

    radio.setPALevel(RF24_PA_LOW);

    radio.openWritingPipe(address[radioNumber]);

    radio.openReadingPipe(1, address[!radioNumber]);

    master(radio);

    pthread_exit(NULL);

}

void master(RF24 radio) {
    radio.stopListening();   
    unsigned int failure = 0;                                       // keep track of failures
    time_t timer;
    time_t t0 = time(&timer); 
    bool finished = false;
    while(!finished){
        /*while(transBuf.isEmpty()){
            usleep(100000);
        }*/
        
    }
    time_t t1 = time(&timer);
    double runtime = difftime(t1, t0);
    cout << "Quitting transmission!" << endl;
    return;
    //cout << "Packet Transmission Rate estimated at : " << 100.0 * payload / runtime << " packets/seconds. " << endl;
    //cout << "Packet Error Rate estimated at : " << 100.0 / runtime << " packets/seconds" << " or a failure rate of " << 1.0 / payload << "."<< endl;
}

void slave(RF24 radio) {
    radio.startListening();                                  // put radio in RX mode

    time_t startTimer = time(nullptr);                       // start a timer
    char message[1024];
    
    bool done = false;
    while(!done && time(nullptr) - startTimer < 600){
        bool finished = false;
        int pack = 0;
        while (time(nullptr) - startTimer < 60 && !finished) {                 // use 6 second timeout
            uint8_t pipe;
            if (radio.available(&pipe)) {                        // is there a payload? get the pipe number that recieved it
                uint8_t bytes = radio.getPayloadSize();          // get the size of the payload
                radio.read(&payload, bytes);                     // fetch payload from FIFO

                for(int i = 0; i < 32; i++){
                    message[i + (32*pack)] = payload[i];
                    if(payload[i] == '\0'){
                        finished = true;
                        break;
                    }
                }
                pack++;

                cout << "Received " << (unsigned int)bytes;      // print the size of the payload
                cout << " bytes on pipe " << (unsigned int)pipe; // print the pipe number
                cout << ": " << payload << endl;                 // print the payload's value
                startTimer = time(nullptr);                      // reset timer
            }
        }
        if(finished)
            cout << "Full message received: " << message << endl;
    }
    if(done)
        cout << "Done! Exiting recevie!" << endl;
    else
        cout << "Nothing received in 60 seconds. Leaving RX role." << endl;
    radio.stopListening();
}


/**
 * Calculate the ellapsed time in microseconds
 */
uint32_t getMicros() {
    // this function assumes that the timer was started using
    // `clock_gettime(CLOCK_MONOTONIC_RAW, &startTimer);`

    clock_gettime(CLOCK_MONOTONIC_RAW, &endTimer);
    uint32_t seconds = endTimer.tv_sec - startTimer.tv_sec;
    uint32_t useconds = (endTimer.tv_nsec - startTimer.tv_nsec) / 1000;

    return ((seconds) * 1000 + useconds) + 0.5;
}
