/*
 * crypto-chrdev.c
 *
 * Implementation of character devices
 * for virtio-cryptodev device
 *
 * Vangelis Koukis <vkoukis@cslab.ece.ntua.gr>
 * Dimitris Siakavaras <jimsiak@cslab.ece.ntua.gr>
 * Stefanos Gerangelos <sgerag@cslab.ece.ntua.gr>
 *
 */
#include <linux/cdev.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/virtio.h>
#include <linux/virtio_config.h>

#include "crypto.h"
#include "crypto-chrdev.h"
#include "debug.h"

#include "cryptodev.h"

/*
 * Global data
 */
struct cdev crypto_chrdev_cdev;

/**
 * Given the minor number of the inode return the crypto device
 * that owns that number.
 **/
static struct crypto_device *get_crypto_dev_by_minor(unsigned int minor){
	struct crypto_device *crdev;
	unsigned long flags;

	debug("Entering");

	spin_lock_irqsave(&crdrvdata.lock, flags);
	list_for_each_entry(crdev, &crdrvdata.devs, list) {
		if (crdev->minor == minor)
			goto out;
	}
	crdev = NULL;

out:
	spin_unlock_irqrestore(&crdrvdata.lock, flags);

	debug("Leaving");
	return crdev;
}

/*************************************
 * Implementation of file operations
 * for the Crypto character device
 *************************************/
//Τροποποιούμε την crypto_chrdev_open ώστε να πυροδοτείται η αντίστοιχη open της native κρυπτογραφικής συσκευής
//θέλουμε να μεταφέρει το μήνυμα του ανοίγματος μιας συσκευής στον hypervisor(backend), εκείνος να κάνει το open της πραγματικής συσκευής
//και να μεταφέρει τον αντίστοιχο file descriptor στον guest(frontend).
static int crypto_chrdev_open(struct inode *inode, struct file *filp) {
	int ret = 0;
	int err;
	unsigned int num_out, num_in, len;
	struct crypto_open_file *crof; //The crypto device this open file is associated with
	struct crypto_device *crdev;   //The virtio device we are associated with
	unsigned int *syscall_type;
	int *host_fd;
  //χρησιμοποιούνται για τη μεταφορά δεδομένων μεταξύ frontend και backend
  struct scatterlist syscall_type_sg, host_fd_sg, *sgs[2];
  unsigned long flags;

  num_out = 0;
  num_in = 0;

	debug("Entering");

	syscall_type = kzalloc(sizeof(*syscall_type), GFP_KERNEL); //allocate memory. The memory is set to zero. - GFP_KERNEL : the type of memory to allocate
	*syscall_type = VIRTIO_CRYPTODEV_SYSCALL_OPEN; //στέλνουμε την εντολή VIRTIO_CRYPTODEV_SYSCALL_OPEN και περιμένουμε το αποτέλεσμα από το backend

	host_fd = kzalloc(sizeof(*host_fd), GFP_KERNEL); //The fd that this device has on the Host
	*host_fd = -1;

	ret = -ENODEV;
	if ((ret = nonseekable_open(inode, filp)) < 0) goto fail;
	//δημιουργείται αυτόματα μία νέα δομή ανοιχτού αρχείου, η οποία χαρακτηρίζεται ως μη ανιχνεύσιμη
	//αυτό σημαίνει ότι ο χρήστης δε μπορεί να αλλάξει τη θέση του fd του ανοιχτού αρχείου (με system calls)

	/* Associate this open file with the relevant crypto device. */
	// μεσω αυτης εχουμε προσβαση στο ανοιχτο αρχειο
	crdev = get_crypto_dev_by_minor(iminor(inode)); //Επιστρέφει την crypto device που έχει το minor number του συγκεκριμένου inode
	                                                //Με αυτό τον τρόπο αποκτάμε πρόσβαση στο ανοιχτό αρχείο
	if (!crdev) { //αν η get_crypto_dev_by_minor επιστρέψει 0, δεν βρέθηκε συσκευή, άρα βγάινουμε από την open
		debug("Could not find crypto device with %u minor",
		      iminor(inode));
		ret = -ENODEV;
		goto fail;
	}
  //Αν βρεθεί η συσκευή, δεσμεύουμε χώρο στη μνήμη για το ανοιχτό αρχέιο
	crof = kzalloc(sizeof(*crof), GFP_KERNEL); //ζητάμε την απαραίτητη μνήμη από το λειτουργικό για το crypto device open file, κάνουμε allocate χώρο
	if (!crof) {
		ret = -ENOMEM;
		goto fail;
	}
	crof->crdev = crdev; //θέτουμε στο ανοιχτό αρχέιο το minor number της crypto device
	crof->host_fd = -1;
	filp->private_data = crof; //δηµιουργεί µια δοµή file η οποία αναπαριστά ενα ανοιχτό αρχείο και µέσω αυτής έχει πρόσβαση στο struct crypto_open_file
	                           //filp: ο file descriptor της native συσκευής τον οποίο αποθηκεύουμε για future reference στο πεδίο private_data του ανοικτού αρχείου.

	/**
	 * We need two sg lists, one for syscall_type and one to get the
	 * file descriptor from the host.
	 **/
	 //δημιουργούμε 2 scatter-gather λίστες ,μία για να διαβάσει ο host τον τύπο του system call που
   //θέλει να κάνει ο guest εδώ open και μία για να επιστρέψει ο host (στον guest) τον file descriptor που θα
   //επιστραφεί από το open() που θα πραγματοποιήσει ο ίδιος
    sg_init_one(&syscall_type_sg, syscall_type, sizeof(syscall_type)); //Initialize a single entry sg list
    sgs[num_out++] = &syscall_type_sg;
    sg_init_one(&host_fd_sg, &crof->host_fd, sizeof(host_fd));
    sgs[num_out + num_in++] = &host_fd_sg;

	/**
	 * Wait for the host to process our data.
	 **/
    //κλειδώνουμε το spinlock της αντίστοιχης συσκευης, πριν το κλειδώσει απενεργοποιεί τα interrupts στη συγκεκριμένη CPU
    spin_lock_irqsave(&crdev->lock, flags);
		//προσθέτουμε στην ουρά virtqueue όλες τις λίστες που έχουμε ορίσει
		//το struct virtqueue *vq; ορίζεται στο crypto.h
    err = virtqueue_add_sgs(crdev->vq, sgs, num_out, num_in, &syscall_type_sg, GFP_ATOMIC);
    //ενημερώσει το backend ότι υπάρχουν καινούργια δεδομένα
		virtqueue_kick(crdev->vq);
    // ο guest κάνει busy wait με συνεχείς κλήσεις της
    //Οσο αυτό είναι NULL αναμένουμε
    while(virtqueue_get_buf(crdev->vq, &len) == NULL); //Do nothing
    //οταν σταματήσει να επιστρέφει NULL, ο host έχει επιστρέψει τα επεξεργασμένα δεδομένα πίσω στον guest πάλι μέσω της VirtQueue
		//ξεκλειδώνεται το spinlock της συσκευής
    spin_unlock_irqrestore(&crdev->lock, flags); //αν δεν υπάρχει νέο μηνυμα

	// ο file descriptor στον guest έχει τώρα τιμή που αφορά το ανοιχτό αρχείο της πραγματικής συσκευής στον host
	/* If host failed to open() return -ENODEV. */
	if (crof->host_fd < 0) {
        debug("Host failed to open the crypto device");
        ret = -ENODEV;
    }
    debug("Host opened /dev/crypto file with fd = %d", crof->host_fd);
fail:
	debug("Leaving");
	return ret;
}
//Καλείται κάθε φορά που μία διεργασία κλείνει ένα ανοιχτό αρχείο της εικονικής συσκευής χαρακτήρων virtio-cryptodev
static int crypto_chrdev_release(struct inode *inode, struct file *filp){
	int ret = 0;
	//ανακτά την εικονική συσκευή με την οποία συνδέεται το ανοιχτό αρχείο και τον πραγματικό file descriptor που έχει στο host μηχάνημα
	struct crypto_open_file *crof = filp->private_data;
	struct crypto_device *crdev = crof->crdev;
	unsigned int *syscall_type;

    struct scatterlist syscall_type_sg, host_fd_to_close_sg, *sgs[2];
    unsigned int len, num_out, num_in;
    unsigned long flags;
    int err;

    num_out = 0;
    num_in = 0;

	debug("Entering");
  //Στέλνουμε δεδομένα στον host
	syscall_type = kzalloc(sizeof(*syscall_type), GFP_KERNEL);
	*syscall_type = VIRTIO_CRYPTODEV_SYSCALL_CLOSE; //στέλνουμε τον fd που επιθυμούμε να κλείσουμε και απελευθερώνουμε όποια δομή δεν απαιτείται πλέον

	/**
	 * Send data to the host.
	 **/
     //now I send 2 scatterlists one for the syscall type and one for the fd
    sg_init_one(&syscall_type_sg, syscall_type, sizeof(syscall_type));
    sgs[num_out++] = &syscall_type_sg;
    sg_init_one(&host_fd_to_close_sg, &crof->host_fd, sizeof(crof->host_fd));
    sgs[num_out++] = &host_fd_to_close_sg;

		//Κλειδώνουμε το spinlock της συσκευής
    spin_lock_irqsave(&crdev->lock, flags);
    err = virtqueue_add_sgs(crdev->vq, sgs, num_out, num_in, &syscall_type_sg, GFP_ATOMIC);
    virtqueue_kick(crdev->vq);
	/**
	 * Wait for the host to process our data.
	 **/
    while(virtqueue_get_buf(crdev->vq, &len) == NULL) ; //Do nothing
    spin_unlock_irqrestore(&crdev->lock, flags);

    debug("Host closed /dev/crypto with fd = %d\n", crof->host_fd);
    /*
     * Check for close() failure
     * Just add a host_ret pointer just like in ioctl methods below
     */

	kfree(crof); //Σε περίπτωση επιτυχίας, ελευθερώνει το χώρο που είχε δεσμέυσει από τη δομή crypto_open_file
	debug("Leaving");
	return ret;

}

//cmd προσδιορίζει το είδος της κλήσης
//Tο όρισμα arg περιέχει τη διεύθυνση ενός αντικειμένου τύπου session_op, το οποίο χρησιμοποιείται για την έναρξη του session
//arg, είναι η διεύθυνση (στο χώρο χρήστη του guest)
static long crypto_chrdev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
  int i;
	long ret = 0;
	int err;
	struct crypto_open_file *crof = filp->private_data;
	struct crypto_device *crdev = crof->crdev;
	struct virtqueue *vq = crdev->vq;
	struct scatterlist syscall_type_sg, ioctl_type_sg, output_msg_sg, input_msg_sg,\
                       sess_sg, host_fd_sg, key_sg, host_ret_sg, ses_sg, cryp_sg,\
                       src_sg, dst_sg, iv_sg, op_sg,\
	                   *sgs[14];
	unsigned int num_out, num_in, len;
  #define MSG_LEN 100
	unsigned char *output_msg, *input_msg, *key=NULL;
	unsigned int *syscall_type, *ioctl_type=NULL;
    long *host_ret = kmalloc(sizeof(long), GFP_KERNEL);
    struct session_op *sess=NULL;
    uint32_t *ses=NULL;
    struct crypt_op *cryp=NULL;
    unsigned char *src=NULL, *dst=NULL, *iv=NULL;
    char bytes;
    unsigned long flags;
    num_out = 0;
	  num_in = 0;
    *host_ret = -1; //by default lets have an error

    debug("Entering");
	/**
	 * Allocate all data that will be sent to the host.
	 **/
	output_msg = kzalloc(MSG_LEN, GFP_KERNEL);
	input_msg = kzalloc(MSG_LEN, GFP_KERNEL);
  syscall_type = kzalloc(sizeof(*syscall_type), GFP_KERNEL);
	*syscall_type = VIRTIO_CRYPTODEV_SYSCALL_IOCTL; //στέλνουμε αυτη την εντολή μέσω της syscall_type scatter gather list

	/**
	 *  These are common to all ioctl commands.
	 **/
	sg_init_one(&syscall_type_sg, syscall_type, sizeof(*syscall_type));
	sgs[num_out++] = &syscall_type_sg;
  sg_init_one(&host_fd_sg, &crof->host_fd, sizeof(crof->host_fd));
  sgs[num_out++] = &host_fd_sg;

	switch (cmd) { //επιλογή command
		// Εδώ αντιγράφουμε από τη διεργασία του χρήστη το session, το βάζουμε στη vitrqueue και το στέλνουμε στο backend, όπου θα γίνει το “πραγματικό” άνοιγμα του session.
	case CIOCGSESSION: //get session
		debug("CIOCGSESSION");
        //στέλνουμε την εντολή VIRTIO_CRYPTODEV_IOCTL_CIOCGSESSION και περιμένουμε το αποτέλεσμα από το backend
        ioctl_type = kzalloc(sizeof(*ioctl_type), GFP_KERNEL);
        *ioctl_type = VIRTIO_CRYPTODEV_IOCTL_CIOCGSESSION;

        //Η διεύθυνση βρίσκεται σε χώρο χρήστη => copy session_op struct from userspace
        //now sess is in kernel space but points to userspace
        sess = kzalloc(sizeof(*sess), GFP_KERNEL);
				//copy_to_user(): κάθε απάντηση που λαμβάνουμε πρέπει να τη μεταφέρουμε πάλι στο user space του guest
        if (copy_from_user(sess, (struct session_op *)arg, sizeof(*sess))) { //αντιγράφονται οι διευθύνσεις των pointers
            debug("Copy session_op from user failed");
            ret = -EFAULT;
            goto out;
        }

				// ο host χρειάζεται να διαβάσει το (ήδη αρχικοποιημένο) session key
        //copy key from userspace
        //sess has only a pointer to the string key
        key = kzalloc(sess->keylen, GFP_KERNEL);
        if (copy_from_user(key, sess->key, sess->keylen)) { //αντιγράφονται οι διευθύνσεις των pointers
            debug("Copy key from user failed");
            ret = -EFAULT;
            goto out;
        }

        //περναμε το κλειδί ξεχωριστά με μία scatter-gather λίστα ανάγνωσης
        sg_init_one(&ioctl_type_sg, ioctl_type, sizeof(*ioctl_type));
        sgs[num_out++] = &ioctl_type_sg;
        sg_init_one(&key_sg, key, sess->keylen);
        sgs[num_out++] = &key_sg;

        //το υπόλοιπο αντικείμενο δίνεται στον host μέσω άλλης λίστας για εγγραφή και ανάγνωση
        sg_init_one(&sess_sg, sess, sizeof(*sess)); // ποιο session επιθυμούμε να ανοιξουμε
        sgs[num_out + num_in++] = &sess_sg;
        sg_init_one(&host_ret_sg, host_ret, sizeof(*host_ret));
        sgs[num_out + num_in++] = &host_ret_sg;
        break;

  //Σε αυτή την περίπτωση η διεργασία θέλει να τερματίσει ένα session με τη συσκευή κρυπτογράφησης
	//Το όρισμα arg περιέχει τη διεύθυνση του identifier του
	//Στέλνουμε στο backend, μαζί με τα υπόλοιπα, και το session id έτσι ώστε αυτό να κλείσει το ζητούμενο session
	case CIOCFSESSION:
		debug("CIOCFSESSION"); //close session

        //define ioctl command
        ioctl_type = kzalloc(sizeof(*ioctl_type), GFP_KERNEL);
        *ioctl_type = VIRTIO_CRYPTODEV_IOCTL_CIOCFSESSION;

        ses = kmalloc(sizeof(uint32_t), GFP_KERNEL);
				//Αντιγράφουμε την τιμή του arg στη μεταβλητή που δημιουργήσαμε στο χώρο πυρήνα, ses
        if (copy_from_user(ses, (uint32_t *)arg, sizeof(*ses))) {
            debug("Copy from user failed");
            ret = -EFAULT;
            goto out;
        }
        //Αποθηκεύουμε τον τύπο της ioctl command σε μία scatter-gather λίστα
        sg_init_one(&ioctl_type_sg, ioctl_type, sizeof(*ioctl_type));
        sgs[num_out++] = &ioctl_type_sg;
        sg_init_one(&ses_sg, ses, sizeof(*ses));
        sgs[num_out++] = &ses_sg;

        sg_init_one(&host_ret_sg, host_ret, sizeof(*host_ret));
        sgs[num_out + num_in++] = &host_ret_sg;
		break;

	//Σε αυτή την περίπτωση η διεργασία θέλει να κάνει κρυπτογράφηση/αποκρυπτογράφηση με τη συσκευή
	case CIOCCRYPT: //encrypt / decrypt
		debug("CIOCCRYPT");

        ioctl_type = kzalloc(sizeof(*ioctl_type), GFP_KERNEL);
        *ioctl_type = VIRTIO_CRYPTODEV_IOCTL_CIOCCRYPT;

        cryp = kmalloc(sizeof(*cryp), GFP_KERNEL);
				//Για να λάβουμε με ασφάλεια το struct crypt_op
				//αντιγράφουμε την τιμή των πεδίων του, στα πεδία του νέου αντικειμένου cryp ίδιου τύπου που δημιουργούμε στο χώρο πυρήνα
        if (copy_from_user(cryp, (struct crypt_op *)arg, sizeof(*cryp))) {
            debug("Copy crypt_op from user failed");
            ret = -EFAULT;
            goto out;
        }
        //src: πεδίο για την αποθήευση του αρχικού μηνύματος
        src = kzalloc(cryp->len, GFP_KERNEL);
				//αντιγράφουμε την τιμή των πεδίων src,iv στα πεδία του νέου αντικειμένου cryp ίδιου τύπου που δημιουργούμε στο χώρο πυρήνα
        if (copy_from_user(src, cryp->src, cryp->len)) {
            debug("Copy src from user failed");
            ret = -EFAULT;
            goto out;
        }
       //iv: initialization vector
        iv = kzalloc(EALG_MAX_BLOCK_LEN, GFP_KERNEL);
        if (copy_from_user(iv, cryp->iv, EALG_MAX_BLOCK_LEN)) {
            debug("Copy iv from user failed");
            ret = -EFAULT;
            goto out;
        }
        //πεδίο για την αποθήκευση του κρυπτογραφημένου/αποκρυπτογραφημένου μηνύματος
        dst = kzalloc(cryp->len, GFP_KERNEL);

        //send with R flag
				//προκειμένου να επιστραφεί στο guest το επεξεργασμένο μήνυμα πρέπει ο host να έχει δικαίωμα εγγραφής στη νέα μεταβλητή που δημιουργήσαμε ως αντίστοιχη της dst σε χώρο πυρήνα
				//Γι'αυτό, δημιουργούμε 4 scatter-gather λίστες
        sg_init_one(&ioctl_type_sg, ioctl_type, sizeof(*ioctl_type));
        sgs[num_out++] = &ioctl_type_sg;
				//scatter-gather λίστα ανάγνωσης των υπόλοιπων πεδίων του αντικειμένου cryp
        sg_init_one(&cryp_sg, cryp, sizeof(*cryp));
        sgs[num_out++] = &cryp_sg;
				//scatter-gather λίστα ανάγνωσης των υπόλοιπων πεδίων των αντικειμένων src, iv
        sg_init_one(&src_sg, src, cryp->len);
        sgs[num_out++] = &src_sg;
        sg_init_one(&iv_sg, iv, EALG_MAX_BLOCK_LEN);
        sgs[num_out++] = &iv_sg;

				//scatter-gather λίστες ανάγνωσης-εγγραφης
        sg_init_one(&host_ret_sg, host_ret, sizeof(*host_ret));
        sgs[num_out + num_in++] = &host_ret_sg;
        sg_init_one(&dst_sg, dst, cryp->len);
        sgs[num_out + num_in++] = &dst_sg;
        break;

    default:
		debug("Unsupported ioctl command");
		break;
	}

	/**
	 * Wait for the host to process our data.
	 **/
	 //Μόλις γίνει κάποιο από τα παραπάνω, κλειδώνουμε το spinlock της συσκευής χαρακτήρων
	spin_lock_irqsave(&crdev->lock, flags);
	//προσθέτουμε τις λίστες στην VirtQueue της συσκευής χαρακτήρων
	err = virtqueue_add_sgs(vq, sgs, num_out, num_in, &syscall_type_sg, GFP_ATOMIC);
  //ενημερώνουμε τον host για προσθήκη νέου στοιχείου στην ουρά
	virtqueue_kick(vq);
	//περιμένουμε μέχρι ο host να επιστρέψει τον buffer με τα επεξεργασμένα δεδομένα στον χώρο πυρήνα του guest
	while (virtqueue_get_buf(vq, &len) == NULL) ;
    spin_unlock_irqrestore(&crdev->lock, flags);

//ανάλογα με το είδος ioctl() που κάναμε κάνουμε χρήση της copy_to_user() ώστε η τιμή των επεξεργασμένων από τον host μεταβλητών,
//που δημιουργήθηκαν και υπάρχουν σε χώρο πυρήνα, να αντιγραφεί στα αντίστοιχα πεδία του ορίσματος arg
    switch (cmd) {
    case CIOCGSESSION:
        //restore the pointer sess->key to original userspace address
        sess->key = ((struct session_op *) arg)->key;
        //and return the session to the userspace
        if (copy_to_user((struct session_op *) arg, sess, sizeof(*sess))) {
            debug("Copy to user failed!");
            ret = -EFAULT;
            goto out;
        }
        break;

    case CIOCFSESSION:
        if (copy_to_user((uint32_t *)arg, ses, sizeof(*ses))) {
            debug("Copy to user failed!");
            ret = -EFAULT;
            goto out;
        }
        break;

    case CIOCCRYPT:
        //return dst data (encrypted/decrypted back to userspace)
        if (copy_to_user(((struct crypt_op *)arg)->dst, dst, cryp->len)) {
            debug("Copy to user failed!");
            ret = -EFAULT;
            goto out;
        }
        break;

    default:
		debug("Unsupported ioctl command (2nd)");
        break;
    }

    ret = *host_ret;
out:
    debug("Leaving ioctl with ret value %ld", ret);

	  //απελευθερώνουμε τον χώρο που δεσμεύσαμε στον χώρο πυρήνα και επιστρέφουμε σε χώρο χρήστη
    kfree(cryp);
    kfree(dst);
    kfree(src);
    kfree(iv);
    kfree(ioctl_type);
    kfree(ses);
    kfree(sess);
    kfree(key);
    kfree(output_msg);
    kfree(input_msg);
    kfree(syscall_type);
    kfree(host_ret);

	return ret;
}

static ssize_t crypto_chrdev_read(struct file *filp, char __user *usrbuf, size_t cnt, loff_t *f_pos) {
	debug("Entering");
	debug("Leaving");
	return -EINVAL;
}

static struct file_operations crypto_chrdev_fops = {
	.owner          = THIS_MODULE,
	.open           = crypto_chrdev_open,
	.release        = crypto_chrdev_release,
	.read           = crypto_chrdev_read,
	.unlocked_ioctl = crypto_chrdev_ioctl,
};

int crypto_chrdev_init(void) {
	int ret;
	dev_t dev_no;
	unsigned int crypto_minor_cnt = CRYPTO_NR_DEVICES;

	debug("Initializing character device...");
	cdev_init(&crypto_chrdev_cdev, &crypto_chrdev_fops);
	crypto_chrdev_cdev.owner = THIS_MODULE;

	dev_no = MKDEV(CRYPTO_CHRDEV_MAJOR, 0);
	ret = register_chrdev_region(dev_no, crypto_minor_cnt, "crypto_devs");
	if (ret < 0) {
		debug("failed to register region, ret = %d", ret);
		goto out;
	}
	ret = cdev_add(&crypto_chrdev_cdev, dev_no, crypto_minor_cnt);
	if (ret < 0) {
		debug("failed to add character device");
		goto out_with_chrdev_region;
	}

	debug("Completed successfully");
	return 0;

out_with_chrdev_region:
	unregister_chrdev_region(dev_no, crypto_minor_cnt);
out:
	return ret;
}

void crypto_chrdev_destroy(void) {
	dev_t dev_no;
	unsigned int crypto_minor_cnt = CRYPTO_NR_DEVICES;

	debug("entering");
	dev_no = MKDEV(CRYPTO_CHRDEV_MAJOR, 0);
	cdev_del(&crypto_chrdev_cdev);
	unregister_chrdev_region(dev_no, crypto_minor_cnt);
	debug("leaving");
}
