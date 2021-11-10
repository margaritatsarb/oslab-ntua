#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <stdlib.h> 
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>

int main(){
    int fd1[2];
    int fd2[2];
    //Δημιουργουμε 2 διοχετευσεις, καναλια δεδομενων μονης κατευθυνσης που χρησιμοποιουμε για επικοινωνια μεταξυ διεργασιων
    pipe(fd1); //παραμετρος η διευθυνση του 1ου στοιχειου ετσι ωστε αν εκτελεστει επιτυχως να δημιουργησει μια διοχετευση και να εγγραψει στις 2 αυτες διευθυνσεις ακεραιων το ακρο αναγνωσης και το ακρο εγγραφης
    pipe(fd2);
    dup2(fd1[0], 33); //akro read
    dup2(fd1[1], 34); //akro write
    dup2(fd2[0], 53); //akro read
    dup2(fd2[1], 54); //akro write
    
    char *const argv[] = {"./riddle",NULL};
    int status = execv(argv[0], argv);
    if (status < 0) {  //τρεχει μονο αν αποτυχει η execv
	perror("Error running executable");
        exit(-1);
    }
}
