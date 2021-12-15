/*
 * lunix-chrdev.c
 *
 * Implementation of character devices
 * for Lunix:TNG
 *
 * Noni Strati,
 * Margarita Tsarmpopoulou
 *
 */

#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/cdev.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mmzone.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>

#include "lunix.h"
#include "lunix-chrdev.h"
#include "lunix-lookup.h"


//Global data

struct cdev lunix_chrdev_cdev;

/* Just a quick [unlocked] check to see if the cached
 * chrdev state needs to be updated from sensor measurements.
 */

static int lunix_chrdev_state_needs_refresh(struct lunix_chrdev_state_struct *state){
	struct lunix_sensor_struct *sensor;
	WARN_ON ( !(sensor = state->sensor)); //???
  debug("looking if it needs refresh");
	//Συγκρίνουμε την τελευταία φορά που πήραμε δεδομένα από τους sensor buffers και την τελευταία φορά που ήρθαν δεδομένα σε αυτόυς και συμπεραίνουμε αν ο δικός μας buffer χρειάζεται refresh
  if (sensor->msr_data[state->type]->last_update != state->buf_timestamp) {
        debug("State needs refreshing!\n");
        return 1; //now state needs refreshing
    }
    return 0;
}

/* Updates the cached state of a character device based on sensor data. Must be
 * called with the character device state lock held.
 */

static int lunix_chrdev_state_update(struct lunix_chrdev_state_struct *state){
	  struct lunix_sensor_struct *sensor;
    long int result;            //lookup tables give long int
		int ret;

	  debug("leaving\n");
    WARN_ON ( !(sensor = state->sensor)); //there is no data available right now, try again later

    /*
	   * Grab the raw data quickly, hold the spinlock for as little as possible.
	   */

		 spin_lock(&sensor->lock); //κλειδώνουμε το spinlock του αντίστοιχου αισθητήρα, πριν το κλειδώσει απενεργοποιεί τα interrupts στη συγκεκριμένη CPU
     uint16_t value = sensor->msr_data[state->type]->values[0]; //ανανέωση τιμών, lookup tables require uint16_t
     uint32_t current_timestamp = sensor->msr_data[state->type]->last_update; //ανανέωση timestamp, defined in lunix_msr_data_struct

		 spin_unlock(&sensor->lock); //αν δεν υπάρχει νέα μέτρηση
	/* Why use spinlocks? See LDD3, p. 119 */

	/*
	 * Any new data available?
     *
 	 * Now we can take our time to format them, holding only the private state
 	 * semaphore. This is implemented in open syscall.
 	 */
    if(lunix_chrdev_state_needs_refresh(state)==1){ //ελέγχουμε αν υπάρχει νέα μέτρηση
        if (!state->raw_data) {   //cooked data
				/*Ελέγχουμε τον τύπο των δεδομένων και τοποθετούμε την τιμή στο κατάλληλο table, λαμβανουμε μία προσημασμένη δεκαδική τιμή*/
            if (state->type == BATT)  result = lookup_voltage[value];
            else if (state->type == TEMP) result = lookup_temperature[value];
            else if (state->type == LIGHT) result = lookup_light[value];
            else {
                debug("Type doesnt match one the three available \
                                                            (BATT, TEMP, LIGHT): internal error");
                ret = -EMEDIUMTYPE;    //wrong medium type
                goto out;
            }

						//μορφοποιούμε τα δεδομένα κάνοντας χρήση των ειδικών πινάκων, ανανεώνουμε το timestamp και μεταφέρουμε τα δεδομένα μας στον buffer.
            ret = 0;
            state->buf_timestamp = current_timestamp;
            state->buf_lim = snprintf(state->buf_data, LUNIX_CHRDEV_BUFSZ, \
                                    "%ld.%03ld\n", result/1000, result%1000);
					 //αντιγράφει την μέτρηση στην τελική αναγνώσιμη μορφή της στο ανοιχτό μας αρχείο
					 //επιστρεφει τον αριθμό των byte που έγραψε στα buf_data, δηλαδή το μέγεθος της μέτρησης
        }
        else{   //raw data
            debug("returning raw bytes (skipped lookup table conversion)\n");
            ret = 0;
            state->buf_timestamp = current_timestamp; //θέτουμε timestamp του buffer το νέο timestamp
            state->buf_lim = snprintf(state->buf_data, LUNIX_CHRDEV_BUFSZ, \
                                    "%x\n", value); //prints raw_data as hex
        }
    }
    else ret = -EAGAIN; //δεν υπάρχουν νέα διαθέσιμα δεδομένα τώρα, προσπάθησε αργότερα

out:
	debug("leaving\n");
	return ret;
}

/*************************************
 * Implementation of file operations
 * for the Lunix character device
 *************************************/

static int lunix_chrdev_open(struct inode *inode, struct file *filp){
	/* Declarations */
    unsigned int minor, sensor, type;
    int ret;
    struct lunix_chrdev_state_struct *state; //θα κάνει reference τον συγκεκριμένο char driver

	  debug("entering\n");
	  ret = -ENODEV;
	  if ((ret = nonseekable_open(inode, filp)) < 0) goto out;
	  //δημιουργείται αυτόματα μία νέα δομή ανοιχτού αρχείου, η οποία χαρακτηρίζεται ως μη ανιχνεύσιμη
    //αυτό σημαίνει ότι ο χρήστης δε μπορεί να αλλάξει τη θέση του fd του ανοιχτού αρχείου (lseek(),pread(), pwrite() system calls)

 	 /*
 	  * Associate this open file with the relevant sensor based on
 	  * the minor number of the device node [/dev/sensor<NO>-<TYPE>]
	  */
    minor = MINOR(inode->i_rdev);   //μας υποδεικνυει τον αριθμό του αισθήτηρα (minor number) και τον τύπο των μετρήσεων, βρίσκουμε τον minor number του ειδικού αρχείου που ανοίξαμε
    sensor = minor/8;               //minor=8*sensor+μετρηση από εκφώνηση (0-15)
    type = minor%8;                 //0-2
    debug("Done assosiating file with sensor %d of type %d\n", sensor, type);

	/* Allocate a new Lunix character device private state structure */

    //GFP_KERNEL	This is a normal allocation and might block.
    //This is the flag to use in process context code when it is safe to sleep
    state = kmalloc(sizeof(struct lunix_chrdev_state_struct), GFP_KERNEL); //ζητάμε την απαραίτητη μνήμη από το λειτουργικό, κάνουμε allocate χώρο
    state->type = type;
    state->sensor = &lunix_sensors[sensor];

    /*buffer init*/
		                            //Aρχικοποιούμε κατάλληλα τα υπόλοιπα πεδία για να αποθηκεύσουμε το state και να το χρησιμοποιήσουμε στις υπόλοιπες συναρτήσεις
    state->buf_lim = 0;         //μέγεθος
    state->buf_timestamp = 0;   //πόσο πρόσφατη είναι η μέτρηση που περιέχει το ανοιχτό αρχείο σε κάθε στιγμή
		//Aποθηκεύουμε την δομή state στα private_data της δομής file του ανοιχτού αρχείου ώστε να έχουμε εύκολη πρόσβαση σε αυτήν σε κάθε λειτουργία πάνω στο ανοιχτό αρχείο
    filp->private_data = state; //δηµιουργεί µια δοµή file η οποία αναπαριστά ενα ανοιχτό αρχείο και µέσω αυτής έχει πρόσβαση στο struct file_operations lunix_chrdev_fops
    state->raw_data = 0;        //by default, in coocked data mode

    sema_init(&state->lock,1);  //initialize semaphore with 1 (unlocked), refers to a single struct file
    ret = 0;
out:
	debug("leaving, with ret = %d\n", ret);
	return ret;
}

static int lunix_chrdev_release(struct inode *inode, struct file *filp){
    debug("Releasing allocated memory for private file data\n");
    kfree(filp->private_data); //απελευθερώνουμε τον χώρο στη μνήμη που είχαμε δεσμεύσει για τα ειδικά δεδομένα
    debug("Done releasing private file data => exiting");
	return 0;
}

static long lunix_chrdev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
	    /* Why? */
    return -EINVAL;
}

static ssize_t lunix_chrdev_read(struct file *filp, char __user *usrbuf, size_t cnt, loff_t *f_pos){
	ssize_t ret, remaining_bytes;
	struct lunix_sensor_struct *sensor;
	struct lunix_chrdev_state_struct *state;

	state = filp->private_data;
	WARN_ON(!state);
	sensor = state->sensor;
	WARN_ON(!sensor);

	debug("entering!\n");

	/*
     * Lock, in case processes with the same fd (struct file) try to access the read
     * eg possible problem would be that a child reads the file and changes the f_pos
     * Then, say, the father reads the file also. He gets wrong values.
     */

    if (down_interruptible(&state->lock)) //κλειδώνουμε το σεμαφόρο του αντίστοιχου ανοιχτού αρχείου και όλες οι άλλες διεργασίες που επιθυμούν να μπουν, μπλοκάρονται (αν πχ είχαμε κάνει προηγουμένως fork)
        return -ERESTARTSYS; //Lets the VFS know (internally) that a signal arrived. ie "ctrl+C"
                             //Now VFS restarts it or sends the user a -EINTR value.
    /*
	 * If the cached character device state needs to be
	 * updated by actual sensor data (i.e. we need to report
	 * on a "fresh" measurement, do so
	 */

	if (*f_pos == 0) { //δεν υπάρχουν διαθέσιμα δεδομένα στο ανοιχτό αρχείο, η διεργασία πρέπει να περιμένει την λήψη νέας μέτρησης
    //επαναλαμβάνεται για κάθε διεργασία
		while (lunix_chrdev_state_update(state) == -EAGAIN) { //η cache είναι άδεια και τα νέα δεδομένα δεν είναι ακόμα διαθέσιμα
			/* The process needs to sleep */
			/* See LDD3, page 153 for a hint */
			      debug("exiting state update: going to sleep");
            up(&state->lock); //ξεκλειδώνουμε τον σεμαφόρο του ανοιχτού αρχείου στην περίπτωση κάποιου interrupt

            if (filp->f_flags & O_NONBLOCK) return -EAGAIN; //if the file was opened with O_NONBLOCK flag return -EAGAIN
            //sleep, if condition is FALSE, if you re woken up check condition again and sleep or leave
            //κοιμίζουμε την διεργασία τοποθετώντας την στην ουρά αναμονής του αισθητήρα από τον οποίο περιμένει μέτρηση
            if (wait_event_interruptible(sensor->wq, lunix_chrdev_state_needs_refresh(state))) return -ERESTARTSYS;
                //wait_event_interruptible returns nonzero when interrupted by signal

            //κλειδωνεται ο σημαφόρος οταν ξυπνησει
            if (down_interruptible(&state->lock)) return -ERESTARTSYS;
		}
	}
    debug("fresh values available\n");

    /* Determine the number of cached bytes to copy to userspace */
    remaining_bytes = state->buf_lim - *f_pos;

    if (cnt >= remaining_bytes) cnt = remaining_bytes; //η τρέχουσα θέση υπερβαίνει το μέγεθος της μέτρησης, του επιστρέφουμε όσα byte απομένουν μέχρι το τέλος της μέτρησης

    /*
     * copy_to_user(to, from, count): Τα δεδομένα μεταφέρονται με ασφάλεια στον χώρο χρήστη
		 * ο πυρήνας έχει πρόσβαση σε όλη τη μνήμη και χωρίς αυτήν μία αναφορά σε invalid διεύθυνση δεν θα γινόταν αντιληπτή
     * returns the number of bytes that couldnt copy
     */

    if (copy_to_user(usrbuf, state->buf_data + *f_pos, cnt)) {
        ret = -EFAULT;
        goto out;
    }

    //fix the position
    *f_pos = *f_pos + cnt; //αυξάνουμε την τιμή του f_pos κατά τον αριθμό των byte που μπορέσαμε να δώσουμε στον χρήστη
    ret = cnt;

	/* Auto-rewind on EOF mode
   * ΄Οταν διαβαστούν όλα τα bytes το *f_pos μηδενίζεται
	 */
	if (*f_pos == state->buf_lim) *f_pos = 0; //η τρέχουσα θέση ειναι ιση το μέγεθος της μέτρησης, φτάσαμε στο τέλος της μέτρησης

out:
	/* Unlock */
    up(&state->lock); //ξεκλειδώνουμε τον σημαφόρο ώστε να επιστρέψουμε την πρόσβαση σε αυτό από άλλες διεργασίες
	  return ret;       //επιστρέφουμε στον χρήστη τον αριθμό των byte που μπορέσαμε να του δώσουμε
}

static int lunix_chrdev_mmap(struct file *filp, struct vm_area_struct *vma){
    struct lunix_chrdev_state_struct *state;
    struct lunix_sensor_struct *sensor;
    struct page *kernel_page;
    unsigned long long *kernel_page_address;

    state = filp->private_data;
		WARN_ON(!state);
    sensor = state->sensor;
		WARN_ON(!sensor);

		debug("Entering...\n");

    //παιρνουμε εναν δεικτη στην virtual address του struct page
    kernel_page = virt_to_page(sensor->msr_data[state->type]->values);
    kernel_page_address = page_address(kernel_page);
		debug("Buffer address: %llu Buffer page address%llu\n", sensor->msr_data[state->type]->values ,kernel_page_address );

    //παιρνουμε την physical address της virtual που πηραμε απο πανω
    vma->vm_pgoff = __pa(kernel_page_address) >> PAGE_SHIFT;

    debug("Exiting...\n");
	  return 0;
}

//Ορίζει ποιες συναρτήσεις θα καλεστούν για τις λειτουργίες του driver
static struct file_operations lunix_chrdev_fops ={
    .owner          = THIS_MODULE,
	.open           = lunix_chrdev_open,
	.release        = lunix_chrdev_release,
	.read           = lunix_chrdev_read,
	.unlocked_ioctl = lunix_chrdev_ioctl,
	.mmap           = lunix_chrdev_mmap
};

//this function is called from lunix-module.c
int lunix_chrdev_init(void){ // running when insmod
	/*
	 * Register the character device with the kernel, asking for
	 * a range of minor numbers (number of sensors * 8 measurements / sensor)
	 * beginning with LINUX_CHRDEV_MAJOR:0
	 */
	int ret;
	dev_t dev_no;
	unsigned int lunix_minor_cnt = lunix_sensor_cnt << 3; //16*8 = 128

	debug("initializing character device\n");

  /* Αρχικοποιεί την δομή cdev (την συσκευή χαρακτήρων) και αναθέτει τα file operations στην lunix_chrdev_fops στην οποία δηλώνεται ποιες συναρτήσεις υλοποιούν τις λειτουργίες που υποστηρίζει η συσκευή.
   * Συσχετίζει τις overloaded συναρτήσεις μας με το char device struct του πυρήνα
   */
	cdev_init(&lunix_chrdev_cdev, &lunix_chrdev_fops); //αρχικοποιούμε τη συσκευή χαρακτήρων με τη αντίστοιχη δομή των file operations
	lunix_chrdev_cdev.owner = THIS_MODULE;

    /*
     * register_chrdev_region
     * Give multiple device numbers to the same device.
     * "The device responds to multiple device numbers."
     */
	dev_no = MKDEV(LUNIX_CHRDEV_MAJOR, 0); //the first device number to which the device responds

    ret = register_chrdev_region(dev_no, lunix_minor_cnt, "Lunix:TNG"); //ανάθεση μιας περιοχής από device minor numbers, αρχίζοντας από τον dev_no δεσμεύει 128 device numbers για τον driver μας
	if (ret < 0) {
		debug("failed to register region, ret = %d\n", ret);
		goto out;
	}
    debug("device registered successfully\n");

    /* cdev_add: From now on the device is "alive" and shall listen to method requests
     * Ενημέρωση του kernel για τους device drivers
     * Ο driver μας είναι πια ζωντανός στο σύστημα και οι λειτουργίες του μπορούν να καλεστούν οποιαδήποτε στιγμή
    */
    ret = cdev_add(&lunix_chrdev_cdev, dev_no, lunix_minor_cnt); //προσθέτουμε στον πυρήνα 128 device numbers ,ξεκινώντας από το dev_no ,τα οποία αντιστοιχούν στην συσκευή χαρακτήρων lunix_chrdev_cdev
	if (ret < 0) {
		debug("failed to add character device\n");
		goto out_with_chrdev_region;
	}
	debug("completed successfully\n");
	return 0;

out_with_chrdev_region:
	unregister_chrdev_region(dev_no, lunix_minor_cnt);
out:
	return ret;
}

void lunix_chrdev_destroy(void){
	dev_t dev_no;
	unsigned int lunix_minor_cnt = lunix_sensor_cnt << 3;

	debug("entering\n");
	dev_no = MKDEV(LUNIX_CHRDEV_MAJOR, 0); //απελευθερώνει τους device numbers που είχε δεσμεύσει η συσκευή χαρακτήρων κατά την αρχικοποίηση της
	cdev_del(&lunix_chrdev_cdev); //διαγράφει τη συσκευή χαρακτήρων από τον πυρήνα
	unregister_chrdev_region(dev_no, lunix_minor_cnt); //απελευθερώνει τους device numbers που είχε δεσμεύσει η συσκευή χαρακτήρων κατά την αρχικοποίηση της
	debug("leaving\n");
}
