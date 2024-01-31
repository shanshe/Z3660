#include <devices/trackdisk.h>

#define STR(s) #s
#define XSTR(s) STR(s)

#define DEVICE_NAME "z3660_scsi.device"
#define DEVICE_DATE "(27 Sep 2023)"
#define DEVICE_ID_STRING "Z3660SCSI " XSTR(DEVICE_VERSION) "." XSTR(DEVICE_REVISION) " " DEVICE_DATE
#define DEVICE_VERSION 1
#define DEVICE_REVISION 02
#define DEVICE_PRIORITY 0

#define ARRAY_SIZE(x) ((sizeof (x) / sizeof ((x)[0])))
#define BIT(x) (1 << (x))

#define A4091_OFFSET_SWITCHES 0

struct MsgPort *W_CreateMsgPort(struct ExecBase *SysBase);
APTR W_CreateIORequest(struct MsgPort *ioReplyPort, ULONG size, struct ExecBase *SysBase);
void W_DeleteMsgPort(struct MsgPort *port, struct ExecBase *SysBase);

#define PRId32 "ld"

typedef struct {
    uint32_t              as_addr;
    struct ExecBase      *as_SysBase;
    int8_t                as_timer_running;
    uint8_t               as_irq_signal;
    uint32_t              as_irq_count;   // Total interrupts
    uint32_t              as_int_mask;
    uint32_t              as_timer_mask;
    struct Task          *as_svc_task;
    struct Interrupt     *as_isr;         // My interrupt server
    volatile uint8_t      as_exiting;
    struct siop_softc    *as_device_private;
    struct MsgPort       *as_timerport[2];
    struct timerequest   *as_timerio[2];
    struct callout      **as_callout_head;
    struct ConfigDev     *as_cd;
    uint32_t             romfile[2];
    /* battmem */
    uint8_t              cdrom_boot;
} a4091_save_t;

void W_DeleteIORequest(APTR iorequest, struct ExecBase *SysBase);
int Save_BattMem(void);

#define DEBUG_BOOTMENU

#define REG_POTGOR       0xDFF016  // Paula proportional pin values [Read]
#define REG_CIAAPRA      0xBFE001  // CIA-A Port A input bits [Read/Write]
#define REG_CIAAPRA_PA6  (1<<6)    // CIA-A Game Port 0 fire button (pin 6)
#define REG_POTGO        0xDFF034  // Paula proportional pin config [Write]
#define REG_POTGOR_DATLY (1<<10)   // Paula Pin 33 P0Y Data
