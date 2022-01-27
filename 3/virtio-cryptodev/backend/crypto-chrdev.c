/*
 * Virtio Cryptodev Device
 *
 * Implementation of virtio-cryptodev qemu backend device.
 *
 * Dimitris Siakavaras <jimsiak@cslab.ece.ntua.gr>
 * Stefanos Gerangelos <sgerag@cslab.ece.ntua.gr>
 * Konstantinos Papazafeiropoulos <kpapazaf@cslab.ece.ntua.gr>
 *
 */

#include "qemu/osdep.h"
#include "qemu/iov.h"
#include "hw/qdev.h"
#include "hw/virtio/virtio.h"
#include "standard-headers/linux/virtio_ids.h"
#include "hw/virtio/virtio-cryptodev.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <crypto/cryptodev.h>

static uint64_t get_features(VirtIODevice *vdev, uint64_t features, Error **errp){
    DEBUG_IN();
    return features;
}

static void get_config(VirtIODevice *vdev, uint8_t *config_data) DEBUG_IN();

static void set_config(VirtIODevice *vdev, const uint8_t *config_data) DEBUG_IN();

static void set_status(VirtIODevice *vdev, uint8_t status) DEBUG_IN();

static void vser_reset(VirtIODevice *vdev) DEBUG_IN();

//Καλείται κάθε φορά που ο frontend οδηγός προσθέτει έναν buffer στην VirtQueue της εικονικής συσκευής χαρακτήρων
//Περνάμε απο το frontend που τρέχει στο kernel space του guest τους buffers, τα struct και ό,τι γυρνάει κάθε system call το backend στο frontend
//Συγκεκριμένα
//ανοίγει το αρχείο /dev/crypto, να παίρνει τα δεδομένα που του έχουμε περάσει μέσω των scatter gather lists
//εκτελεί τα εκάστοτε syscalls στο module του hypervisor και να κλείνει το αρχείο /dev/crypto
static void vq_handle_output(VirtIODevice *vdev, VirtQueue *vq){
    VirtQueueElement *elem;
    unsigned int *syscall_type, *ioctl_type;
    unsigned char *key;
    long *host_ret;
    struct session_op *sess;
    struct crypt_op *cryp;
    uint32_t *ses;
    unsigned char *src, *dst, *iv;

    int *fd_ptr, *fd_ptr_to_close;

    DEBUG_IN();
    //Παίρνει το αντικείμενο από την ουρά, το οποίο είναι τύπου VirtQueueElement
    //διαθετει τους πινακες in_sg και out_sg και κάθε θέση τους αντιστοιχεί μία scatter-gather λίστα
    //in_sg αντιστοιχεί στις λίστες που προορίζονται για εγγραφή
    //out_sg αντιστοιχεί στις λίστες που προορίζονται για ανάγνωση
    elem = virtqueue_pop(vq, sizeof(VirtQueueElement));
    if (!elem) {
        DEBUG("No item to pop from VQ :(");
        return;
    }

    DEBUG("I have got an item from VQ :)");

    //iov_base: χρησιμοποιείται για να ανακτήσουμε την διεύθυνση του αντικειμένου, με το οποίο είχε αρχικοποιηθεί η αντίστοιχη scatter-gather λίστα
    //το πρώτο στοιχείο που προσθέταμε από τον guest στις λίστες ήταν ο τύπος του system call που θέλαμε να γίνει
    syscall_type = elem->out_sg[0].iov_base;
    switch (*syscall_type) {
      // Η διεργασία που εκτελείται στην εικονική μηχανή επιθυμεί να ανοίξει ένα ειδικό αρχείο της εικονικής συσκευής
    case VIRTIO_CRYPTODEV_SYSCALL_TYPE_OPEN:
        DEBUG("VIRTIO_CRYPTODEV_SYSCALL_TYPE_OPEN");
        //Στην πρώτη θέση του πίνακα in_sg βρίσκεται η διεύθυνση του file descriptor που θα πρέπει να χρησιμοποιηθεί για το άνοιγμα του ειδικού αρχείου και να επιστραφεί στον guest
        //και την αναθέτουμε στο δείκτη fd_ptr
        fd_ptr = elem->in_sg[0].iov_base;
        *fd_ptr = open(CRYPTODEV_FILENAME, O_RDWR); //στο ειδικό αρχείο /dev/crypto
        if (*fd_ptr < 0) {
            DEBUG("File does not exist");
        }
        printf("Opened /dev/crypto file with fd = %d\n", *fd_ptr);
        break;
    //Η διεργασία που εκτελείται στην εικονική μηχανή επιθυμεί να κλείσει ένα ανοιχτό αρχείο της εικονικής συσκευής
    case VIRTIO_CRYPTODEV_SYSCALL_TYPE_CLOSE:
        DEBUG("VIRTIO_CRYPTODEV_SYSCALL_TYPE_CLOSE");
        //στην δεύτερη θέση του πίνακα out_sg βρίσκεται η διεύθυνση του file descriptor που θα πρέπει να χρησιμοποιηθεί για το κλείσιμο του ανοιχτού αρχείου
        fd_ptr_to_close = elem->out_sg[1].iov_base;
        if (close(*fd_ptr_to_close) < 0) {
            DEBUG("close() error");
        }
        else {
            printf("Closed /dev/crypto file with fd = %d\n", *fd_ptr_to_close);
        }
        break;
    //Η διεργασία που εκτελείται στην εικονική μηχανή επιθυμεί να κάνει μία κλήση συστήματος ioctl() στην εικονική συσκευή
    case VIRTIO_CRYPTODEV_SYSCALL_TYPE_IOCTL:
        DEBUG("VIRTIO_CRYPTODEV_SYSCALL_TYPE_IOCTL");
        // στην δεύτερη θέση του πίνακα out_sg βρίσκεται η διεύθυνση του file descriptor
        fd_ptr = elem->out_sg[1].iov_base;
        //στην τρίτη θέση του πίνακα out_sg βρίσκεται η διεύθυνση της μεταβλητής που περιγράφει τον τύπο της ioctl() κλήσης
        ioctl_type = elem->out_sg[2].iov_base;
        switch (*ioctl_type) {
        //Θέλουμε να ξεκινήσουμε ένα session
        case VIRTIO_CRYPTODEV_IOCTL_CIOCGSESSION:
            DEBUG("Entering CIOCGSESSION");
            key = elem->out_sg[3].iov_base;
            sess = elem->in_sg[0].iov_base; //ανακτούμε τη διεύθυνση του αντικειμένου sess, δηλαδή του fd του ανοιχτού αρχείου
            host_ret = elem->in_sg[1].iov_base; //ανακτούμε τη διεύθυνση της μεταβητής επιστροφής host_ret, δηλαδή το επεξεργασμένο μήνυμα

            sess->key = key;
            *host_ret = ioctl(*fd_ptr, CIOCGSESSION, sess);
            if (*host_ret) perror("ioctl(CIOCGSESSION)");
            //διαβάζουμε την τιμή του κλειδιού κρυπτογράφησης
            printf("The key is:\n");
            int i;
            for(i=0; i< sess->keylen;i++)  printf("%x", *(sess->key + i));

            printf("\n");
            DEBUG("Leaving CIOCGSESSION");
            break;
        //Θέλουμε να τερματίσουμε ένα session
        case VIRTIO_CRYPTODEV_IOCTL_CIOCFSESSION:
            DEBUG("Entering CIOCFSESSION");
            ses = elem->out_sg[3].iov_base;
            host_ret = elem->in_sg[0].iov_base;

            *host_ret = ioctl(*fd_ptr, CIOCFSESSION, ses);
            if (*host_ret)
                perror("ioctl(CIOCFSESSION)");

            DEBUG("Leaving CIOCFSESSION");
            break;
        //Θέλουμε να κάνουμε κρυπτογράφηση/αποκρυπτογράφηση ενός μηνύματος
        case VIRTIO_CRYPTODEV_IOCTL_CIOCCRYPT:
            DEBUG("Entering CIOCCRYPT");
            //αναθέτουμε τις διευθύνσεις που περιέχουν οι in_sg, out_sg σε μεταβλητές
            //έχουν μεταφερθεί από τον guest στον host μόνο για ανάγνωση
            cryp = elem->out_sg[3].iov_base;
            src = elem->out_sg[4].iov_base;
            iv = elem->out_sg[5].iov_base;
            //είναι για εγγραφή
            host_ret = elem->in_sg[0].iov_base;
            dst = elem->in_sg[1].iov_base; //προορίζεται να αποθηκεύσει το επεξεργασμένο κείμενο
            //Αρχικοποιούμε κατάλληλα τα πεδία και τώρα έχουν τα κατάλληλα δικαιώματα
            cryp->src = src;
            cryp->dst = dst;
            cryp->iv = iv;
            //Το αποτέλεσμα της src αποθηκεύεται στην dst
            *host_ret = ioctl(*fd_ptr, CIOCCRYPT, cryp);
            if (*host_ret)
                perror("ioctl(CIOCCRYPT)");

            printf("\n");
            DEBUG("Leaving CIOCCRYPT");
            break;

        default:
            DEBUG("Unsupported ioctl command!");
            break;
        }

        break;

    default:
        DEBUG("Unknown syscall_type");
        break;
    }

    //προσθέτουμε τα επεξεργασμένα στοιχεία στην ουρά VirtIO ring buffer
    //τα στέλνουμε στο frontend για να τα διαχειριστεί κατάλληλα
    virtqueue_push(vq, elem, 0);

    // ειδοποιείται ο guest για την ύπαρξη νέων δεδομένων
    virtio_notify(vdev, vq);
    g_free(elem);
}
//Δημιουργεί την virtio συσκευή (δηλαδή τις απαραίτητες δομές της) και προσθέτει σε αυτήν μια virtio ουρά που θα χρησιμοποιηθεί για την επικοινωνία guest-qemu
static void virtio_cryptodev_realize(DeviceState *dev, Error **errp){
    VirtIODevice *vdev = VIRTIO_DEVICE(dev);

    DEBUG_IN();

    virtio_init(vdev, "virtio-cryptodev", VIRTIO_ID_CRYPTODEV, 0);
    virtio_add_queue(vdev, 128, vq_handle_output);
}

static void virtio_cryptodev_unrealize(DeviceState *dev, Error **errp) DEBUG_IN();

static Property virtio_cryptodev_properties[] = {
    DEFINE_PROP_END_OF_LIST(),
};
//Αρχικοποιούνται τα βασικά πεδία της κλάσης virtio της συσκευής
static void virtio_cryptodev_class_init(ObjectClass *klass, void *data){
    DeviceClass *dc = DEVICE_CLASS(klass);
    VirtioDeviceClass *k = VIRTIO_DEVICE_CLASS(klass);

    DEBUG_IN();
    dc->props = virtio_cryptodev_properties;
    set_bit(DEVICE_CATEGORY_INPUT, dc->categories);

    k->realize = virtio_cryptodev_realize;
    k->unrealize = virtio_cryptodev_unrealize;
    k->get_features = get_features;
    k->get_config = get_config;
    k->set_config = set_config;
    k->set_status = set_status;
    k->reset = vser_reset;
}

static const TypeInfo virtio_cryptodev_info = {
    .name          = TYPE_VIRTIO_CRYPTODEV,
    .parent        = TYPE_VIRTIO_DEVICE,
    .instance_size = sizeof(VirtCryptodev),
    .class_init    = virtio_cryptodev_class_init,
};

static void virtio_cryptodev_register_types(void) type_register_static(&virtio_cryptodev_info);

type_init(virtio_cryptodev_register_types)
