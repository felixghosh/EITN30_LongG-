#ifndef IP_HPP
#define IP_HPP

#include <string>
#include <cstring>



class Frame {   //Fragment of ip packet
public:
    char* data;
    int size;
    int id;
    int num;
    bool end;

    Frame(char* data, uint8_t size, uint16_t id, uint16_t num, bool end);
    char* serialize();
};




#endif