#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <stdlib.h> 
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>

int create_sparse_file(char *path, u_int64_t size)
{
    int fd = 0; 
    fd = open(path, O_RDWR|O_CREAT, 0666);
    if (fd == -1) {
        return -1;
    }    
    if (lseek(fd, size - 1, SEEK_CUR) == -1) {
        return -1;
    }    
    //write(fd, "\0", 1);
    close(fd);
    return 0;
}

int main(){
    create_sparse_file("bf00",0);
    //int fd1=open("bf00",O_RDWR,0666);
    //write(fd1,"X",1);
    //close(fd1);
}