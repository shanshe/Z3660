#ifndef SCSIMSG_H
#define SCSIMSG_H 1

#include <devices/trackdisk.h>

#define INQUIRY                 0x12
typedef struct scsi_inquiry_data {
    uint8_t device;
#define SID_TYPE                0x1f    /* device type mask */
#define SID_QUAL                0xe0    /* device qualifier mask */
#define SID_QUAL_LU_NOTPRESENT  0x20    /* logical unit not present */
    uint8_t dev_qual2;
    uint8_t version;
    uint8_t response_format;
    uint8_t additional_length;
    uint8_t flags1;

    uint8_t flags2;
#define SID_REMOVABLE           0x80
    uint8_t flags3;
#define SID_SftRe       0x01
#define SID_CmdQue      0x02
#define SID_Linked      0x08
#define SID_Sync        0x10
#define SID_WBus16      0x20
#define SID_WBus32      0x40
#define SID_RelAdr      0x80

#define SID_REMOVABLE           0x80
    uint8_t vendor[8];
    uint8_t product[16];
    uint8_t revision[4];
    uint8_t vendor_specific[20];
    uint8_t flags4;
    uint8_t reserved;
    uint8_t version_descriptor[8][2];
} __packed scsi_inquiry_data_t;  // 74 bytes

typedef struct scsi_generic {
    uint8_t opcode;
    uint8_t bytes[15];
} __packed scsi_generic_t;

#define READ_CAPACITY_10        0x25
typedef struct scsi_read_capacity_10_data {
    uint8_t addr[4];
    uint8_t length[4];
} __packed scsi_read_capacity_10_data_t;

int dev_scsi_inquiry(struct IOExtTD *tio, uint unit, scsi_inquiry_data_t *inq);
scsi_read_capacity_10_data_t * dev_scsi_read_capacity_10(struct IOExtTD *tio, uint lun);
int dev_scsi_get_drivegeometry(struct IOExtTD *tio, struct DriveGeometry *geom);

#endif
