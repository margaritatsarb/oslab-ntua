#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <stdlib.h> 
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>

int main(){
    int fd;
    fd = open ("s.txt", O_RDONLY| O_CREAT);
    dup2(fd, 99); //εκχωρει ενα νεο fd που αναφέρεται στην ίδια περιγραφή ανοιχτού αρχείου με το παλιό fd
                  //se antithesi me dup, orizoume emeis ton new fd
    char *const argv[] = {"./riddle",NULL};
    int status = execv(argv[0], argv);
    if (status < 0) {  //τρεχει μονο αν αποτυχει η execv
	perror("Error running executable");
        exit(-1);
    }
}
