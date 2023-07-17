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
	char kickstart[50];
} CONFIG;

enum CONFITEM {
	CONFITEM_NONE,
	CONFITEM_BOOTMODE,
	CONFITEM_KICKSTART,
	CONFITEM_NUM
};

void read_config_file(void);

#endif /* SRC_CONFIG_FILE_H_ */
