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


using namespace std;

void* sender(void* p_radio);
void* receiver(void* p_radio);
void* readTun(void* arg);
void* writeTun(void* arg);

RF24 radioSend(17, 0);
RF24 radioReceive(27,60);


char payload[32];

TransBuf transBuf;

void master(RF24 radio);
void slave(RF24 radio);

// custom defined timer for evaluating transmission time in microseconds
struct timespec startTimer, endTimer;
uint32_t getMicros(); // prototype to get ellapsed time in microseconds

void print_queue(std::deque<Frame*> q)
{
  while (!q.empty())
  {
    std::cout << q.front() << " ";
    q.pop_front();
  }
  std::cout << std::endl;
}

int main(int argc, char** argv) {
    char data[28];
    memset(data, 1, 28);
    uint8_t size = 28;
    uint16_t id = 12;
    uint16_t num = 827;
    Frame test(data, size, id, num, true);

    char* c = test.serialize();

    dumpHex(c, " ", sizeof c);

    pthread_t send, receive, read_tun_thread, write_tun_thread;

    //Setup radiso
    RF24 *p_radio_send = (RF24*) malloc(sizeof(RF24));
    RF24 *p_radio_receive = (RF24*) malloc(sizeof(RF24));
    *p_radio_send = radioSend;
    *p_radio_receive = radioReceive;

    //Setup tun interface
    setup_tun("10.0.0.2");

    //Create threads
    pthread_create(&send, NULL, sender, p_radio_send);
    pthread_create(&receive, NULL, receiver, p_radio_receive);
    pthread_create(&read_tun_thread, NULL, readTun, NULL);
    pthread_create(&write_tun_thread, NULL, writeTun, NULL);
    
    //wait for threads to join
    void *ret; 
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
    printf("READTIME!\n");
    char readbuf[1024];
    memset(readbuf, 0, 1024);
    while(true){
        int x = read_tun(readbuf, sizeof readbuf);
        std::string data(readbuf);
        cout << data << endl;
        std::string sep = " ";
        dumpHex(readbuf, sep, x);
        fragment_packet(readbuf, x, &transBuf);
        print_queue(transBuf.queue);
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
        cout << "please enter the message you would like to send:" << endl;
        string temp;
        getline(cin, temp);
        
        char message[1024];
        
        strcpy(message, temp.c_str());
        if(!strcmp(message, "quit")){
            finished = true;
            break;
        }
        cout << "SIZE OF TEMP " << temp.length() << endl;
        int nbrPack = temp.length() / 32 + 1;   
        int i;
        int pack = 0;                                 
        while (failure < 1000 && pack < nbrPack) {
            for(i = 0; i < 32; i++){
                if(message[i+(32*pack)] == '\0'){
                    payload[i] == '\0'; 
                    break;
                }
                else
                    payload[i] = message[i+(32*pack)];
            }
            
            clock_gettime(CLOCK_MONOTONIC_RAW, &startTimer);            // start the timer
            bool report = radio.write(&payload, i);         // transmit & save the report
            uint32_t timerEllapsed = getMicros();                       // end the timer

            if (report) {
                // payload was delivered
                cout << "Transmission successful! Time to transmit = ";
                cout << timerEllapsed;                                  // print the timer result
                cout << " us. Sent: " << payload << endl;               // print payload sent
                //payload += 0.01;                                        // increment float payload
                pack++;   
            } else {
                // payload was not delivered
                cout << "Transmission failed or timed out" << endl;
                failure++;
            }

            // to make this example readable in the terminal
            //delay(1000);  // slow transmissions down by 1 second
        }
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
    while(!done && time(nullptr) - startTimer < 60){
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
