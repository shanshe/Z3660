# SCSI
Put in this directory all hdf images that you want to mount with SD SCSI emulation.

Remember their names, you will need to name them exactly in the z3660cfg.txt file.

SD SCSI emulation <b>DO NOT</b> work with UAE single partition disk images. You can check what type the disk image is using a hex editor: if it starts with "RDSK", it is a full drive RDSK/RDB image and will work here. If it starts with "DOS" it is most likely a UAE single partition disk image and will not work.