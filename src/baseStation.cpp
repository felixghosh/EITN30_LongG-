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
#include <arpa/inet.h>
#include <map>
#define MUADDR "192.168.0.3/24"
#define BSADDR "192.168.0.1/24"

#include "tun.hpp"

using namespace std;

void* setup_sender(void* p_radio);
void* setup_receiver(void* p_radio);
void* readTun(void* arg);
void* writeTun(void* arg);

pthread_mutex_t mutex;

RF24 radioSend(17, 0);
RF24 radioReceive(27,60);


char payload[32]; 

TransBuf *transBuf = new TransBuf;
map<int,list<Frame>> recvMap; 

void sender(RF24 radio);  
void receiver(RF24 radio); 

// custom defined timer for evaluating transmission time in microseconds
struct timespec startTimer, endTimer;
uint32_t getMicros(); // prototype to get ellapsed time in microseconds

int main(int argc, char** argv) {
    pthread_t send_thread, receive_thread, read_tun_thread, write_tun_thread;
    pthread_mutex_init(&mutex, NULL);

    //Setup radios
    RF24 *p_radio_send = (RF24*) malloc(sizeof(RF24));
    RF24 *p_radio_receive = (RF24*) malloc(sizeof(RF24));
    *p_radio_send = radioSend;
    *p_radio_receive = radioReceive;

    //Setup tun interface
    setup_tun(BSADDR);

    //Create threads
    pthread_create(&send_thread, NULL, setup_sender, p_radio_send);
    pthread_create(&receive_thread, NULL, setup_receiver, p_radio_receive);
    pthread_create(&read_tun_thread, NULL, readTun, NULL);
    pthread_create(&write_tun_thread, NULL, writeTun, NULL);

    //wait for threads to join
    void *ret;
    if(pthread_join(read_tun_thread, &ret) != 0){
        perror("pthread_join() error");
        exit(3);
    }
    if(pthread_join(write_tun_thread, &ret) != 0){
        perror("pthread_join() error");
        exit(3);
    } 
    if(pthread_join(send_thread, &ret) != 0){
        perror("pthread_join() error");
        exit(3);
    }
    if(pthread_join(receive_thread, &ret) != 0){
        perror("pthread_join() error");
        exit(3);
    }
    else return 0;
    
}

void* readTun(void* arg){
    char readBuf[1024];
    char* reconstructed;
    while(true){
        int x = read_tun(readBuf, sizeof readBuf);
        
        
        string sep = " ";
        if(((readBuf[0] & 0xF0) >> 4) == 4){         //Check for ipv4 packet         
            char destBytes[4];
            for(int i = 16; i < 20; i++)
                destBytes[i - 16] = readBuf[i];
            char dest[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, destBytes, dest, INET_ADDRSTRLEN);

            if(strcmp(dest, BSADDR) != 0){
                fragment_packet(readBuf, x, transBuf);      //Puts frames in transbuf
            }
            else
                printf("For us!\n");
        }
    }
    pthread_exit(NULL);
}

void* writeTun(void* arg){
    char writebuf[1024];
    while(true){
        pthread_mutex_lock(&mutex);
        map<int, list<Frame>>::iterator itr = recvMap.begin();
        for(itr; itr != recvMap.end(); itr++){
            if(itr->second.back().end == true) {
                char* packet = reassemble_packet(itr->second, itr->second.size());
                
                //dumpHex(packet, " ", 84);
                int len = packet[3];
                write_tun(packet, len);
                break;
            }
        }
        recvMap.erase(itr->first);
        pthread_mutex_unlock(&mutex);
        
        
        //usleep(1000000);
    }
    
    pthread_exit(NULL);
}

void* setup_receiver(void* p_radio){
    RF24 radio = *((RF24 *) p_radio);
    free(p_radio);

    // perform hardware check
    if (!radio.begin()) {
        cout << "radio hardware is not responding!!" << endl;
        return 0; // quit now
    }

    radio.setChannel(120);
    radio.setDataRate(RF24_2MBPS);
    // to use different addresses on a pair of radios, we need a variable to
    // uniquely identify which address this radio will use to transmit
    bool radioNumber = 0; // 0 uses address[0] to transmit, 1 uses address[1] to transmit

    // Let these addresses be used for the pair
    uint8_t address[2][6] = {"1Node", "2Node"};
    // It is very helpful to think of an address as a path instead of as
    // an identifying device destination

    radio.setPayloadSize(sizeof(payload));

    radio.setPALevel(RF24_PA_LOW);

    //radio.openWritingPipe(address[radioNumber]);

    radio.openReadingPipe(1, address[!radioNumber]);

    receiver(radio);

    pthread_exit(NULL);

}

void* setup_sender(void* p_radio){
    RF24 radio = *((RF24 *) p_radio);
    free(p_radio);

    // perform hardware check
    if (!radio.begin()) {
        cout << "radio hardware is not responding!!" << endl;
        return 0; // quit now
    }

    radio.setChannel(70);
    radio.setDataRate(RF24_2MBPS);
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

    //radio.openReadingPipe(1, address[!radioNumber]);

    sender(radio);

    pthread_exit(NULL);

}

/**
 * make this node act as the transmitter
 */
void sender(RF24 radio) {
    char* payload;
    radio.stopListening();   
    unsigned int failure = 0;                                       // keep track of failures
    time_t timer;
    time_t t0 = time(&timer); 
    bool finished = false;
    while(!finished){
        while(transBuf->isEmpty()){
            usleep(100000);
        }
        Frame* f = transBuf->getFirst();
        payload = f->serialize();
        clock_gettime(CLOCK_MONOTONIC_RAW, &startTimer);            // start the timer 
        bool success = radio.write(payload, f->size+4);
        uint32_t timerEllapsed = getMicros();                       // end the timer
        
        /*if (success) {
            // payload was delivered
            cout << "Transmission successful! Time to transmit = ";
            cout << timerEllapsed;                                  // print the timer result
            cout << " us. Sent: " << payload << endl;               // print payload sent  
        } else {
            // payload was not delivered
            cout << "Transmission failed or timed out" << endl;
            failure++;
        }*/
        
    }
    time_t t1 = time(&timer);
    double runtime = difftime(t1, t0);
    cout << "Quitting transmission!" << endl;
    return;
    //cout << "Packet Transmission Rate estimated at : " << 100.0 * payload / runtime << " packets/seconds. " << endl;
    //cout << "Packet Error Rate estimated at : " << 100.0 / runtime << " packets/seconds" << " or a failure rate of " << 1.0 / payload << "."<< endl;
}

/**
 * make this node act as the receiver
 */
void receiver(RF24 radio) {
    radio.startListening();                                  // put radio in RX mode

    time_t startTimer = time(nullptr);                       // start a timer
    char message[1024];
    
    bool done = false;
    while(!done){
        bool finished = false;
        int pack = 0;
        //std::list<Frame> frames;
        while (!finished) {                 // use 6 second timeout
            uint8_t pipe;
            if (radio.available(&pipe)) {                        // is there a payload? get the pipe number that recieved it
                uint8_t bytes = radio.getPayloadSize();          // get the size of the payload
                radio.read(&payload, bytes);                     // fetch payload from FIFO
                Frame* f = new Frame(payload);
                int fId = f->id;
                pthread_mutex_lock(&mutex);
                if(recvMap.find(fId) == recvMap.end()){
                    std::list<Frame>* frames = new list<Frame>();
                    recvMap.insert(pair<int,list<Frame>>(fId, *frames));
                }

                recvMap[fId].push_back(*f);
                pthread_mutex_unlock(&mutex);
                if(f->end)
                    finished = true;
                pack++;

                /*cout << "Received " << (unsigned int)bytes;      // print the size of the payload
                cout << " bytes on pipe " << (unsigned int)pipe; // print the pipe number
                cout << ": " << payload << endl;*/                 // print the payload's value
                startTimer = time(nullptr);                      // reset timer
            }
        }
    }
    if(done)
        cout << "Done! Exiting recevie!" << endl;
    else
        cout << "Nu är något snett" << endl;
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
