#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

main(void){
     if (msync(0x7ff04d1a5000, 4096,MS_SYNC )< 0 ) {
          perror("msync failed with error:");
          return -1;
     }
     else (void) printf("%s","msync completed successfully.");
}