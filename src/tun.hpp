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

void reflect(uint8_t *p, size_t nbytes);
void setup_tun(std::string address);
int tun_alloc(char *dev);
static inline void put32(uint8_t *p, size_t offset, uint32_t n);
static inline uint32_t get32(uint8_t *p, size_t offset);
void swap(uint8_t *p, uint8_t *q, int nbytes);