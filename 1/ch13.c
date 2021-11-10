#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(){
    int fd=open(".hello_there",O_RDWR);
    //Παιρνουμε SIGBUS επειδη προσπαθει το εκτελεσιμο να γραψει περα απο το τελος της mapped περιοχης του αρχειου => κανουμε το αρχειο να εχει μεγεθος τοσο ωστε να μην συμβαινει αυτο
    ftruncate(fd, 32768); //resize the file to 32KB 
 }
