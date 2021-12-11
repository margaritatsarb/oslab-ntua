/*
 * lunix-main.c
 *
 * Main module file for Lunix:TNG
 *
 * Vangelis Koukis <vkoukis@cslab.ece.ntua.gr>
 *
 */

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include "lunix.h"
#include "lunix-chrdev.h"
#include "lunix-ldisc.h"
#include "lunix-protocol.h"

/*
 * Global state for Lunix:TNG sensors
 */
int lunix_sensor_cnt = LUNIX_SENSOR_CNT;
struct lunix_sensor_struct *lunix_sensors;
struct lunix_protocol_state_struct lunix_protocol_state;

/*
 * Module init and cleanup functions
 */
//υπεύθυνη για ορισμένες λειτουργίες αρχικοποίησης του οδηγού χαρακτήρων μας και των απαραίτητων δομών το
int __init lunix_module_init(void){
	int ret;
	int si_done;

	printk(KERN_INFO "Initializing the Lunix:TNG module [max %d sensors]\n",
		lunix_sensor_cnt);

	ret = -ENOMEM;
	//Οι αισθητήρες μας είναι 16 στον αριθμό, επομένως ζητάμε από το λειτουργικό μας την αντίστοιχη μνήμη
	lunix_sensors = kzalloc(sizeof(*lunix_sensors) * lunix_sensor_cnt, GFP_KERNEL); //Δέσμευση μνήμης για τις δομές sensor που αποθηκεύουν τα δεδομένα από το line discipline
	if (!lunix_sensors) {
		printk(KERN_ERR "Failed to allocate memory for Lunix sensors\n");
		goto out;
	}
	lunix_protocol_init(&lunix_protocol_state);

	/*
	 * Initialize all sensors. On exit, si_done is the index of the last
	 * successfully initialized sensor.
	 */
	 //Για κάθε σένσορα καλείται η lunix_sensor_init που αρχικοποιεί την δομή μας και ζητά σελίδες μνήμης για την δομή των μετρήσεων, αρχικοποίηση της μηχανής καταστάσεων του πρωτοκόλλου του οδηγού μας.
	for (si_done = -1; si_done < lunix_sensor_cnt - 1; si_done++) {
		debug("initializing sensor %d\n", si_done + 1);
		ret = lunix_sensor_init(&lunix_sensors[si_done + 1]);
		debug("initialized sensor %d, ret = %d\n", si_done + 1, ret);
		if (ret < 0) {
			goto out_with_sensors;
		}
	}

	/*
	 * Initialize the Lunix line discipline
	 */
	if ((ret = lunix_ldisc_init()) < 0)
		goto out_with_sensors;

	/*
	 * Αρχικοποιει τις συσκευές μας και να τις αναθέτει στον πυρήνα του συστήματος
	 */
	if ((ret = lunix_chrdev_init()) < 0)
		goto out_with_ldisc;

	return 0;

	/*
	 * Something's gone wrong, undo everything
	 * we've done up to this point
	 */
out_with_ldisc:
	debug("at out_with_ldisc\n");
	lunix_ldisc_destroy();

out_with_sensors:
	debug("at out_with_sensors\n");
	for (; si_done >= 0; si_done--)
		lunix_sensor_destroy(&lunix_sensors[si_done]);
	kfree(lunix_sensors);

out:
	debug("at out\n");
	return ret;
}

void __exit lunix_module_cleanup(void){ //καλει την lunix_chrdev_destroy() η οποία καθαριζει όποιο υπόλειμμα στη μνήμη απο το kernel module μας
	int si_done;

	debug("entering, destroying chrdev and ldisc\n");
	lunix_chrdev_destroy();
	lunix_ldisc_destroy();

	debug("destroying sensor buffers\n");
	for (si_done = lunix_sensor_cnt - 1; si_done >= 0; si_done--)
		lunix_sensor_destroy(&lunix_sensors[si_done]);
	kfree(lunix_sensors);

	printk(KERN_INFO "Lunix:TNG module unloaded successfully\n");
}


/*
 * Module information section
 */

MODULE_AUTHOR("Vangelis Koukis");
MODULE_LICENSE("GPL");

module_param(lunix_sensor_cnt, int, 0);
MODULE_PARM_DESC(lunix_sensor_cnt, "Maximum number of sensors to support");

module_init(lunix_module_init); //ορίζει ποια συνάρτηση θα κληθεί κατά τη φόρτωση
module_exit(lunix_module_cleanup);
