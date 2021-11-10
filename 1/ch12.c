#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(){
    pid_t pid;
    pid = fork();
    //Για να τρεχουν οι διεργασιες "ταυτοχρονα"
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
        gets(text); //Ονομα αρχειου στην εικονικη μνημη του εκτελεσιμου
        int fd;
        if ((fd=open(text, O_RDWR)) < 0){
            perror("open");
            return 0;
        }
        //πηγαινε στην διευθυνση του αρχειου που θα κανει read το εκτελεσιμο για να γινει εκει μετα το write
        if ((lseek (fd, 111, SEEK_SET)) < 0){ //η ζητουμενη διευθυνση ειναι παντα 0x6f bytes αφου εχει επιστρεψει η διευθυνση απο την mmap 
            perror("lseek");
            return 0;
        }
        char textt[2]; //γραμμα που πρεπει να γραψουμε
        fgets(textt, sizeof(textt), stdin);
        write (fd, textt, 1);
    }
}
