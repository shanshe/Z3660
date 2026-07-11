// SD-card file manager for the boot-time serial console.
//
// Presents a small "SD>" command loop over the serial UART with a handful of
// file operations (DIR, COPY, REN, DEL, MKDIR, CRC, FREE) working on both
// mounted volumes: 0: (FAT boot partition) and 1: (exFAT data partition).
// Reached from the boot menu (see show_options()/main_thread() in mobotest.c).

#ifndef SD_FILEOPS_H
#define SD_FILEOPS_H

// Mount both SD volumes, run the interactive "SD>" command loop until the user
// types EXIT, then unmount and return to the caller. Runs on the serial UART.
void sd_fileops_menu(void);

#endif //SD_FILEOPS_H
