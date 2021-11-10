#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <stdlib.h> 
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>

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
}
