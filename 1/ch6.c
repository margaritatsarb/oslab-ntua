#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <stdlib.h> 
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>

int main(){
    int fd,a;
    fd = open ("s.txt", O_RDWR);
    dup2(fd, 33);
    dup2(fd, 54);
    dup2(fd, 34);
    fd = open ("ss.txt", O_RDWR);
    dup2(fd, 53);
    
    char *const argv[] = {"./riddle",NULL};
    int status = execv(argv[0], argv);
    if (status < 0) {  //τρεχει μονο αν αποτυχει η execv
	perror("Error running executable");
        exit(-1);
    }
}