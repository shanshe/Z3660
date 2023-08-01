/*
 * config_file.h
 *
 *  Created on: 28 feb. 2023
 *      Author: shanshe
 */

#ifndef SRC_CONFIG_FILE_H_
#define SRC_CONFIG_FILE_H_

enum BOOTMODE{
	CPU,
	MUSASHI,
	UAE,
	UAEJIT,
	BOOTMODE_NUM
};

enum YESNO{
	NO,
	YES,
	YESNO_NUM
};

typedef struct {
	int boot_mode;
	int scsiboot;
	char kickstart[100];
	char scsi[7][100];
	int autoconfig_ram;
} CONFIG;

enum CONFITEM {
	CONFITEM_NONE,
	CONFITEM_BOOTMODE,
	CONFITEM_KICKSTART,
	CONFITEM_SCSI_BOOT_ENABLE,
	CONFITEM_SCSI0,
	CONFITEM_SCSI1,
	CONFITEM_SCSI2,
	CONFITEM_SCSI3,
	CONFITEM_SCSI4,
	CONFITEM_SCSI5,
	CONFITEM_SCSI6,
	CONFITEM_AUTOCONFIG_RAM_ENABLE,
	CONFITEM_NUM
};

void read_config_file(void);
void read_env_files(void);
extern const char *bootmode_names[];

extern CONFIG config;
#endif /* SRC_CONFIG_FILE_H_ */
