#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(){
    pid_t pid;
    pid = fork();
    if (pid<0) perror("error");
    else if (pid == 0) {
        char *const argv[] = {"./riddle",NULL};
        int status = execv(argv[0], argv);
        if (status < 0) {  //τρεχει μονο αν αποτυχει η execv
	    perror("Error running executable");
            exit(-1);
        }
    }
    else {
        char text[20];
        gets(text);
        int fd;
        if ((fd=open(text, O_RDWR)) < 0){
            perror("open");
            return 0;
        }
        if ((lseek (fd, 111, SEEK_SET)) < 0){
            perror("lseek");
            return 0;
        }
        char textt[2];
        fgets(textt, sizeof(textt), stdin);
        write (fd, textt, 1);
        mmap (0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    
    }
}
