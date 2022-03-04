#ifndef TUN_HPP
#define TUN_HPP

#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <inttypes.h>
#include <iostream>

#include <linux/if_tun.h>
#include <linux/if.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>


#include "transbuf.hpp"
#include "ip.hpp"
#include <list>


void reflect(uint8_t *p, size_t nbytes);
void setup_tun(std::string address);
int tun_alloc(char *dev);
static inline void put32(uint8_t *p, size_t offset, uint32_t n);
static inline uint32_t get32(uint8_t *p, size_t offset);
void swap(uint8_t *p, uint8_t *q, int nbytes);
int read_tun(char* readbuf, size_t len);
void write_tun(char* writebuf, size_t len);
void dumpHex(char* data, std::string separator, size_t len);
void fragment_packet(char* packbuf, int len, TransBuf* transBuf);
char* reassemble_packet(std::list<Frame> frames, int len);


#endif