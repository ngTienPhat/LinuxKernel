#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

#define MAX_LENGTH 256 

int main() {
  int fd;
  char numberFromModule[MAX_LENGTH];
  
  fd = open("/dev/RandomGenerator", O_RDONLY);
  if (fd < 0) {
    printf("Failed to open the device...");
    return;
  }
  
  if (read(fd, numberFromModule, MAX_LENGTH) != 0) {
    printf("Failed to generate random number...\n");
    return;
  }
  
  int randNumber;
  memcpy(&randNumber, numberFromModule, sizeof(int));
  printf("Random number generated %d\n", randNumber);
  
  return 0;
}
