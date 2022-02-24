/*
 * See documentation at https://nRF24.github.io/RF24
 * See License information at root directory of this library
 * Author: Brendan Doherty (2bndy5)
 */

/**
 * A simple example of sending data from 1 nRF24L01 transceiver to another.
 *
 * This example was written to be used on 2 devices acting as "nodes".
 * Use `ctrl+c` to quit at any time.
 */
#include <ctime>       // time()
#include <cstring>
#include <iostream>    // cin, cout, endl
#include <string>      // string, getline()
#include <time.h>      // CLOCK_MONOTONIC_RAW, timespec, clock_gettime()
#include <RF24/RF24.h> // RF24, RF24_PA_LOW, delay()
#include <pthread.h>
#include <string.h>

using namespace std;

void* sender(void* p_radio);
void* receiver(void* p_radio);

/****************** Linux ***********************/
// Radio CE Pin, CSN Pin, SPI Speed
// CE Pin uses GPIO number with BCM and SPIDEV drivers, other platforms use their own pin numbering
// CS Pin addresses the SPI bus number at /dev/spidev<a>.<b>
// ie: RF24 radio(<ce_pin>, <a>*10+<b>); spidev1.0 is 10, spidev1.1 is 11 etc..

// Generic:
RF24 radioSend(17, 0);
RF24 radioReceive(27,60);

/****************** Linux (BBB,x86,etc) ***********************/
// See http://nRF24.github.io/RF24/pages.html for more information on usage
// See http://iotdk.intel.com/docs/master/mraa/ for more information on MRAA
// See https://www.kernel.org/doc/Documentation/spi/spidev for more information on SPIDEV

// For this example, we'll be using a payload containing
// a single float number that will be incremented
// on every successful transmission
//float payload = 0.0;

char payload[32]; 


void master(RF24 radio);  // prototype of the TX node's behavior
void slave(RF24 radio);   // prototype of the RX node's behavior

// custom defined timer for evaluating transmission time in microseconds
struct timespec startTimer, endTimer;
uint32_t getMicros(); // prototype to get ellapsed time in microseconds

int main(int argc, char** argv) {

   /* // perform hardware check
    if (!radio.begin()) {
        cout << "radio hardware is not responding!!" << endl;
        return 0; // quit now
    }

    radio.setChannel(120);
    // to use different addresses on a pair of radios, we need a variable to
    // uniquely identify which address this radio will use to transmit
    bool radioNumber = 1; // 0 uses address[0] to transmit, 1 uses address[1] to transmit

    // print example's name
    cout << argv[0] << endl;

    // Let these addresses be used for the pair
    uint8_t address[2][6] = {"1Node", "2Node"};
    // It is very helpful to think of an address as a path instead of as
    // an identifying device destination

    // Set the radioNumber via the terminal on startup
    cout << "Which radio is this? Enter '0' or '1'. Defaults to '0' ";
    string input;
    getline(cin, input);
    radioNumber = input.length() > 0 && (uint8_t)input[0] == 49;

    // save on transmission time by setting the radio to only transmit the
    // number of bytes we need to transmit a float
    radio.setPayloadSize(sizeof(payload)); // float datatype occupies 4 bytes

    // Set the PA Level low to try preventing power supply related problems
    // because these examples are likely run with nodes in close proximity to
    // each other.
    radio.setPALevel(RF24_PA_LOW); // RF24_PA_MAX is default.

    // set the TX address of the RX node into the TX pipe
    radio.openWritingPipe(address[radioNumber]);     // always uses pipe 0

    // set the RX address of the TX node into a RX pipe
    radio.openReadingPipe(1, address[!radioNumber]); // using pipe 1

    // For debugging info
    // radio.printDetails();       // (smaller) function that prints raw register values
    // radio.printPrettyDetails(); // (larger) function that prints human readable data

    // ready to execute program now
    setRole(); // calls master() or slave() based on user input*/

    pthread_t send, receive;
    RF24 *p_radio_send = (RF24*) malloc(sizeof(RF24));
    RF24 *p_radio_receive = (RF24*) malloc(sizeof(RF24));
    *p_radio_send = radioSend;
    *p_radio_receive = radioReceive;
    pthread_create(&send, NULL, sender, p_radio_send);
    pthread_create(&receive, NULL, receiver, p_radio_receive);
    while(true){  
        void *ret; 
        if(pthread_join(send, &ret) != 0){
            perror("pthread_create() error");
            exit(3);
        }
        else return 0;
    }
}

void* receiver(void* p_radio){
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
    bool radioNumber = 0; // 0 uses address[0] to transmit, 1 uses address[1] to transmit

    // Let these addresses be used for the pair
    uint8_t address[2][6] = {"1Node", "2Node"};
    // It is very helpful to think of an address as a path instead of as
    // an identifying device destination

    radio.setPayloadSize(sizeof(payload));

    radio.setPALevel(RF24_PA_LOW);

    //radio.openWritingPipe(address[radioNumber]);

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

    radio.setChannel(70);
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

    master(radio);

    pthread_exit(NULL);

}





/**
 * make this node act as the transmitter
 */
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

/**
 * make this node act as the receiver
 */
void slave(RF24 radio) {
    radio.startListening();                                  // put radio in RX mode

    time_t startTimer = time(nullptr);                       // start a timer
    char message[1024];
    
    bool done = false;
    while(!done){
        bool finished = false;
        int pack = 0;
        while (!finished) {                 // use 6 second timeout
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
