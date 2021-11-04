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
    printf("%d",fd);
    if (fd == -1) {
        return -1;
    }    
    if (lseek(fd, size - 1, SEEK_SET) == -1) {
        return -1;
    }    
    //write(fd, "\0", 1);
}

int main(){
    int fd = open("bf00", O_RDWR|O_CREAT, 0666);
    lseek(fd,1073741824, SEEK_SET);
    write(fd,"X",1);
    close(fd);
    fd = open("bf01", O_RDWR|O_CREAT, 0666);
    lseek(fd,1073741824, SEEK_SET);
    write(fd,"X",1);
    close(fd);
    fd = open("bf02", O_RDWR|O_CREAT, 0666);
    lseek(fd,1073741824, SEEK_SET);
    write(fd,"X",1);
    close(fd);
    fd = open("bf03", O_RDWR|O_CREAT, 0666);
    lseek(fd,1073741824, SEEK_SET);
    write(fd,"X",1);
    close(fd);
    fd = open("bf04", O_RDWR|O_CREAT, 0666);
    lseek(fd,1073741824, SEEK_SET);
    write(fd,"X",1);
    close(fd);
    fd = open("bf05", O_RDWR|O_CREAT, 0666);
    lseek(fd,1073741824, SEEK_SET);
    write(fd,"X",1);
    close(fd);
    fd = open("bf06", O_RDWR|O_CREAT, 0666);
    lseek(fd,1073741824, SEEK_SET);
    write(fd,"X",1);
    close(fd);
    fd = open("bf07", O_RDWR|O_CREAT, 0666);
    lseek(fd,1073741824, SEEK_SET);
    write(fd,"X",1);
    close(fd);
    fd = open("bf08", O_RDWR|O_CREAT, 0666);
    lseek(fd,1073741824, SEEK_SET);
    write(fd,"X",1);
    close(fd);
    fd = open("bf09", O_RDWR|O_CREAT, 0666);
    lseek(fd,1073741824, SEEK_SET);
    write(fd,"X",1);
    close(fd);
    //close(3);
    //int fd1=open("bf00",O_RDWR,0666);
    //write(fd1,"X",1);
    //close(fd1);
}
