#include "tun.hpp"
#include <stdlib.h>
#include <list>

#define CHECKAUX(e,s)                            \
 ((e)? (void)0: (fprintf(stderr, "'%s' failed at %s:%d - %s\n", s, __FILE__, __LINE__,strerror(errno)), exit(0)))
#define CHECK(e) (CHECKAUX(e,#e))
#define CHECKSYS(e) (CHECKAUX((e)==0,#e))
#define CHECKFD(e) (CHECKAUX((e)>=0,#e))
#define STRING(e) #e

void dumpHex(char* data, std::string separator, size_t len) {
  std::string result;

  for (unsigned int i = 0; i < len; i++){
    if (i > 0)
      std::cout << separator;
    printf("%02x", data[i]);
  }
  printf("\n");
} 

static int tun_fd;

int tun_alloc(char *dev) 
{
  assert(dev != NULL);
  int fd = open("/dev/net/tun", O_RDWR);
  CHECKFD(fd);
 
  struct ifreq ifr; 
  memset(&ifr, 0, sizeof(ifr)); 
  ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
  strncpy(ifr.ifr_name, dev, IFNAMSIZ); 
  CHECKSYS(ioctl(fd, TUNSETIFF, (void *) &ifr));
  strncpy(dev, ifr.ifr_name, IFNAMSIZ); 
  return fd;
}

void setup_tun(std::string address){
  char dev[IFNAMSIZ+1];
   memset(dev,0,sizeof(dev));
   strncpy(dev, "lg0", 3);
   // Allocate the tun device
  tun_fd = tun_alloc(dev);
  if (tun_fd < 0){
    printf("Error creating tun device!");
    exit(0);
  }

  system("sudo /sbin/ifconfig lg0 up");

  std::string addr_command = "sudo ip addr add " + address + " dev lg0";
  system(addr_command.c_str());
}

void fragment_packet(char* packbuf, int len, TransBuf* transBuf){
    uint16_t num;
    bool end = false;
    char data[28];
    int nbrPack = len % 28 == 0 ? len/28 : len / 28+1;
    uint16_t id = rand() % 16384;
    printf("len: %d\n", len);
    for(num = 0; num < nbrPack; num++){
        uint16_t i;
        for(i = 0; i < 28; i++){
            if(num*28 + i >= len){
                data[i] = '\0';
                end = true; 
                break;
            }
            data[i] = packbuf[i+(28*num)];
        }
        printf("ASIGNING SIZE: %d\n", i);
        Frame frame(data, i, id, num, end);
        //std::cout << frame.toString() << std::endl;
        //dumpHex(frame.data, " ", i);
        transBuf->append(&frame);
    }
    printf("pointer in fp: %d\n", transBuf);

}

char* reassemble_packet(std::list<Frame> frames, int len){
  frames.sort([](const Frame & a, const Frame & b){return a.num < b.num;});
  char* packbuf = new char[1024];
  memset(packbuf, 0, 1024);
  for(int i = 0; i < len; i++){
    strcat(packbuf, frames.front().data);
    frames.pop_front();
  }
  return packbuf;
}

int read_tun(char* readbuf, size_t len){
  //printf("tun_fd: %d\n", tun_fd);
  return read(tun_fd, readbuf, len);
}

void write_tun(char* writebuf, size_t len){
  write(tun_fd, writebuf, len);
}

/*int main(int argc, char *argv[])
{
  char dev[IFNAMSIZ+1];
  memset(dev,0,sizeof(dev));
  //if (argc > 1) strncpy(dev,argv[1],sizeof(dev)-1);
  strncpy(dev, "lg0", 3);
 
  // Allocate the tun device
  int fd = tun_alloc(dev);
  if (fd < 0) exit(0);
 
  uint8_t buf[2048];
  while(true) {
    std::cout << "här"; 
    // Sit in a loop, read a packet from fd, reflect
    // addresses and write back to fd.
    ssize_t nread = read(fd,buf,sizeof(buf));
    CHECK(nread >= 0);
    if (nread == 0) break;
    reflect(buf,nread);
    ssize_t nwrite = write(fd,buf,nread);
    CHECK(nwrite == nread);
  }
}*/

static inline void put32(uint8_t *p, size_t offset, uint32_t n)
{
  memcpy(p+offset,&n,sizeof(n));
}
 
static inline uint32_t get32(uint8_t *p, size_t offset)
{
  uint32_t n;
  memcpy(&n,p+offset,sizeof(n));
  return n;
}

void swap(uint8_t *p, uint8_t *q, int nbytes)
{
  for (int i = 0; i < nbytes; i++) {
    uint8_t t = *p; *p = *q; *q = t;
    p++; q++;
  }
}
 
void reflect(uint8_t *p, size_t nbytes)
{  
  printf("received packet!\n");
  (void)nbytes;
  uint8_t version = p[0] >> 4;
  switch (version) {
  case 4:
    
    break;
  case 6:
    swap(p+8, p+24, 16);
    break;
  default:
    fprintf(stderr, "Unknown protocol %u\n", version);
    exit(0);
  }
  uint32_t src = get32(p,12);
    uint32_t dst = get32(p,16);
    put32(p,12,dst);
    put32(p,16,src);
}