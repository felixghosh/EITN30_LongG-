#include "ip.hpp"
#include "tun.hpp"

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
    ser_data[1] = (num << 4 & 0xF0) | (end << 3 & 0xFF) | (size >> 2 & 0xFF);
    ser_data[2] = (size << 6 & 0xC0) | (id >> 8 & 0xFF);
    ser_data[3] = id & 0xFF;

    //printf("\n%#08x %#08x %#08x %#08x\n", ser_data[0], ser_data[1], ser_data[2], ser_data[3]);
    for(int i = 4; i < size + 4; i++) {
        ser_data[i] = data[i - 4];
        //printf("%#08x ", data[i - 4]);
    }
    return ser_data;
}

Frame::Frame(char* buf){
    uint16_t num = (buf[0] << 4) | ((buf[1] &0xF0) >> 4);
    bool end = (buf[1] & 0x08) >> 3;
    uint8_t size = ((buf[1] & 0x07) << 2) | ((buf[2] & 0xC0) >> 6);
    uint16_t id = ((buf[2] & 0x3F) << 8) | (buf[3] & 0xFF);
    char* data = new char[size];
    for(int i = 4; i < size - 4; i++){
        data[i-4] = buf[i];
    }
    //printf("num: %d, end: %d, size: %d, id: %d\n", num, end, size, id);
    this->data = data;
    this->size = size;
    this->id = id;
    this->num = num;
    this->end = end;
}