#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     //close
#include <fcntl.h>
#include <sys/types.h>  //socket
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <ctype.h>
#include <sys/select.h> //include the whole family
#include <sys/socket.h> //socket
#include <arpa/inet.h>
#include <netdb.h>      //gethostbyname
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <crypto/cryptodev.h>

#include "socket-common.h"
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define DATA_SIZE       256
#define BLOCK_SIZE      16
#define KEY_SIZE        16  /* AES128 */

unsigned char buf[256];
unsigned char key[] = "orjfjhfividkinm";        //encryption key
unsigned char inv[] = "dfijsoifjieifji";        //initialization vector
struct session_op sess;

/*Insist until all of the data has been written*/
ssize_t insist_write(int fd, const void *buf, size_t cnt){
	ssize_t ret;
	size_t orig_cnt = cnt;

	while (cnt > 0) {
	        ret = write(fd, buf, cnt);
	        if (ret < 0)
	            return ret;
	        buf += ret;
	        cnt -= ret;
	}

	return orig_cnt;
}

int encrypt(int cfd){
        int i;
        struct crypt_op cryp; //περιέχει πεδία όπως source, destination, operation
        //πεδία που δέχονται την είσοδο και φιλοξενούν την έξοδο κάθε ενέργειας encrypt/decrypt
				struct {
                unsigned char   in[DATA_SIZE], //τα μηνύματά μας μεταδίδονται σε “κβάντα” των 256 χαρκτήρων
                                encrypted[DATA_SIZE], //>256 στο πρώτο read θα διαβαστούν οι πρώτοι χαρακτήρες και στη συνέχεια, στην επόμενη επανάληψη οι υπόλοιποι
                                iv[BLOCK_SIZE];
        } data; //crypto structure

        memset(&cryp, 0, sizeof(cryp));

        /*Encrypt data.in to data.encrypted*/
        cryp.ses = sess.ses;
        cryp.len = sizeof(data.in);  //fixed στο μέγεθος του data.in
        cryp.src = buf; //buffer με τα δεδομένα που θα κρυπτογραφηθούν
        cryp.dst = data.encrypted; //buffer με τα δεδομένα που θα κρυπτογραφηθούν
        cryp.iv = inv; //στη συνεχεια αρχικοποιείται σε μηδενικά
        cryp.op = COP_ENCRYPT;
        //για να χρησιμοποιησουμε την συσκευή /dev/crypto
        if (ioctl(cfd, CIOCCRYPT, &cryp)) {
                perror("ioctl(CIOCCRYPT)");
                return 1;
        }

        i = 0;
				//συμπληρώνουμε το μηνυμα με τον χαρακτήρα ΄\0΄ μέχρι να γεμίσει ο buffer
				//έτσι ώστε το bufsize να είναι σύμφωνα με τις προδιαγραφές του cryptodev
        memset(buf, '\0', sizeof(buf));
        while(data.encrypted[i] != '\0'){
                buf[i] = data.encrypted[i];
                i++;
        }
        return 0;
}

int decrypt(int cfd){
        int i;
        struct crypt_op cryp;
        struct {
                unsigned char   in[DATA_SIZE],
                                decrypted[DATA_SIZE],
                                iv[BLOCK_SIZE];
        } data;

        memset(&cryp, 0, sizeof(cryp));

        /*Decrypt data.encrypted to data.decrypted*/
        cryp.ses = sess.ses;
        cryp.len = sizeof(data.in);
        cryp.src = buf;
        cryp.dst = data.decrypted;
        cryp.iv = inv;
        cryp.op = COP_DECRYPT;
				/* Decrypt data */
        if (ioctl(cfd, CIOCCRYPT, &cryp)) {
                perror("ioctl(CIOCCRYPT)");
                return 1;
        }

        i = 0;
        memset(buf, '\0', sizeof(buf));
        while(data.decrypted[i] != '\0'){
                buf[i] = data.decrypted[i];
                i++;
        }
        return 0;
}

int main(int argc, char **argv){
    char addrstr[INET_ADDRSTRLEN];
    int sd, newsd, cfd;
    struct sockaddr_in sa;
    socklen_t len;
    ssize_t n;

    memset(&sess, 0, sizeof(sess));

    /*Make sure a broken caonnection doesn't kill us*/
    signal(SIGPIPE, SIG_IGN);// If all file descriptors referring to the read end of a pipe have been closed, then a
                              //write(2) will cause a SIGPIPE signal to be generated for the calling process.

    /*Create TCP/IP socket, used as the main chat channel*/
    if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) { //protocol family IPv4, TCP
                                                      //domain=PF_INET: refers to anything in the protocol, usually sockets/ports
                                                      //type=SOCK_STREAM: two-way, connection-based byte streams,Μεταδοση δεδομενων πανω στο καναλι επικοινωνιας
                                                      //protocol=0: Αφηνουμε το συστημα να επιλεξει το πρωτοκολλο το οποιο θα υλοποιησει την  επικοινωνια πανω στο καναλι
        perror("socket");
        exit(1);
    }
    fprintf(stderr, "Created TCP socket\n");

    /*Bind to a well-known port*/
    memset(&sa, 0, sizeof(sa));                     //is used to zero sin_zero[8]
    sa.sin_family = AF_INET;                        //address family IPv4,Ιδιο με την παραμετρο domain κατα την δημιουργια του socket
    sa.sin_port = htons(TCP_PORT);                  //convert to network byte order
    sa.sin_addr.s_addr = inet_addr("127.0.0.1");    //for localhost
    // δένει τον file descriptor που προσδιορίζεται από το όρισμα sockfd, με μία τη διεύθυνση που δίνεται στο όρισμα *addr
    if (bind(sd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        perror("bind");
        exit(1);
    }
    fprintf(stderr, "Bound TCP socket to port %d\n", TCP_PORT);

    /* Listen for incoming connections, καθιστά το socket κατάλληλο να δεχτεί συνδέσεις */
    if (listen(sd, TCP_BACKLOG) < 0) {
        perror("listen");
        exit(1);
    }

    /*Loop forever, accepting connections*/
    for (;;) {
        fprintf(stderr, "Waiting for an incoming connection...\n");

        /*Accept an incoming connection*/
        len = sizeof(struct sockaddr_in);
				//μπλοκάρει μέχρι να δεχθεί αίτημα για εισερχόμενη σύνδεση
				//αφαιρεί το πρώτο αίτημα εισερχόμενης σύνδεσης από την ουρά του sd, δημιουργεί ένα νέο fd
        if ((newsd = accept(sd, (struct sockaddr *)&sa, &len)) < 0) {
            perror("accept");
            exit(1);
        }
        if (!inet_ntop(AF_INET, &sa.sin_addr, addrstr, sizeof(addrstr))) {
            perror("could not format IP address");
            exit(1);
        }
        fprintf(stderr, "Incoming connection from %s:%d\n",
            addrstr, ntohs(sa.sin_port));
        if(argc==2){
            /*open crypto device*/
            cfd = open("/dev/crypto", O_RDWR);
            if (cfd < 0) {
                perror("open(/dev/crypto)");
                return 1;
            }

            /*Get crypto session for AES128*/
            sess.cipher = CRYPTO_AES_CBC;
            sess.keylen = KEY_SIZE;
            sess.key = key;
            //αρχικοποιούμε μια σύνοδο
			    	//Στη δομή session επιστρέφονται οι πληροφορίες της συνόδου με τη συσκευή
            if (ioctl(cfd, CIOCGSESSION, &sess)) {
                perror("ioctl(CIOCGSESSION)");
                return 1;
            }
        }

        /*We break out of the loop when the remote peer goes away*/
        for (;;) {
            fd_set fdset;
            int nfds;
            FD_ZERO(&fdset);                //αρχικοποιούμε στο 0 τα bit όλων των fd
            FD_SET(STDIN_FILENO, &fdset);   // select will check for input from terminal
            FD_SET(newsd, &fdset);        // select will check for input from socket descriptor (Client)
            //select only considers file descriptors that are smaller than nfds
            nfds = MAX(STDIN_FILENO,newsd) + 1; //the highest-numbered file descriptor in any of the three sets, plus 1
            int selectt;
            //επιστρέφει 0 αν γινει timeout, -1 αν συμβει λάθος και μεγαλύτερο του 0 αν υπαρξει δυνατοτητα εισοδου/εξοδου
            if ((selectt = select(nfds, &fdset, NULL, NULL, NULL)) <= 0) {
                perror("select\n");
                continue;                                       //just try again
            }

            if (FD_ISSET(STDIN_FILENO, &fdset)) { //User has typed something, we can read from stdin without blocking
                if(argc==2) memset(buf, '\0', sizeof(buf));
                n = read(0, buf, sizeof(buf));
                if (n < 0) {
                    perror("[server] read from stdin");
                    exit(1);
                }
								//υπάρχει ενέργεια στον αντίστοιχο client sd αλλά δεν υπάρχουν δεδομένα να διαβαστούν και επιστρεφονται 0 χαρακτηρες
                if (n == 0)  //EOF
                    break;

                if(argc==2){
                    if(encrypt(cfd)) perror("encrypt");
							  }

                if (insist_write(newsd, buf, sizeof(buf)) != sizeof(buf)) {
                    perror("[server] write to peer");
                    exit(1);
                }
            }
            else if (FD_ISSET(newsd, &fdset)) { //test if a file descriptor is still present in a set
                n = read(newsd, buf, sizeof(buf));
                if (n <= 0) {
                    if (n < 0)
                        perror("[server] read from peer");
                    else
                        fprintf(stderr, "[server] peer went away\n");
                    break;
                }

								if(argc==2){
                    if(decrypt(cfd)) perror("decrypt");
							  }

                if (insist_write(1, buf, n) != n) {
                    perror("[server] write to stdout");
                    break;
                }
            }
        }
        /*Make sure we don't leak open files*/
        // κλείνει το νέο socket descriptor που δημιούργησε η accept().
        if (close(newsd) < 0)
            perror("close");

	  	  if(argc==2){
      	    /*Finish crypto session*/
        	  if (ioctl(cfd, CIOCFSESSION, &sess.ses)) {
          	    perror("ioctl(CIOCFSESSION)");
          	    return 1;
    	      }

      	    /*close cryto device*/
      	    if (close(cfd) < 0) {
        	      perror("close(cfd)");
          	    return 1;
      	    }
  	  	}
	  }

    /*Unreachable*/
    return 1;
}
