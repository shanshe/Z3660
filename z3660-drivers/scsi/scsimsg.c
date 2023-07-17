#ifdef DEBUG_SCSIMSG
#define USE_SERIAL_OUTPUT
#endif

#include "port.h"
#include "printf.h"
#include <exec/types.h>
#include <exec/io.h>
#include <devices/scsidisk.h>
#include "scsimsg.h"
#include "memory.h"

int
dev_scsi_inquiry(struct IOExtTD *tio, uint unit, scsi_inquiry_data_t *inq)
{
	scsi_generic_t cmd;
	struct SCSICmd scmd;
//	uint lun = unit / 10;

#define SCSIPI_INQUIRY_LENGTH_SCSI2	 36

	memset(&cmd, 0, sizeof (cmd));
	cmd.opcode = INQUIRY;
	cmd.bytes[0] = 0;//lun << 5;
	//cmd.bytes[1] = 0;  // Page code
	//cmd.bytes[2] = 0;
	cmd.bytes[3] = sizeof (scsi_inquiry_data_t);
	//cmd.bytes[4] = 0;  // Control

	memset(&scmd, 0, sizeof (scmd));
	scmd.scsi_Data = (UWORD *) inq;
	scmd.scsi_Length = sizeof (*inq);

	scmd.scsi_Command = (UBYTE *) &cmd;
	scmd.scsi_CmdLength = 6;

	scmd.scsi_Flags = SCSIF_READ | SCSIF_AUTOSENSE;

	scmd.scsi_SenseData = NULL;
	scmd.scsi_SenseLength = 0;

	tio->iotd_Req.io_Command = HD_SCSICMD;
	tio->iotd_Req.io_Length  = sizeof (scmd);
	tio->iotd_Req.io_Data	= &scmd;

	return (DoIO((struct IORequest *) tio));
}

int
dev_scsi_get_drivegeometry(struct IOExtTD *tio, struct DriveGeometry *geom)
{
    tio->iotd_Req.io_Command = TD_GETGEOMETRY;
    tio->iotd_Req.io_Length  = sizeof (*geom);
    tio->iotd_Req.io_Data    = geom;
    tio->iotd_Req.io_Offset  = 0;
    tio->iotd_Req.io_Flags   = 0;

    return (DoIO((struct IORequest *) tio));
}
