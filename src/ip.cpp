#include "ip.hpp"

Frame::Frame(char* data, uint8_t size, uint16_t id, uint16_t num, bool end){
    this->data = data;
    this->size = size;
    this->id = id;
    this->num = num;
    this->end = end;
}

char* Frame::serialize() {
    char* ser_data = new char[32];
    ser_data[0] = num >> 4 & 0xFF;
    ser_data[1] = (num << 4 & 0x0F) | (end << 3 & 0xFF) | (size >> 5 & 0xFF);
    ser_data[2] = (size << 6 & 0xF0) | (id >> 8 & 0xFF);
    ser_data[3] = id & 0xFF;

    printf("\n%#08x %#08x %#08x %#08x\n", ser_data[0], ser_data[1], ser_data[2], ser_data[3]);
    for(int i = 4; i < size + 4; i++) {
        ser_data[i] = data[i - 4];
    }
    return ser_data;
}