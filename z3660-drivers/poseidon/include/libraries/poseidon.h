/****************************************************************************
 * Poseidon USB Stack for Z3660 - Amiga 68060 Side
 * Based on original Poseidon by Chris Hodges
 * Adapted for Z3660 ARM/68060 communication
 ****************************************************************************/

#ifndef LIBRARIES_POSEIDON_H
#define LIBRARIES_POSEIDON_H

#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/semaphores.h>
#include <exec/errors.h>
#include <dos/dos.h>
#include <utility/tagitem.h>
#include <utility/pack.h>

/* Library version */
#define POSEIDON_VERSION    4
#define POSEIDON_REVISION   5

/* Types for psdGetAttrs() and psdSetAttrs() */
#define PGA_STACK      0x01
#define PGA_USBCLASS   0x02
#define PGA_HARDWARE   0x03
#define PGA_DEVICE     0x04
#define PGA_CONFIG     0x05
#define PGA_INTERFACE  0x06
#define PGA_ENDPOINT   0x07
#define PGA_ERRORMSG   0x08
#define PGA_PIPE       0x09
#define PGA_APPBINDING 0x0a
#define PGA_EVENTNOTE  0x0b
#define PGA_STACKCFG   0x0c
#define PGA_PIPESTREAM 0x0d
#define PGA_DESCRIPTOR 0x0e
#define PGA_RTISO      0x0f

/* Tags for psdGetAttrs(PGA_STACK,...) */
#define PA_Dummy             (TAG_USER + 2612)
#define PA_ConfigRead        (PA_Dummy + 0x01)
#define PA_HardwareList      (PA_Dummy + 0x20)
#define PA_ClassList         (PA_Dummy + 0x21)
#define PA_ErrorMsgList      (PA_Dummy + 0x22)
#define PA_GlobalConfig      (PA_Dummy + 0x44)
#define PA_CurrConfigHash    (PA_Dummy + 0x45)
#define PA_SavedConfigHash   (PA_Dummy + 0x46)
#define PA_MemPoolUsage      (PA_Dummy + 0x50)
#define PA_ReleaseVersion    (PA_Dummy + 0x60)
#define PA_OSVersion         (PA_Dummy + 0x61)

/* Tags for psdGetAttrs(PGA_HARDWARE,...) */
#define HA_Dummy             (TAG_USER + 0x2612)
#define HA_DeviceName        (HA_Dummy + 0x10)
#define HA_DeviceUnit        (HA_Dummy + 0x11)
#define HA_ProductName       (HA_Dummy + 0x12)
#define HA_Manufacturer      (HA_Dummy + 0x13)
#define HA_Version           (HA_Dummy + 0x14)
#define HA_Revision          (HA_Dummy + 0x15)
#define HA_Description       (HA_Dummy + 0x16)
#define HA_Copyright         (HA_Dummy + 0x17)
#define HA_DriverVersion     (HA_Dummy + 0x18)
#define HA_Capabilities      (HA_Dummy + 0x19)
#define HA_DeviceList        (HA_Dummy + 0x20)

/* Tags for psdGetAttrs(PGA_DEVICE,...) */
#define DA_Dummy             (TAG_USER + 0x4711)
#define DA_IsLowspeed        (DA_Dummy + 0x01)
#define DA_IsHighspeed       (DA_Dummy + 0x02)
#define DA_IsConnected       (DA_Dummy + 0x03)
#define DA_IsConfigured      (DA_Dummy + 0x04)
#define DA_IsDead            (DA_Dummy + 0x05)
#define DA_IsSuspended       (DA_Dummy + 0x06)
#define DA_Address           (DA_Dummy + 0x10)
#define DA_NumConfigs        (DA_Dummy + 0x11)
#define DA_CurrConfig        (DA_Dummy + 0x12)
#define DA_VendorID          (DA_Dummy + 0x20)
#define DA_ProductID         (DA_Dummy + 0x21)
#define DA_Version           (DA_Dummy + 0x22)
#define DA_Class             (DA_Dummy + 0x23)
#define DA_SubClass          (DA_Dummy + 0x24)
#define DA_Protocol          (DA_Dummy + 0x25)
#define DA_NumInterfaces     (DA_Dummy + 0x26)
#define DA_CurrInterface     (DA_Dummy + 0x27)
#define DA_MaxPktSize0       (DA_Dummy + 0x28)
#define DA_Manufacturer      (DA_Dummy + 0x30)
#define DA_ProductName       (DA_Dummy + 0x31)
#define DA_SerialNumber      (DA_Dummy + 0x32)
#define DA_Hardware          (DA_Dummy + 0x40)
#define DA_ConfigList        (DA_Dummy + 0x41)
#define DA_DescriptorList    (DA_Dummy + 0x42)
#define DA_ClassList         (DA_Dummy + 0x43)
#define DA_BindingList       (DA_Dummy + 0x44)
#define DA_HubDevice         (DA_Dummy + 0x50)
#define DA_UsbVersion        (DA_Dummy + 0x51)
#define DA_AtHubPortNumber   (DA_Dummy + 0x52)
#define DA_PowerSupply       (DA_Dummy + 0x53)
#define DA_PowerDrained      (DA_Dummy + 0x54)
#define DA_InhibitPopup      (DA_Dummy + 0x60)
#define DA_InhibitClassBind  (DA_Dummy + 0x61)

/* Tags for psdGetAttrs(PGA_CONFIG,...) */
#define CA_Dummy             (TAG_USER + 0x0815)
#define CA_ConfigNum         (CA_Dummy + 0x10)
#define CA_MaxPower          (CA_Dummy + 0x11)
#define CA_SelfPowered       (CA_Dummy + 0x12)
#define CA_RemoteWakeup      (CA_Dummy + 0x13)
#define CA_ConfigName        (CA_Dummy + 0x20)
#define CA_Device            (CA_Dummy + 0x30)
#define CA_InterfaceList     (CA_Dummy + 0x31)

/* Tags for psdGetAttrs(PGA_INTERFACE,...) */
#define IFA_Dummy            (TAG_USER + 0x2155)
#define IFA_InterfaceNum     (IFA_Dummy + 0x10)
#define IFA_AlternateNum     (IFA_Dummy + 0x11)
#define IFA_Class            (IFA_Dummy + 0x12)
#define IFA_SubClass         (IFA_Dummy + 0x13)
#define IFA_Protocol         (IFA_Dummy + 0x14)
#define IFA_NumEndpoints     (IFA_Dummy + 0x15)
#define IFA_InterfaceName    (IFA_Dummy + 0x20)
#define IFA_Config           (IFA_Dummy + 0x30)
#define IFA_EndpointList     (IFA_Dummy + 0x31)
#define IFA_AlternateList    (IFA_Dummy + 0x32)
#define IFA_ClassBinding     (IFA_Dummy + 0x40)
#define IFA_BindingClass     (IFA_Dummy + 0x41)

/* Tags for psdGetAttrs(PGA_ENDPOINT,...) */
#define EA_Dummy             (TAG_USER + 0x2047)
#define EA_EndpointNum       (EA_Dummy + 0x10)
#define EA_Direction         (EA_Dummy + 0x11)
#define EA_TransferType      (EA_Dummy + 0x12)
#define EA_MaxPktSize        (EA_Dummy + 0x13)
#define EA_Interval          (EA_Dummy + 0x14)
#define EA_IsIn              (EA_Dummy + 0x15)
#define EA_SyncType          (EA_Dummy + 0x16)
#define EA_UsageType         (EA_Dummy + 0x17)
#define EA_Interface         (EA_Dummy + 0x30)

/* Tags for psdGetAttrs(PGA_PIPE,...) */
#define PPA_Dummy            (TAG_USER + 0x7350)
#define PPA_IORequest        (PPA_Dummy + 0x10)
#define PPA_MessagePort      (PPA_Dummy + 0x11)
#define PPA_Endpoint         (PPA_Dummy + 0x12)
#define PPA_Device           (PPA_Dummy + 0x13)
#define PPA_NoShortPackets   (PPA_Dummy + 0x20)
#define PPA_NakTimeout       (PPA_Dummy + 0x21)
#define PPA_NakTimeoutTime   (PPA_Dummy + 0x22)
#define PPA_AllowRuntPackets (PPA_Dummy + 0x23)

/* USB Request Types */
#define URTF_OUT             0x00
#define URTF_IN              0x80
#define URTF_STANDARD        0x00
#define URTF_CLASS           0x20
#define URTF_VENDOR          0x40
#define URTF_DEVICE          0x00
#define URTF_INTERFACE       0x01
#define URTF_ENDPOINT        0x02
#define URTF_OTHER           0x03

/* USB Standard Requests */
#define USR_GET_STATUS       0x00
#define USR_CLEAR_FEATURE    0x01
#define USR_SET_FEATURE      0x03
#define USR_SET_ADDRESS      0x05
#define USR_GET_DESCRIPTOR   0x06
#define USR_SET_DESCRIPTOR   0x07
#define USR_GET_CONFIGURATION 0x08
#define USR_SET_CONFIGURATION 0x09
#define USR_GET_INTERFACE    0x0A
#define USR_SET_INTERFACE    0x0B
#define USR_SYNCH_FRAME      0x0C

/* USB Descriptor Types */
#define UDT_DEVICE           0x01
#define UDT_CONFIGURATION    0x02
#define UDT_STRING           0x03
#define UDT_INTERFACE        0x04
#define UDT_ENDPOINT         0x05
#define UDT_DEVICE_QUALIFIER 0x06

/* USB Classes */
#define USB_CLASS_PER_INTERFACE     0x00
#define USB_CLASS_AUDIO             0x01
#define USB_CLASS_COMM              0x02
#define USB_CLASS_HID               0x03
#define USB_CLASS_PHYSICAL          0x05
#define USB_CLASS_IMAGE             0x06
#define USB_CLASS_PRINTER           0x07
#define USB_CLASS_MASS_STORAGE      0x08
#define USB_CLASS_HUB               0x09
#define USB_CLASS_CDC_DATA          0x0A
#define USB_CLASS_VENDOR_SPECIFIC   0xFF

/* USB Transfer Types */
#define USEAF_CONTROL        0x00
#define USEAF_ISOCHRONOUS    0x01
#define USEAF_BULK           0x02
#define USEAF_INTERRUPT      0x03

/* Error Codes */
#define UHIOERR_NO_ERROR     0x00
#define UHIOERR_TIMEOUT      0x06
#define UHIOERR_STALL        0x07
#define UHIOERR_BABBLE       0x08
#define UHIOERR_NAK          0x09
#define UHIOERR_HOSTERROR    0x0A
#define UHIOERR_USBOFFLINE   0x0B

/* Forward declarations */
struct PsdDevice;
struct PsdInterface;
struct PsdEndpoint;
struct PsdPipe;
struct PsdConfig;
struct PsdHardware;
struct PsdUsbClass;

/* Opaque structures for Amiga side */
struct PsdDevice {
    struct Node pd_Node;
};

struct PsdInterface {
    struct Node pif_Node;
};

struct PsdEndpoint {
    struct Node pep_Node;
};

struct PsdConfig {
    struct Node pc_Node;
};

struct PsdPipe {
    struct Message pp_Msg;
};

struct PsdHardware {
    struct Node phw_Node;
};

struct PsdUsbClass {
    struct Node puc_Node;
};

/* Library Base */
struct PsdBase {
    struct Library ps_Library;
};

/* Function prototypes for Amiga side */

/* Memory Management */
APTR psdAllocVec(ULONG size);
VOID psdFreeVec(APTR mem);

/* Device Management */
struct PsdDevice *psdFindDevice(struct PsdDevice *pd, struct TagItem *tags);
struct PsdDevice *psdGetNextDevice(struct PsdDevice *pd);
APTR psdGetDeviceAttrs(struct PsdDevice *pd, ULONG attr, ...);
LONG psdSetDeviceAttrs(struct PsdDevice *pd, ULONG attr, ...);

/* Pipe Management */
struct PsdPipe *psdAllocPipe(struct PsdDevice *pd, struct MsgPort *mp, struct PsdEndpoint *pep);
VOID psdFreePipe(struct PsdPipe *pp);
VOID psdPipeSetup(struct PsdPipe *pp, UWORD rt, UWORD rq, UWORD val, UWORD idx);
LONG psdDoPipe(struct PsdPipe *pp, APTR data, ULONG len);
LONG psdDoAsync(struct PsdPipe *pp, APTR data, ULONG len);
VOID psdAbortPipe(struct PsdPipe *pp);

/* Configuration Management */
struct PsdConfig *psdFindConfig(struct PsdDevice *pd, ULONG cfgnum);
LONG psdSetConfig(struct PsdDevice *pd, ULONG cfgnum);

/* Interface Management */
struct PsdInterface *psdFindInterface(struct PsdDevice *pd, ULONG ifnum, ULONG alt);
LONG psdSetInterface(struct PsdInterface *pif, ULONG alt);

/* String Management */
STRPTR psdGetStringDescriptor(struct PsdDevice *pd, ULONG stridx);

/* Hardware Management */
LONG psdAddHardware(struct PsdHardware *phw);
VOID psdRemHardware(struct PsdHardware *phw);

/* Class Management */
LONG psdAddClass(struct PsdUsbClass *puc);
VOID psdRemClass(struct PsdUsbClass *puc);
struct PsdUsbClass *psdFindClass(STRPTR classname);

/* Event System */
#define EHMF_DEVICEREM       0x00000001
#define EHMF_DEVICEADD       0x00000002
#define EHMF_CONFIGCHG       0x00000004
#define EHMF_RESUMEDEV       0x00000008
#define EHMF_SUSPENDDEV      0x00000010

VOID psdAddEventHandler(APTR eh, ULONG events);
VOID psdRemEventHandler(APTR eh);

/* Utility Functions */
LONG psdCheckPipe(struct PsdPipe *pp);
UWORD psdGetVersion(VOID);
STRPTR psdGetVersionString(VOID);

/* Z3660 specific functions */
LONG psdInitStack(VOID);
VOID psdShutdownStack(VOID);

#endif /* LIBRARIES_POSEIDON_H */
