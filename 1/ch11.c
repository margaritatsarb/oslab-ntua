#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main() {
    int fd1;
    int stat;
    fd1 = openat(AT_FDCWD, "secret_number", O_RDWR|O_CREAT|O_TRUNC, 0600);
    char str1[72];
    int pid = fork();
    if (pid<0) perror("error");
    else if (pid == 0) {
        //printf("child %d\n",getpid());
        char *const argv[] = {"./riddle",NULL};
        int status = execv(argv[0], argv);
        if (status < 0) {  //τρεχει μονο αν αποτυχει η execv
	    perror("Error running executable");
            exit(-1);
        }
    }
    else {
        //printf("parent %d\n",getpid());
        sleep(5);
        read(fd1, str1, 72);
        printf("%s",str1);
        if(sizeof(str1)==0) printf("no");
        sleep(5);
    }
}