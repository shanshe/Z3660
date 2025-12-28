
int zz_usb_init(void);
unsigned long zz_usb_read_blocks(int dev_index, unsigned long blknr, unsigned long blkcnt, void *buffer);
unsigned long zz_usb_write_blocks(int dev_index, unsigned long blknr, unsigned long blkcnt, void *buffer);
unsigned long zz_usb_storage_capacity(int dev_index);
void isr_usb(void *dummy);
void z3660_usb_on_transaction_complete(void);
