#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main(){
    int fd1;
    char str1[80],str2[80];
    char * myfifo = "/home/mt/Desktop/os/ask1/magic_mirror";
    mkfifo(myfifo, 0666);
    
    // First open in read only and read
    fd1 = open(myfifo,O_RDONLY);
    read(fd1, str1, 1);
    close(fd1);

    // Now open in write mode and write string taken from user.
    fd1 = open(myfifo,O_WRONLY);
    fgets(str2, 2, stdin);
    write(fd1, str2, 1);
    close(fd1);
}