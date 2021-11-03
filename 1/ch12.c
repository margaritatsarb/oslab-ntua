#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(){
    char *const argv[] = {"./riddle",NULL};
    int status = execv(argv[0], argv);
    if (status < 0) {  //τρεχει μονο αν αποτυχει η execv
	perror("Error running executable");
        exit(-1);
    }
    unsigned char *f;
    struct stat statbuf;
    //const char *filepath = argv[1];
    int fd = open("secret_number", O_RDWR);
    printf("%d\n",fd);
    //int status = fstat (fd, & statbuf);
    int size=statbuf.st_size;
    f = (char *) mmap(NULL,4096,PROT_READ|PROT_WRITE,MAP_SHARED,4,0);
    //close(fd);
    for (int i = 0; i < size; i++) {
        char c;
        c = f[i];
        putchar(c);
    }
    
}