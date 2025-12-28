#ifndef USB_QH_DEFS_H
#define USB_QH_DEFS_H

/* Queue Head Endpoint 1 macros */
#define QH_ENDPT1_C(x)          (((x) & 0x1) << 27)  /* Control Endpoint Flag */
#define QH_ENDPT1_MAXPKT(x)  (((x) & 0x7ff) << 16)  /* Maximum Packet Length */

/* Queue Head Endpoint 2 macros */
#define QH_ENDPT2_SMASK(x)   QH_ENDPT2_UFSMASK(x)  // Use UFSMASK for microframe mask
#define QH_ENDPT2_UFRAME(x)  ((x & 0x7) << 24)     // Start split microframe
#define QH_ENDPT2_CMASK(x)   QH_ENDPT2_UFCMASK(x)  // Use UFCMASK for complete split
#define QH_ENDPT2_INTERVAL(x) ((x & 0x7f) << 0)    // Poll interval

#endif /* USB_QH_DEFS_H */