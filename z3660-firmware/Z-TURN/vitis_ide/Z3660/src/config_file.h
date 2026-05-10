/*
 * config_file.h
 *
 *  Created on: 28 feb. 2023
 *      Author: shanshe
 */

#ifndef SRC_CONFIG_FILE_H_
#define SRC_CONFIG_FILE_H_

enum BOOTMODE{
   MOBOCPU,
   CPU,
   MUSASHI,
   UAE_030,
   UAEJIT_030,
   UAE_040,
   UAEJIT_040,
   BOOTMODE_NUM
};

enum YESNO{
   NO,
   YES,
   YESNO_NUM
};
enum YESNOMIN{
   _NO,
   _YES,
   _MIN,
   YESNOMIN_NUM
};
enum BOOTSCREEN_RES{
   RES_800x600,
   RES_1280x720,
   RES_1920x1080,
   RES_NUM
};
enum ARM_FREQUENCY{
   FREQ_667,
   FREQ_767,
   FREQ_867,
   FREQ_900,
   FREQ_933,
   FREQ_967,
   FREQ_1000,
   FREQ_1033,
   FREQ_1067,
   FREQ_1100,
   FREQ_1133,
   FREQ_1167,
   FREQ_1200,
   FREQ_1233,
   FREQ_1267,
   FREQ_1300,
   FREQ_NUM
};
extern const char *arm_frequency_names[FREQ_NUM];
typedef struct {
   uint32_t start;
   uint32_t length;
   char name[150];
} range;

typedef struct {
   int boot_mode;
   int scsiboot;
   int kickstart;
   int ext_kickstart;
   int scsi_num[7];
   char hdf[20][150];
   int autoconfig_ram;
   int autoconfig_rtg;
   int cpu_ram;
   int mount_sd_0x76;
   int mount_sd_root;
   float resistor;
   float temperature;
   int cpufreq;
   char kickstart0[150];
   char kickstart1[150];
   char kickstart2[150];
   char kickstart3[150];
   char kickstart4[150];
   char kickstart5[150];
   char kickstart6[150];
   char kickstart7[150];
   char kickstart8[150];
   char kickstart9[150];
   char ext_kickstart0[150];
   char ext_kickstart1[150];
   char ext_kickstart2[150];
   char ext_kickstart3[150];
   char ext_kickstart4[150];
   char ext_kickstart5[150];
   char ext_kickstart6[150];
   char ext_kickstart7[150];
   char ext_kickstart8[150];
   char ext_kickstart9[150];
   char sound_language[150];
   int enable_test;
   int bootscreen_resolution;
   int doubled_cursor;
   uint8_t mac_address[6];
   uint8_t pad[2];
   float bp_ton;
   float bp_toff;
   int boot_delay;
   int sd_clock;
   int monitor_switch;
   range test_range[8];
   int arm_frequency;
} CONFIG;
typedef struct {
   int boot_mode;
   int scsiboot;
   int autoconfig_ram;
   int autoconfig_rtg;
   int cpu_ram;
   int mount_sd_0x76;
   int mount_sd_root;
   int kickstart;
   int ext_kickstart;
   int enable_test;
   int cpufreq;
   int bootscreen_resolution;
   int doubled_cursor;
   int scsi_num[7];
   uint8_t mac_address[6];
   char pad[2];
   float bp_ton;
   float bp_toff;
   char preset_name[50];
   int monitor_switch;
   range test_range[8];
   int arm_frequency;
} ENV_FILE_VARS;

enum CONFITEM {
   CONFITEM_NONE,
   CONFITEM_BOOTMODE,
   CONFITEM_KICKSTART,
   CONFITEM_EXT_KICKSTART,
   CONFITEM_SCSI_BOOT_ENABLE,
   CONFITEM_HDF0,
   CONFITEM_HDF1,
   CONFITEM_HDF2,
   CONFITEM_HDF3,
   CONFITEM_HDF4,
   CONFITEM_HDF5,
   CONFITEM_HDF6,
   CONFITEM_HDF7,
   CONFITEM_HDF8,
   CONFITEM_HDF9,
   CONFITEM_HDF10,
   CONFITEM_HDF11,
   CONFITEM_HDF12,
   CONFITEM_HDF13,
   CONFITEM_HDF14,
   CONFITEM_HDF15,
   CONFITEM_HDF16,
   CONFITEM_HDF17,
   CONFITEM_HDF18,
   CONFITEM_HDF19,
   CONFITEM_SCSI0,
   CONFITEM_SCSI1,
   CONFITEM_SCSI2,
   CONFITEM_SCSI3,
   CONFITEM_SCSI4,
   CONFITEM_SCSI5,
   CONFITEM_SCSI6,
   CONFITEM_AUTOCONFIG_RAM_ENABLE,
   CONFITEM_AUTOCONFIG_RTG_ENABLE,
   CONFITEM_CPU_RAM_ENABLE,
   CONFITEM_MOUNT_SD_0x76,
   CONFITEM_MOUNT_SD_ROOT,
   CONFITEM_RESISTOR,
   CONFITEM_TEMPERATURE,
   CONFITEM_CPUFREQ,
   CONFITEM_KICKSTART0,
   CONFITEM_KICKSTART1,
   CONFITEM_KICKSTART2,
   CONFITEM_KICKSTART3,
   CONFITEM_KICKSTART4,
   CONFITEM_KICKSTART5,
   CONFITEM_KICKSTART6,
   CONFITEM_KICKSTART7,
   CONFITEM_KICKSTART8,
   CONFITEM_KICKSTART9,
   CONFITEM_EXT_KICKSTART0,
   CONFITEM_EXT_KICKSTART1,
   CONFITEM_EXT_KICKSTART2,
   CONFITEM_EXT_KICKSTART3,
   CONFITEM_EXT_KICKSTART4,
   CONFITEM_EXT_KICKSTART5,
   CONFITEM_EXT_KICKSTART6,
   CONFITEM_EXT_KICKSTART7,
   CONFITEM_EXT_KICKSTART8,
   CONFITEM_EXT_KICKSTART9,
   CONFITEM_SOUND_LANGUAGE,
   CONFITEM_ENABLE_TEST,
   CONFITEM_BOOTSCREEN_RESOLUTION,
   CONFITEM_DOUBLED_CURSOR,
   CONFITEM_MAC_ADDRESS,
   CONFITEM_BP_TON,
   CONFITEM_BP_TOFF,
   CONFITEM_BOOT_DELAY,
   CONFITEM_SD_CLOCK,
   CONFITEM_MONITOR_SWITCH,
   CONFITEM_TEST_RANGE0,
   CONFITEM_TEST_RANGE1,
   CONFITEM_TEST_RANGE2,
   CONFITEM_TEST_RANGE3,
   CONFITEM_TEST_RANGE4,
   CONFITEM_TEST_RANGE5,
   CONFITEM_TEST_RANGE6,
   CONFITEM_TEST_RANGE7,
   CONFITEM_ARM_FREQUENCY,
   CONFITEM_NUM
};
#define CPUFREQ_MIN 50
#define CPUFREQ_MAX 120

void read_config_file(int verbose);
void read_env_files(int verbose);
extern const char *bootmode_names[];
int write_env_files(ENV_FILE_VARS *env_file);
int write_env_files_boot(ENV_FILE_VARS *env_file);
int write_env_files_scsi(ENV_FILE_VARS *env_file);
int write_env_files_misc(ENV_FILE_VARS *env_file);
int write_env_files_bootscres(ENV_FILE_VARS *env_file);
int write_env_files_preset(ENV_FILE_VARS *env_file);
int delete_env_files(void);
int remove_preset_file(void);
extern CONFIG config,default_config,temp_config;
extern ENV_FILE_VARS env_file_vars_temp[9]; // really size 8
extern int preset_selected;
#endif /* SRC_CONFIG_FILE_H_ */
