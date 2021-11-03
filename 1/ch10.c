#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    int fd1;
    char str1[4096];
    char * myfifo = "/home/mt/Desktop/os/ask1/secret_number";
    //mkfifo(myfifo, 0666);
    
    fd1 = open(myfifo,O_RDWR|O_CREAT|O_TRUNC);
    //read(fd1, str1, 4096);
    //printf("%s\n",str1);
    //close(fd1);
    //sleep(30);
    char *const argv[] = {"./riddle",NULL};
    int status = execv(argv[0], argv);
    if (status < 0) {  //τρεχει μονο αν αποτυχει η execv
	perror("Error running executable");
        exit(-1);
    }
    
    
}