#ifndef IP_HPP
#define IP_HPP

#include <string>
#include <cstring>



class Frame {   //Fragment of ip packet
public:
    char* data;
    uint16_t size;
    uint16_t id;
    uint16_t num;
    bool end;

    Frame(char* data, uint16_t size, uint16_t id, uint16_t num, bool end);
    Frame(char* buf);
    char* serialize();
    std::string toString();
    int getSize();
};




#endif