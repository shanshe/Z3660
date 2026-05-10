int zz_usb_init();
int zz_usb_host_init();
unsigned long zz_usb_read_blocks(int dev_index, unsigned long blknr, unsigned long blkcnt, void *buffer);
unsigned long zz_usb_write_blocks(int dev_index, unsigned long blknr, unsigned long blkcnt, void *buffer);
unsigned long zz_usb_storage_capacity(int dev_index);
