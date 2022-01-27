#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>     //close
#include <sys/types.h>  //socket
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/select.h> //include the whole family
#include <sys/socket.h> //socket
#include <arpa/inet.h>
#include <netdb.h>      //gethostbyname
#include <time.h>
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
        struct crypt_op cryp;//περιέχει πεδία όπως source, destination, operation
        //πεδία που δέχονται την είσοδο και φιλοξενούν την έξοδο κάθε ενέργειας encrypt/decrypt
        struct {
                unsigned char   in[DATA_SIZE],        //τα μηνύματά μας μεταδίδονται σε “κβάντα” των 256 χαρκτήρων
                                encrypted[DATA_SIZE], //>256 στο πρώτο read θα διαβαστούν οι πρώτοι χαρακτήρες και στη συνέχεια, στην επόμενη επανάληψη οι υπόλοιποι
                                iv[BLOCK_SIZE];
        } data;

        memset(&cryp, 0, sizeof(cryp));

        /*Encrypt data.in to data.encrypted*/
        cryp.ses = sess.ses;
        cryp.len = sizeof(data.in);//fixed στο μέγεθος του data.in
        cryp.src = buf; //buffer με τα δεδομένα που θα κρυπτογραφηθούν
        cryp.dst = data.encrypted;//buffer με τα δεδομένα που θα κρυπτογραφηθούν
        cryp.iv = inv;//στη συνεχεια αρχικοποιείται σε μηδενικά
				              //iv είναι ένας initialized vector που χρησιμοποιείται για την κρυπτογράφηση
        cryp.op = COP_ENCRYPT; //κρυπτογράφηση και στη συνέχεια αντιγράφουμε στη δομή cryp τα δεδομένα προς επεξεργασία
				//για να χρησιμοποιησουμε την συσκευή /dev/crypto

        if (ioctl(cfd, CIOCCRYPT, &cryp)) {
                perror("ioctl(CIOCCRYPT)");
                return 1;
        }

        i = 0;
				//συμπληρώνουμε το μηνυμα με τον χαρακτήρα '\0' μέχρι να γεμίσει ο buffer
				//έτσι ώστε το bufsize να είναι σύμφωνα με τις προδιαγραφές του cryptodev
        memset(buf, '\0', sizeof(buf));
        while(data.encrypted[i] != '\0'){
                buf[i] = data.encrypted[i];
                i++;
        }

        return 0;
}

int decrypt(int cfd) {
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
        cryp.src = buf;//buffer με τα δεδομένα που θα αποκρυπτογραφηθούν
        cryp.dst = data.decrypted;
        cryp.iv = inv;
        cryp.op = COP_DECRYPT;
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
    int sd, cfd, port;
    char *hostname;
    struct hostent *hp;
    struct sockaddr_in sa;
    ssize_t n;
    struct pollfd pfds[2];

    memset(&sess, 0, sizeof(sess));

    if (argc != 3) {
        fprintf(stderr, "Usage: %s hostname port\n", argv[0]);
        exit(1);
    }
    hostname = argv[1];
    port = atoi(argv[2]);

    /*Create TCP/IP socket, used as the main chat channel*/
		//Ορίσματα: PF_INET+SOCK_STREAM+0=TCP
    if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {  //sd=fd για το νέο socket
																											 //protocol family IPv4, TCP
    						                                       //domain=PF_INET: refers to anything in the protocol, usually sockets/ports
                                                       //type=SOCK_STREAM: Provides sequenced, reliable, two-way, connection-based byte streams.
                                                       //protocol=0: the caller does not want to specify the protocol and will leave it up to the service provider
        perror("socket");
        exit(1);
    }
    fprintf(stderr, "Created TCP socket\n");

    /*Look up remote hostname on DNS*/
    if (!(hp = gethostbyname(hostname))) {  //if it doesn't return the hostent structure, error
        printf("DNS lookup failed for host %s\n", hostname);
        exit(1);
    }

    /*Connect to remote TCP port*/
	sa.sin_family = AF_INET;  //Ιδιο με την παραμετρο domain κατα την δημιουργια του socket
	sa.sin_port = htons(port);// Let the system choose, σε ποιο port θα ακουει
	memcpy(&sa.sin_addr.s_addr, hp->h_addr, sizeof(struct in_addr));//θέτουμε στην hp τη διεύθυνση στην οποία θέλουμε να δέχεται συνδέσεις το socket
	fprintf(stderr, "Connecting to remote host... "); // stderr = standard error -> your process writes diagnostic output to this file handle
  fflush(stderr); //forces a write of all user-space buffered data for the given output
	if (connect(sd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
		perror("connect");
		exit(1);
	}
	fprintf(stderr, "Connected.\n");

		if(argc==4){
				/*open crypto device*/
    		cfd = open("/dev/crypto", O_RDWR);
    		if (cfd < 0) {
      		  perror("open(/dev/crypto)");
      		  return 1;
  		  }

  		  /*Get crypto session for AES128*/
  		  sess.cipher = CRYPTO_AES_CBC; //module that makes it easy to perform AES128 encryption for files and directories
    		sess.keylen = KEY_SIZE;
  		  sess.key = key;
				//αρχικοποιούμε μια σύνοδο
				//Στη δομή session επιστρέφονται οι πληροφορίες της συνόδου με τη συσκευή
  		  if (ioctl(cfd, CIOCGSESSION, &sess)) {
      		  perror("ioctl(CIOCGSESSION)");
      		  return 1;
    		}
    }
    for (;;) {
			fd_set fdset;
			int nfds;
			FD_ZERO(&fdset);                // we must initialize before each call to select
			FD_SET(STDIN_FILENO, &fdset);   // select will check for input from terminal
			FD_SET(sd, &fdset);             // select will check for input from socket descriptor (Client)
			//select only considers file descriptors that are smaller than nfds
			nfds = MAX(STDIN_FILENO,sd) + 1; //the highest-numbered file descriptor in any of the three sets, plus 1
			// wait until any of the input file descriptors are ready to receive
			int selectt;
			//επιστρέφει 0 αν γινει timeout, -1 αν συμβει λάθος και μεγαλύτερο του 0 αν υπαρξει δυνατοτητα εισοδου/εξοδου
			if ((selectt = select(nfds, &fdset, NULL, NULL, NULL)) <= 0) {
					perror("select\n");
					continue;                                       //just try again
			}
			//Ελέγχουμε αν ο fd STDIN_FILENO είναι έτοιμος για r ή w
			//Ο client στέλνει μήνυμα
        if (FD_ISSET(STDIN_FILENO, &fdset)) { //FD_ISSET() tests if STDIN_FILENO is part of the set
					                                    //STDIN_FILENO: fd of standard input device (value 0)
																							//User has typed something, we can read from stdin without blocking
            if(argc==4) memset(buf, '\0', sizeof(buf));
            n = read(0, buf, sizeof(buf));    // n = number of bytes read - read() attempts to read up to count bytes from STDIN into the buffer starting at buffer
            if (n < 0) {
                perror("[client] read from client");
                exit(1);
            }
            if (n == 0)  //EOF
                break;

						if(argc==4){
                if(encrypt(cfd)) perror("encrypt");
            }

            if (insist_write(sd, buf, sizeof(buf)) != sizeof(buf)) {
                perror("[client] write to peer");
                exit(1);
            }
        }
				//Ελέγχουμε αν ο fd του socket είναι έτοιμος για r ή w
				//Ο client ελέγχει αν έλαβε κάποιο μήνυμα από τον server
        else if (FD_ISSET(sd, &fdset)) {  //FD_ISSET() tests if socket's fd is part of the set
            n = read(sd, buf, sizeof(buf)); // read() attempts to read up to count bytes from sd into the buffer starting at buffer
            if (n <= 0) {
                if (n < 0)
                    perror("[client] read from peer");
                else
                    fprintf(stderr, "[client] peer went away...exiting\n");
                break;
            }
            if(argc==4){
                if(decrypt(cfd)) perror("decrypt");
            }

            if (insist_write(1, buf, n) != n) {
                perror("[client] write to stdout");
                exit(1);
            }
        }
    }

    /*Make sure we don't leak open files*/
    if (close(sd) < 0)
        perror("close");

		if(argc==4){
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
    return 0;
}
