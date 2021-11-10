#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <stdlib.h> 
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>
#include <sys/file.h>

int main(){
    int fd=open("/proc/sys/kernel/ns_last_pid",O_RDWR | O_CREAT, 0644);
    printf("%d\n",fd);
    int f = flock(fd, LOCK_EX); //μονο αυτη η διεργασία μπορεί να κρατά αποκλειστικό κλείδωμα για αυτο το αρχειο
    if (f==-1) perror("flock");
    ftruncate(fd, 0); //στην περιπτωση που fd που εχει ειναι μεγαλυτερος απο 5 bytes
    write(fd,"32766",5);
    pid_t pid;
    //κανουμε fork ετσι ωστε η επομενη διεργασια να περιεχει το ζητουμενο pid το οποιο κληρονομει το εκτελεσιμο καθως τρεχει με execv
    pid = fork(); 
    if (pid<0) perror("error");
    else if (pid == 0) {
        //32767
        char *const argv[] = {"./riddle",NULL};
        int status = execv(argv[0], argv);
        if (status < 0) {  //τρεχει μονο αν αποτυχει η execv
	    perror("Error running executable");
            exit(-1);
        }
        exit(0);
    }
    else exit(0);
    if (flock(fd, LOCK_UN)) { //Αφαιρει την υπάρχουσα κλειδαριά που διατηρείται από αυτήν τη διαδικασία
        perror("flock");
    }
    close(fd);
    return 0;
}
