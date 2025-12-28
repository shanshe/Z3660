/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef BLK_H
#define BLK_H

#ifdef CONFIG_SYS_64BIT_LBA
typedef uint64_t lbaint_t;
#define LBAFlength "ll"
#else
typedef ulong lbaint_t;
#define LBAFlength "l"
#endif
#define LBAF "%" LBAFlength "x"
#define LBAFU "%" LBAFlength "u"

/* Interface types: */
enum if_type {
	IF_TYPE_UNKNOWN = 0,
	IF_TYPE_IDE,
	IF_TYPE_SCSI,
	IF_TYPE_ATAPI,
	IF_TYPE_USB,
	IF_TYPE_DOC,
	IF_TYPE_MMC,
	IF_TYPE_SD,
	IF_TYPE_SATA,
	IF_TYPE_HOST,
	IF_TYPE_NVME,
	IF_TYPE_EFI,
	IF_TYPE_VIRTIO,

	IF_TYPE_COUNT,			/* Number of interface types */
};

#define BLK_VEN_SIZE		40
#define BLK_PRD_SIZE		20
#define BLK_REV_SIZE		8

/*
 * Identifies the partition table type (ie. MBR vs GPT GUID) signature
 */
enum sig_type {
	SIG_TYPE_NONE,
	SIG_TYPE_MBR,
	SIG_TYPE_GUID,

	SIG_TYPE_COUNT			/* Number of signature types */
};

typedef struct {
	u8 b[16];
} efi_guid_t __attribute__((aligned(8)));

/*
 * With driver model (CONFIG_BLK) this is uclass platform data, accessible
 * with dev_get_uclass_platdata(dev)
 */
struct blk_desc {
	/*
	 * TODO: With driver model we should be able to use the parent
	 * device's uclass instead.
	 */
	enum if_type	if_type;	/* type of the interface */
	int		devnum;		/* device number */
	unsigned char	part_type;	/* partition type */
	unsigned char	target;		/* target SCSI ID */
	unsigned char	lun;		/* target LUN */
	unsigned char	hwpart;		/* HW partition, e.g. for eMMC */
	unsigned char	type;		/* device type */
	unsigned char	removable;	/* removable device */
#ifdef CONFIG_LBA48
	/* device can use 48bit addr (ATA/ATAPI v7) */
	unsigned char	lba48;
#endif
	lbaint_t	lba;		/* number of blocks */
	unsigned long	blksz;		/* block size */
	int		log2blksz;	/* for convenience: log2(blksz) */
	char		vendor[BLK_VEN_SIZE + 1]; /* device vendor string */
	char		product[BLK_PRD_SIZE + 1]; /* device product number */
	char		revision[BLK_REV_SIZE + 1]; /* firmware revision */
	enum sig_type	sig_type;	/* Partition table signature type */
	union {
		uint32_t mbr_sig;	/* MBR integer signature */
		efi_guid_t guid_sig;	/* GPT GUID Signature */
	};
	unsigned long	(*block_read)(struct blk_desc *block_dev,
				      lbaint_t start,
				      lbaint_t blkcnt,
				      void *buffer);
	unsigned long	(*block_write)(struct blk_desc *block_dev,
				       lbaint_t start,
				       lbaint_t blkcnt,
				       const void *buffer);
	unsigned long	(*block_erase)(struct blk_desc *block_dev,
				       lbaint_t start,
				       lbaint_t blkcnt);
	void		*priv;		/* driver private struct pointer */
};

#endif
