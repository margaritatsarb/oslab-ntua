#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <stdlib.h> 
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>

int main(){
    int fd;
    int fd1[2];
    int fd2[2];
    pipe(fd1);
    pipe(fd2);
    dup2(fd1[0], 33);
    dup2(fd1[1], 34);
    dup2(fd2[0], 53);
    dup2(fd2[1], 54);
    
    char *const argv[] = {"./riddle",NULL};
    int status = execv(argv[0], argv);
    if (status < 0) {  //τρεχει μονο αν αποτυχει η execv
	perror("Error running executable");
        exit(-1);
    }
}
