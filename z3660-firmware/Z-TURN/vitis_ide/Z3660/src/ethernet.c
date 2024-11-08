/*
 * ethernet.c
 *
 *  Created on: 23 sept. 2022
 *      Author: shanshe
 */

/*
 * MNT ZZ9000 Amiga Graphics and Coprocessor Card Operating System (ZZ9000OS)
 *
 * Copyright (C) 2019, Lukas F. Hartmann <lukas@mntre.com>
 *                     MNT Research GmbH, Berlin
 *                     https://mntre.com
 *
 * More Info: https://mntre.com/zz9000
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * GNU General Public License v3.0 or later
 *
 * https://spdx.org/licenses/GPL-3.0-or-later.html
 *
 * Some portions (from EMACPS example code, weird custom license) Copyright (C) 2010 - 2015 Xilinx, Inc.
 *
 */

#include <stdio.h>
#include "platform.h"
#include <xil_cache.h>
#include <xil_mmu.h>
#include "sleep.h"
//#include "xparameters.h"
#include "xil_types.h"
#include <xemacps.h>
#include <xscugic.h>
#include "ethernet.h"
#include "interrupt.h"
#include "memorymap.h"
#include "main.h"
#include "debug_console.h"
extern DEBUG_CONSOLE debug_console;

static XEmacPs EmacPsInstance;
void DEBUG_ETHERNET(const char *format, ...)
{
   if(debug_console.debug_ethernet==0)
      return;
   va_list args;
   va_start(args, format);
   vprintf(format,args);
   va_end(args);
}

#define FRAME_MAX_BACKLOG 64

#define RXBD_CNT       1   /* Number of RxBDs to use */
#define TXBD_CNT       4   /* Number of TxBDs to use */

// could also be 55, 77 (eth1), see interrupts.pdf last page
// XPS_GEM0_INT_ID == 54
#define EMACPS_IRPT_INTR	XPS_GEM0_INT_ID

#define SLCR_LOCK_ADDR			(XPS_SYS_CTRL_BASEADDR + 0x4)
#define SLCR_UNLOCK_ADDR		(XPS_SYS_CTRL_BASEADDR + 0x8)
#define SLCR_GEM0_CLK_CTRL_ADDR		(XPS_SYS_CTRL_BASEADDR + 0x140)
#define SLCR_GEM1_CLK_CTRL_ADDR		(XPS_SYS_CTRL_BASEADDR + 0x144)

#define SLCR_LOCK_KEY_VALUE		0x767B
#define SLCR_UNLOCK_KEY_VALUE		0xDF0D
#define SLCR_ADDR_GEM_RST_CTRL		(XPS_SYS_CTRL_BASEADDR + 0x214)

#define EMACPS_PHY_DELAY_SEC     4	//Amount of time to delay waiting on PHY to reset
#define EMACPS_SLCR_DIV_MASK	0xFC0FC0FF

static int32_t GemVersion;
uint8_t EmacPsMAC[6] = {0x00,0x80,0x10,0x00,0x01,0x00};

static volatile int32_t DeviceErrors = 0;
static volatile uint32_t FramesTx = 0;
static volatile uint32_t FramesRx = 0;
static volatile uint16_t frame_serial = 0;
static volatile uint32_t frames_received = 0;
static volatile uint16_t frames_backlog = 0;
int frames_received_from_backlog = 0;

#define ETH_PHY_TYPE_MICREL    0
#define ETH_PHY_TYPE_MOTORCOMM 1
static int eth_phy_type_ = ETH_PHY_TYPE_MICREL;

uint32_t PhyAddr;

typedef char EthernetFrame[XEMACPS_MAX_VLAN_FRAME_SIZE_JUMBO] __attribute__ ((aligned(64)));

volatile char* TxFrame = (char*)(RTG_BASE+TX_FRAME_ADDRESS);		/* Transmit buffer */
volatile char* RxFrame = (char*)(RTG_BASE+RX_FRAME_ADDRESS);		/* Receive buffer */

/*
 * Buffer descriptors are allocated in uncached memory. The memory is made
 * uncached by setting the attributes appropriately in the MMU table.
 */
#define RXBD_SPACE_BYTES XEmacPs_BdRingMemCalc(XEMACPS_BD_ALIGNMENT, RXBD_CNT)
#define TXBD_SPACE_BYTES XEmacPs_BdRingMemCalc(XEMACPS_BD_ALIGNMENT, TXBD_CNT)

#define PHY_DETECT_REG1 2
#define PHY_DETECT_REG2 3
#define PHY_ID_MARVELL	0x141
#define PHY_ID_MICREL_KSZ9031 0x22

static void xEmacPsSendHandler(void *Callback);
static void xEmacPsRecvHandler(void *Callback);
static void xEmacPsErrorHandler(void *Callback, uint8_t direction, uint32_t word);
LONG setup_phy(XEmacPs * EmacPsInstancePtr);
static LONG emacPsSetupIntrSystem(XEmacPs *EmacPsInstancePtr, uint16_t EmacPsIntrId);

void micrel_auto_negotiate(XEmacPs *xemacpsp, uint32_t phy_addr, int eth_phy_type);
uint32_t micrel_auto_negotiate_step2(XEmacPs *xemacpsp, uint32_t phy_addr, int eth_phy_type);

enum {
   ETH_TASK_SETUP,
   ETH_TASK_NEGOTIATE,
   ETH_TASK_INIT,
   ETH_TASK_READY
};

int ethernet_task_state = ETH_TASK_SETUP;

void xEmacPsClkSetup(XEmacPs *EmacPsInstancePtr, uint16_t EmacPsIntrId, int link_speed)
{
   uint32_t SlcrTxClkCntrl;
   //uint32_t CrlApbClkCntrl;

   if (GemVersion == 2)
   {
      // SLCR unlock
      *(volatile unsigned int *)(SLCR_UNLOCK_ADDR) = SLCR_UNLOCK_KEY_VALUE;
      if (EmacPsIntrId == XPS_GEM0_INT_ID) {
         // GEM0 clock configuration
         SlcrTxClkCntrl = *(volatile unsigned int *)(SLCR_GEM0_CLK_CTRL_ADDR);

         SlcrTxClkCntrl &= EMACPS_SLCR_DIV_MASK;

         if (link_speed == 100) {
            SlcrTxClkCntrl |= (XPAR_PS7_ETHERNET_0_ENET_SLCR_100MBPS_DIV1 << 20);
            SlcrTxClkCntrl |= (XPAR_PS7_ETHERNET_0_ENET_SLCR_100MBPS_DIV0 << 8);
         } else if (link_speed == 1000) {
            SlcrTxClkCntrl |= (XPAR_PS7_ETHERNET_0_ENET_SLCR_1000MBPS_DIV1 << 20);
            SlcrTxClkCntrl |= (XPAR_PS7_ETHERNET_0_ENET_SLCR_1000MBPS_DIV0 << 8);
         } else if (link_speed == 10) {
            SlcrTxClkCntrl |= (XPAR_PS7_ETHERNET_0_ENET_SLCR_10MBPS_DIV1 << 20);
            SlcrTxClkCntrl |= (XPAR_PS7_ETHERNET_0_ENET_SLCR_10MBPS_DIV0 << 8);
         } else {
            printf("XEmacPsClkSetup: invalid link speed %d\n", link_speed);
         }
         *(volatile unsigned int *)(SLCR_GEM0_CLK_CTRL_ADDR) = SlcrTxClkCntrl;
      } else if (EmacPsIntrId == XPS_GEM1_INT_ID) {
      }
      // SLCR lock
      *(unsigned int *)(SLCR_LOCK_ADDR) = SLCR_LOCK_KEY_VALUE;
   }
}

static XEmacPs_Bd *BdRxPtr;

int init_ethernet_buffers() {
   XEmacPs* EmacPsInstancePtr = &EmacPsInstance;
   XEmacPs_Bd BdTemplate;

   XEmacPs_Stop(EmacPsInstancePtr);

   XEmacPs_BdClear(&BdTemplate);

   int Status = XEmacPs_BdRingCreate(&(XEmacPs_GetRxRing
         (EmacPsInstancePtr)),
         RTG_BASE+RX_BD_LIST_START_ADDRESS,
         RTG_BASE+RX_BD_LIST_START_ADDRESS,
         XEMACPS_BD_ALIGNMENT,
         RXBD_CNT);
   if (Status != XST_SUCCESS) {
      printf("EMAC: Error setting up RxBD space, BdRingCreate\n");
      return(XST_FAILURE);
   }

   Status = XEmacPs_BdRingClone(&(XEmacPs_GetRxRing(EmacPsInstancePtr)),
         &BdTemplate, XEMACPS_RECV);
   if (Status != XST_SUCCESS) {
      printf("EMAC: Error setting up RxBD space, BdRingClone\n");
      return(XST_FAILURE);
   }

   XEmacPs_BdClear(&BdTemplate);
   XEmacPs_BdSetStatus(&BdTemplate, XEMACPS_TXBUF_USED_MASK);

   // Create the TxBD ring
   Status = XEmacPs_BdRingCreate(&(XEmacPs_GetTxRing
         (EmacPsInstancePtr)),
         RTG_BASE+TX_BD_LIST_START_ADDRESS,
         RTG_BASE+TX_BD_LIST_START_ADDRESS,
         XEMACPS_BD_ALIGNMENT,
         TXBD_CNT);
   if (Status != XST_SUCCESS) {
      printf("Error setting up TxBD space, BdRingCreate\n");
      return(XST_FAILURE);
   }
   Status = XEmacPs_BdRingClone(&(XEmacPs_GetTxRing(EmacPsInstancePtr)), &BdTemplate, XEMACPS_SEND);
   if (Status != XST_SUCCESS) {
      printf("Error setting up TxBD space, BdRingClone\n");
      return(XST_FAILURE);
   }

   Status = XEmacPs_BdRingAlloc(&
         (XEmacPs_GetRxRing(EmacPsInstancePtr)),
         RXBD_CNT, &BdRxPtr);
   if (Status != XST_SUCCESS) {
      printf("EMAC: Error allocating RxBDs\n");
      return(XST_FAILURE);
   }
   for (int i=0; i<RXBD_CNT; i++) {
      XEmacPs_BdSetAddressRx(BdRxPtr, RxFrame + FRAME_SIZE*i);
      BdRxPtr = XEmacPs_BdRingNext(&(XEmacPs_GetRxRing(EmacPsInstancePtr)), BdRxPtr);
   }
   Status = XEmacPs_BdRingToHw(&(XEmacPs_GetRxRing(EmacPsInstancePtr)), RXBD_CNT, BdRxPtr);
   if (Status != XST_SUCCESS) {
      printf("EMAC: Error committing RxBD to HW\n");
      return(XST_FAILURE);
   }

   XEmacPs_Start(EmacPsInstancePtr);
   printf("EMAC: XEmacPs_Start done.\n");

   return(XST_SUCCESS);
}

int ethernet_init() {
   XEmacPs_Config *Config;
   long Status;
   XEmacPs* EmacPsInstancePtr = &EmacPsInstance;

   ethernet_task_state = ETH_TASK_SETUP;
   frames_backlog = 0;
   frames_received_from_backlog = 0;

   DeviceErrors = 0;
   FramesTx = 0;
   FramesRx = 0;
   frame_serial = 0;
   frames_received = 0;

   Config = XEmacPs_LookupConfig(XPAR_XEMACPS_0_DEVICE_ID);
   Status = XEmacPs_CfgInitialize(EmacPsInstancePtr, Config, Config->BaseAddress);

   if (Status != XST_SUCCESS) {
      printf("EMAC: Error in initialize\n");
      return(XST_FAILURE);
   }

   GemVersion = ((Xil_In32(Config->BaseAddress + 0xFC)) >> 16) & 0xFFF;
   printf("EMAC: GemVersion: %ld\n", GemVersion);

   Status = XEmacPs_SetMacAddress(EmacPsInstancePtr, EmacPsMAC, 1);
   if (Status != XST_SUCCESS) {
      printf("EMAC: Error setting MAC address\n");
      return(XST_FAILURE);
   }

   Status = XEmacPs_SetHandler(EmacPsInstancePtr, XEMACPS_HANDLER_DMASEND,
         (void *) xEmacPsSendHandler,
         EmacPsInstancePtr);
   Status |=
         XEmacPs_SetHandler(EmacPsInstancePtr, XEMACPS_HANDLER_DMARECV,
               (void *) xEmacPsRecvHandler,
               EmacPsInstancePtr);
   Status |=
         XEmacPs_SetHandler(EmacPsInstancePtr, XEMACPS_HANDLER_ERROR,
               (void *) xEmacPsErrorHandler,
               EmacPsInstancePtr);

   if (Status != XST_SUCCESS) {
      printf("EMAC: Error assigning handlers\n");
      return(XST_FAILURE);
   }

   // FIXME address space?
   /*
    * The BDs need to be allocated in uncached memory. Hence the 1 MB
    * address range that starts at address 0x0FF00000 is made uncached.
    */

   //	Xil_SetTlbAttributes(RTG_BASE+TX_BD_LIST_START_ADDRESS, STRONG_ORDERED);
   //	Xil_SetTlbAttributes(RTG_BASE+TX_FRAME_ADDRESS, STRONG_ORDERED);
   //	Xil_SetTlbAttributes(RTG_BASE+RX_BACKLOG_ADDRESS, STRONG_ORDERED);
   //Xil_SetTlbAttributes(RTG_BASE+RX_FRAME_ADDRESS, 0xc02);
   //Xil_SetTlbAttributes(RTG_BASE+TX_FRAME_ADDRESS, 0xc02);

   XEmacPs_SetMdioDivisor(EmacPsInstancePtr, MDC_DIV_224);

   setup_phy(EmacPsInstancePtr);

   ethernet_update_mac_address();

   return(XST_SUCCESS);
}

void ethernet_task() {
   XEmacPs* EmacPsInstancePtr = &EmacPsInstance;

   switch (ethernet_task_state)
   {
   case ETH_TASK_SETUP:
      // FIXME
      emacPsSetupIntrSystem(EmacPsInstancePtr, EMACPS_IRPT_INTR);

      ethernet_task_state = ETH_TASK_NEGOTIATE;
      break;


   case ETH_TASK_NEGOTIATE: {
      int complete = micrel_auto_negotiate_step2(EmacPsInstancePtr, PhyAddr, eth_phy_type_);

      if (complete!=XST_FAILURE) {
         ethernet_task_state = ETH_TASK_INIT;
      }
      break;


   }
   case ETH_TASK_INIT: {
      // init_ethernet_buffers() also starts EmacPS
      uint16_t status = init_ethernet_buffers();
      if (status != XST_SUCCESS) {
         printf("EMAC: init_ethernet_buffers() error\n");
      }

      ethernet_task_state = ETH_TASK_READY;
      break;
   }
   case ETH_TASK_READY:
      break;
   default:
      ethernet_task_state = ETH_TASK_READY;
   }
}


static void xEmacPsSendHandler(void *Callback)
{
   XEmacPs_Bd *BdTxPtr;
   //XEmacPs *EmacPsInstancePtr = (XEmacPs *) Callback;
   XEmacPs* EmacPsInstancePtr = &EmacPsInstance;

   uint32_t status = XEmacPs_ReadReg(EmacPsInstancePtr->Config.BaseAddress, XEMACPS_TXSR_OFFSET);
   XEmacPs_WriteReg(EmacPsInstancePtr->Config.BaseAddress, XEMACPS_TXSR_OFFSET, status);

   DEBUG_ETHERNET("XEMACPS_TXSR status: %lu\n", status);

   int bds_sent = XEmacPs_BdRingFromHwTx(&(XEmacPs_GetTxRing(EmacPsInstancePtr)), 1, &BdTxPtr);

   if (bds_sent == 1) {
      status = XEmacPs_BdGetStatus(BdTxPtr);
      if(debug_console.debug_ethernet)
      {
         DEBUG_ETHERNET("BD status: ");
         if (status&XEMACPS_TXBUF_USED_MASK) DEBUG_ETHERNET("USED ");
         if (status&XEMACPS_TXBUF_WRAP_MASK) DEBUG_ETHERNET("WRAP ");
         if (status&XEMACPS_TXBUF_RETRY_MASK) DEBUG_ETHERNET("RETRY "); // retry limit exceeded
         if (status&XEMACPS_TXBUF_URUN_MASK) DEBUG_ETHERNET("URUN"); // tx underrun
         if (status&XEMACPS_TXBUF_EXH_MASK) DEBUG_ETHERNET("EXH "); // buffers exhausted
         if (status&XEMACPS_TXBUF_TCP_MASK) DEBUG_ETHERNET("TCP "); // late collision
         if (status&XEMACPS_TXBUF_NOCRC_MASK) DEBUG_ETHERNET("NOCRC "); // no crc
         if (status&XEMACPS_TXBUF_LAST_MASK) DEBUG_ETHERNET("LAST ");
         if (status&XEMACPS_TXBUF_LEN_MASK) DEBUG_ETHERNET("LEN ");
         DEBUG_ETHERNET("\n");
      }
      status = XEmacPs_BdRingFree(&(XEmacPs_GetTxRing(EmacPsInstancePtr)), 1, BdTxPtr);

      if (status != XST_SUCCESS) {
         printf("XEmacPs_BdRingFree error: %lu\n",status);
         return;
      }

      XEmacPs_BdSetStatus(BdTxPtr, XEMACPS_TXBUF_USED_MASK); // XEMACPS_TXBUF_WRAP_MASK

      FramesTx++;
   }
}

#define XEMACPS_BD_TO_INDEX(ringptr, bdptr) \
      (((uint32_t)bdptr - (uint32_t)(ringptr)->BaseBdAddr) / (ringptr)->Separation)

void ethernet_alloc_rx_frames() {
   XEmacPs* EmacPsInstancePtr = &EmacPsInstance;

   XEmacPs_BdRing* rxring = &(XEmacPs_GetRxRing(EmacPsInstancePtr));
   XEmacPs_Bd* rxbd;

   int free_bds = XEmacPs_BdRingGetFreeCnt(rxring);

   for (int i=0; i<free_bds; i++) {

      int Status = XEmacPs_BdRingAlloc(rxring, 1, &rxbd);
      if (Status != XST_SUCCESS) {
         printf("EMAC: Error allocating RxBD\n");
      } else {

         int bd_index=XEMACPS_BD_TO_INDEX(rxring, rxbd);
         XEmacPs_BdClearRxNew(rxbd);
			XEmacPs_BdSetAddressRx(rxbd, RxFrame+bd_index*FRAME_SIZE); // FIXME redundant?

         Status = XEmacPs_BdRingToHw(rxring, 1, rxbd);
         if (Status != XST_SUCCESS) {
            printf("EMAC: Error committing RxBD to HW\n");

            XEmacPs_BdRingUnAlloc(rxring, 1, rxbd); // FIXME double check
         }
      }
   }

   if (!free_bds) {
      printf("EMAC: no BDs free for allocation\n");
   }
}
int num_rx_bufs=0;
extern int interrupt_enabled_ethernet;
static void xEmacPsRecvHandler(void *Callback)
{
   uint32_t status;
//   XEmacPs* EmacPsInstancePtr = (XEmacPs *) Callback;
   XEmacPs* EmacPsInstancePtr = &EmacPsInstance;

   XEmacPs_BdRing* rxring = &(XEmacPs_GetRxRing(EmacPsInstancePtr));
   XEmacPs_Bd* rxbdset, *cur_bd_ptr;

   num_rx_bufs = XEmacPs_BdRingFromHwRx(rxring, RXBD_CNT, &rxbdset);

   // we immediately process the incoming frame.
   // main task will then signal the Amiga via interrupt
   // driver on Amiga side will call ethernet_receive_frame after copying the frame
   // and this will free up the EmacPS receive buffer again.

   if (num_rx_bufs > 0) {
      DEBUG_ETHERNET("EMAC: num_rx_bufs %d\n", num_rx_bufs);

      cur_bd_ptr = rxbdset;

      for (int i=0; i<num_rx_bufs; i++) {

         frame_serial++;
//         printf("f_s %d\n",frame_serial);

         DEBUG_ETHERNET("RX ser: %d\n",frame_serial);

         uint32_t bd_idx = XEMACPS_BD_TO_INDEX(rxring, cur_bd_ptr);
         int rx_bytes = XEmacPs_BdGetLength(cur_bd_ptr);

			uint8_t* frame_ptr = (uint8_t*)(RxFrame + bd_idx*FRAME_SIZE);

         DEBUG_ETHERNET("EMAC: RX: %d [%d] bd_idx: %d\n", frame_serial, rx_bytes, bd_idx);

//         Xil_DCacheInvalidateRange((UINTPTR)frame_ptr, FRAME_SIZE);
//         Xil_L1DCacheFlushRange((UINTPTR)frame_ptr, FRAME_SIZE);
//         Xil_L2CacheFlushRange((UINTPTR)frame_ptr, FRAME_SIZE);

         if (frames_backlog<FRAME_MAX_BACKLOG) {
            // copy the frame to the backlog (frames that amiga hasn't fetched yet)
            uint8_t* frame_bl_ptr = (uint8_t*)(RTG_BASE+RX_BACKLOG_ADDRESS+frames_backlog*FRAME_SIZE);
            memcpy(frame_bl_ptr+RX_FRAME_PAD, frame_ptr, rx_bytes);
            if(rx_bytes>FRAME_SIZE)
               printf("rx_bytes>FRAME_SIZE!!! <--------------------");
//            printf("f_s %d rx_b %d fb %d\n",frame_serial,rx_bytes,frames_backlog);

            *(frame_bl_ptr)   = (rx_bytes&0xff00)>>8;
            *(frame_bl_ptr+1) = (rx_bytes&0xff);
            *(frame_bl_ptr+2) = (frame_serial&0xff00)>>8;
            *(frame_bl_ptr+3) = (frame_serial&0xff);

            frames_backlog++;

            DEBUG_ETHERNET("bd %ld [%d] copied from %p to %p\n", bd_idx, rx_bytes, frame_ptr, frame_bl_ptr);
         } else {
            if(interrupt_enabled_ethernet)
               DEBUG_ETHERNET("EMAC: frames_backlog full\n");
            frames_backlog = 0;
            frames_received_from_backlog = 0;

         }

         XEmacPs_BdClearRxNew(cur_bd_ptr);
         cur_bd_ptr = XEmacPs_BdRingNext(rxring, cur_bd_ptr);

         frames_received++;
      }

      int Status = XEmacPs_BdRingFree(rxring, num_rx_bufs, rxbdset);
      if (Status != XST_SUCCESS) {
         DEBUG_ETHERNET("EMAC: Error freeing RxBDs\n");
      } else {
         DEBUG_ETHERNET("EMAC: freed %d RxBDs\n", num_rx_bufs);
      }

      ethernet_alloc_rx_frames();

      DEBUG_ETHERNET("EMAC: backlog %d fetched %d\n", frames_backlog, frames_received_from_backlog);
   }

   status = XEmacPs_ReadReg(EmacPsInstancePtr->Config.BaseAddress, XEMACPS_RXSR_OFFSET);
   XEmacPs_WriteReg(EmacPsInstancePtr->Config.BaseAddress, XEMACPS_RXSR_OFFSET, status);
}

volatile uint8_t* ethernet_current_receive_ptr() {
   return((volatile uint8_t*)(RX_BACKLOG_ADDRESS+frames_received_from_backlog*FRAME_SIZE));
}

int ethernet_get_backlog() {
   return(frames_backlog);
}

int ethernet_receive_frame() {
   volatile uint8_t* frm = ethernet_current_receive_ptr()+RTG_BASE;
   //	printf("ethernet_receive_frame: %d bytes: %d serial: %d frames_backlog %d\n", frames_received_from_backlog, frm[0]<<8|frm[1], frm[2]<<8|frm[3],frames_backlog);
   /*
	int frame_size=frm[0]<<8|frm[1];
	volatile uint8_t* frm2=frm+4;
	for (int y=0; y<frame_size; y++) {
		printf("%02x",frm2[y]);
		if (y%4==3) printf(" ");
		if (y%32==31) printf("\n");
	}
	printf("\n==========================================\n");
    */
   if (frames_backlog>0) {
      frames_received_from_backlog++;
      if (frames_received_from_backlog >= frames_backlog) {
         // start over
         frames_backlog = 0;
         frames_received_from_backlog = 0;

         // fake the frame serial (so Amiga doesn't check this frame again)
         uint8_t old_serial_0=frm[2];
         uint8_t old_serial_1=frm[3];
         frm = ethernet_current_receive_ptr()+RTG_BASE;
         frm[2]=old_serial_0;
         frm[3]=old_serial_1;

         DEBUG_ETHERNET("EMAC: caught up with backlog\n");
      }
   } else {
      // this is NOT an error, Amiga wants data and there is no data on RX buffers
      DEBUG_ETHERNET("EMAC: ethernet_receive_frame() called but backlog is empty\n");
      frames_received_from_backlog=0;
   }
   return(frames_received_from_backlog);
}

uint32_t get_frames_received() {
   return(frames_received);
}

static int frames_dropped = 0;

uint8_t* ethernet_get_mac_address_ptr() {
   return((uint8_t*)&EmacPsMAC);
}

void ethernet_update_mac_address() {
   XEmacPs* EmacPsInstancePtr = &EmacPsInstance;

   if (ethernet_task_state != ETH_TASK_READY) {
      return;
   }

   printf("Ethernet: New MAC address %02x:%02x:%02x:%02x:%02x:%02x\n",
         EmacPsMAC[0],EmacPsMAC[1],EmacPsMAC[2],EmacPsMAC[3],EmacPsMAC[4],EmacPsMAC[5]);

   XEmacPs_Stop(EmacPsInstancePtr);

   int Status = XEmacPs_SetMacAddress(EmacPsInstancePtr, EmacPsMAC, 1);
   if (Status != XST_SUCCESS) {
      printf("EMAC: Error setting MAC address\n");
   }

   XEmacPs_Start(EmacPsInstancePtr);
}

static void xEmacPsErrorHandler(void *Callback, uint8_t Direction, uint32_t ErrorWord)
{
   //XEmacPs *EmacPsInstancePtr = (XEmacPs *) Callback;

   DeviceErrors++;

   switch (Direction) {
   case XEMACPS_RECV:
      if (ErrorWord & XEMACPS_RXSR_HRESPNOK_MASK) {
         printf("EMAC: Receive DMA error\n");
      }
      if (ErrorWord & XEMACPS_RXSR_RXOVR_MASK) {
         DEBUG_ETHERNET("EMAC: Receive over run\n");
      }
      if (ErrorWord & XEMACPS_RXSR_BUFFNA_MASK) {
         printf("EMAC: Receive buffer not available\n");
         // signal to host that frames are available
         frames_dropped++;
         frames_received++;
         if (frames_dropped%10 == 0) {
            DEBUG_ETHERNET("ETHDROP: %d\n",frames_dropped);
            ethernet_alloc_rx_frames();
         }
      }
      break;
   case XEMACPS_SEND:
      if (ErrorWord & XEMACPS_TXSR_HRESPNOK_MASK) {
         printf("EMAC: Transmit DMA error\n");
      }
      if (ErrorWord & XEMACPS_TXSR_URUN_MASK) {
         printf("EMAC: Transmit under run\n");
      }
      if (ErrorWord & XEMACPS_TXSR_BUFEXH_MASK) {
         printf("EMAC: Transmit buffer exhausted\n");
      }
      if (ErrorWord & XEMACPS_TXSR_RXOVR_MASK) {
         printf("EMAC: Transmit retry excessed limits\n");
      }
      if (ErrorWord & XEMACPS_TXSR_FRAMERX_MASK) {
         printf("EMAC: Transmit collision\n");
      }
      if (ErrorWord & XEMACPS_TXSR_USEDREAD_MASK) {
         printf("EMAC: Transmit buffer not available\n");
      }
      break;
   }
   /*
    * Bypassing the reset functionality as the default tx status for q0 is
    * USED BIT READ. so, the first interrupt will be tx used bit and it resets
    * the core always.
    */
   if (GemVersion == 2) {
      //EmacPsResetDevice(EmacPsInstancePtr);
   }
}

uint32_t xEmacPsDetectPHY(XEmacPs * EmacPsInstancePtr)
{
   uint32_t PhyAddr;
   uint32_t Status;
   uint16_t PhyReg1;
   uint16_t PhyReg2;

   for (PhyAddr = 0; PhyAddr <= 31; PhyAddr++) {
      Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr,
            PHY_DETECT_REG1, &PhyReg1);

      Status |= XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr,
            PHY_DETECT_REG2, &PhyReg2);

      if ((Status == XST_SUCCESS) &&
            (PhyReg1 > 0x0000) && (PhyReg1 < 0xffff) &&
            (PhyReg2 > 0x0000) && (PhyReg2 < 0xffff)) {
         /* Found a valid PHY address */
         return(PhyAddr);
      }
   }

   return(PhyAddr);		/* default to 32(max of iteration) */
}

LONG setup_phy(XEmacPs * EmacPsInstancePtr)
{
   uint16_t PhyIdentity;

   PhyAddr = xEmacPsDetectPHY(EmacPsInstancePtr);

   if (PhyAddr >= 32) {
      printf("EMAC: Error detecting PHY\n");
      return(XST_FAILURE);
   }

   XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, PHY_DETECT_REG1, &PhyIdentity);

   if (PhyIdentity == PHY_ID_MICREL_KSZ9031) {
      eth_phy_type_ = ETH_PHY_TYPE_MICREL;
      printf("EMAC: MICROCHIP KSZ9031 PHY detected\n");
      micrel_auto_negotiate(EmacPsInstancePtr, PhyAddr,eth_phy_type_);
      return(XST_SUCCESS);
   }
   else if (PhyIdentity == 0x4f51) {
      eth_phy_type_ = ETH_PHY_TYPE_MOTORCOMM;
      printf("EMAC: MOTORCOMM YT8531S PHY detected\n");
      micrel_auto_negotiate(EmacPsInstancePtr, PhyAddr,eth_phy_type_);
      return(XST_SUCCESS);
   }
   else {
      printf("EMAC: Unsupported PHY with id 0x%x\n",PhyIdentity);
   }

   return(XST_FAILURE);
}

#define ADVERTISE_10HALF	0x0020  /* Try for 10mbps half-duplex  */
#define ADVERTISE_1000XFULL	0x0020  /* Try for 1000BASE-X full-duplex */
#define ADVERTISE_10FULL	0x0040  /* Try for 10mbps full-duplex  */
#define ADVERTISE_1000XHALF	0x0040  /* Try for 1000BASE-X half-duplex */
#define ADVERTISE_100HALF	0x0080  /* Try for 100mbps half-duplex */
#define ADVERTISE_1000XPAUSE	0x0080  /* Try for 1000BASE-X pause    */
#define ADVERTISE_100FULL	0x0100  /* Try for 100mbps full-duplex */
#define ADVERTISE_1000XPSE_ASYM	0x0100  /* Try for 1000BASE-X asym pause */
#define ADVERTISE_100BASE4	0x0200  /* Try for 100mbps 4k packets  */

#define ADVERTISE_100_AND_10	(ADVERTISE_10FULL | ADVERTISE_100FULL | ADVERTISE_10HALF | ADVERTISE_100HALF)
#define ADVERTISE_100		(ADVERTISE_100FULL | ADVERTISE_100HALF)
#define ADVERTISE_10		(ADVERTISE_10FULL | ADVERTISE_10HALF)

#define ADVERTISE_1000		0x0300

#define IEEE_ASYMMETRIC_PAUSE_MASK				0x0800
#define IEEE_PAUSE_MASK							0x0400
#define IEEE_AUTONEG_ERROR_MASK					0x8000

#define IEEE_CONTROL_REG_OFFSET					0
#define IEEE_STATUS_REG_OFFSET					1
#define IEEE_AUTONEGO_ADVERTISE_REG				4
#define IEEE_PARTNER_ABILITIES_1_REG_OFFSET		5
#define IEEE_PARTNER_ABILITIES_2_REG_OFFSET		8
#define IEEE_1000BASET_CONTROL_REG  9
#define IEEE_1000BASET_STATUS_REG	10
#define IEEE_COPPER_SPECIFIC_CONTROL_REG		16
#define IEEE_SPECIFIC_STATUS_REG				17
#define IEEE_COPPER_SPECIFIC_STATUS_REG_2		19
#define IEEE_EXT_PHY_SPECIFIC_CONTROL_REG   	20
#define IEEE_CONTROL_REG_MAC					21
#define IEEE_PAGE_ADDRESS_REGISTER				22

#define IEEE_CTRL_1GBPS_LINKSPEED_MASK			0x2040
#define IEEE_CTRL_LINKSPEED_MASK				0x0040
#define IEEE_CTRL_LINKSPEED_1000M				0x0040
#define IEEE_CTRL_LINKSPEED_100M				0x2000
#define IEEE_CTRL_LINKSPEED_10M					0x0000
#define IEEE_CTRL_RESET_MASK					0x8000
#define IEEE_CTRL_AUTONEGOTIATE_ENABLE			0x1000
#define IEEE_STAT_AUTONEGOTIATE_CAPABLE			0x0008
#define IEEE_STAT_AUTONEGOTIATE_COMPLETE		0x0020
#define IEEE_STAT_AUTONEGOTIATE_RESTART			0x0200
#define IEEE_STAT_1GBPS_EXTENSIONS				0x0100
#define IEEE_AN1_ABILITY_MASK					0x1FE0
#define IEEE_AN3_ABILITY_MASK_1GBPS				0x0C00
#define IEEE_AN1_ABILITY_MASK_100MBPS			0x0380
#define IEEE_AN1_ABILITY_MASK_10MBPS			0x0060
#define IEEE_RGMII_TXRX_CLOCK_DELAYED_MASK		0x0030

#define IEEE_1000BASE_STATUS_REG 0x0a

void micrel_auto_negotiate(XEmacPs *xemacpsp, uint32_t phy_addr, int eth_phy_type)
{
   uint16_t control;
   uint16_t status;
   int link_speed = 100;

   if(eth_phy_type == ETH_PHY_TYPE_MOTORCOMM) {
      printf("PHY: Start Ethernet PHY auto negotiation (MotorCom YT8531S)\n"); //MotorComm
      // access IEEE MII regs
      //		XEmacPs_PhyWrite(xemacpsp, phy_addr, 0x100, 0x6);
   } else {
      printf("PHY: Start Ethernet PHY auto negotiation (Microchip KSZ9031)\n"); //Micrel
   }

   XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_PAGE_ADDRESS_REGISTER, 2);
   XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_CONTROL_REG_MAC, &control);
   control |= IEEE_RGMII_TXRX_CLOCK_DELAYED_MASK;
   XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_CONTROL_REG_MAC, control);
   XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_PAGE_ADDRESS_REGISTER, 0);

   XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, &control);  //reg 0x04
   control |= IEEE_ASYMMETRIC_PAUSE_MASK;   //0x0800
   control |= IEEE_PAUSE_MASK;
   control |= ADVERTISE_100;
   control |= ADVERTISE_10;
   XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, control);

   if (link_speed == 100) {
      // register 0: 100 mbit
      XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
      control &= ~(1<<6);
      // FIXME: why is this disabled?
      //control |= (1<<13);
      XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, control);

      // register 9, disable 1000base-t
      XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_1000BASET_CONTROL_REG, &control);
      control &= ~(1<<9 | 1<<8);
      XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_1000BASET_CONTROL_REG,control);
   } else if (link_speed == 1000) {
      // register 0: 1000 mbit
      /*XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
		control |= (1<<6);
		control &= ~(1<<13);
		XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, control);*/

      // register 9
      XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_1000BASET_CONTROL_REG, &control);
      control |= ADVERTISE_1000;
      XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_1000BASET_CONTROL_REG, control);

      // this is "reserved" according to manual?!
      // page 0, register 10h
      XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_COPPER_SPECIFIC_CONTROL_REG,&control);
      control |= (7 << 12); // max number of gigabit attempts
      control |= (1 << 11); // enable downshift
      XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_COPPER_SPECIFIC_CONTROL_REG,control);
   }

   // register 0
   XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
   if (link_speed == 1000) {
      control |= IEEE_CTRL_AUTONEGOTIATE_ENABLE;
   }
   control |= IEEE_STAT_AUTONEGOTIATE_RESTART;
   XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, control);

   if (link_speed == 1000) {
      XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
      control |= IEEE_CTRL_RESET_MASK;
      XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, control);

      while (1) {
         XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
         if (control & IEEE_CTRL_RESET_MASK) continue; // ??? weird
         else break;
      }
   }

   XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_STATUS_REG_OFFSET, &status);

   printf("PHY: Waiting for PHY to complete auto negotiation. (status: %x).\n", status);
}

uint32_t micrel_auto_negotiate_step2(XEmacPs *xemacpsp, uint32_t phy_addr, int eth_phy_type) {
   uint16_t status;
   uint16_t status_speed;
   uint16_t link_speed = 1; // XST_FAILURE

   XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_STATUS_REG_OFFSET, &status);

   //	printf("PHY status: %x\n", status);

   if (status & IEEE_STAT_AUTONEGOTIATE_COMPLETE) {
      if(eth_phy_type == ETH_PHY_TYPE_MOTORCOMM) {
         // http://www.fpgadeveloper.com/2018/05/board-bring-up-myir-myd-y7z010-dev-board.html
         XEmacPs_PhyRead(xemacpsp, phy_addr, 0x11, &status_speed);

         if ((status_speed & 0xc000) == 0)
            link_speed = 10;
         else if((status_speed & 0xc000) == 0x4000)
            link_speed = 100;
         else if((status_speed & 0xc000) == 0x8000)
            link_speed = 1000;

         printf("PHY (MotorComm): Link speed: %d mbit (status: %x)\n", link_speed, status_speed);
         printf("PHY (MotorComm): Link up: %x\n",(status_speed&(1<<10))>>10);
      } else {
         // http://www.fpgadeveloper.com/2018/05/board-bring-up-myir-myd-y7z010-dev-board.html

         XEmacPs_PhyRead(xemacpsp, phy_addr, 0x1F, &status_speed);

         if (status_speed & 0x040)
            link_speed = 1000;
         else if(status_speed & 0x020)
            link_speed = 100;
         else if(status_speed & 0x010)
            link_speed = 10;

         printf("PHY (Micrel): Link speed: %d mbit\n",link_speed);
      }

      XEmacPs_SetOperatingSpeed(xemacpsp, link_speed);
      xEmacPsClkSetup(xemacpsp, EMACPS_IRPT_INTR, link_speed);

      return(link_speed);
   } else {
      uint16_t temp;

      // TODO: why?
      XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_COPPER_SPECIFIC_STATUS_REG_2, &temp);
      //timeout_counter++;

      //if (timeout_counter > 2) {
      //	printf("PHY: Auto negotiation timeout\n");
      //	break;
      //}

      return(XST_FAILURE);
   }
}

/****************************************************************************/
/**
 *
 * This function setups the interrupt system so interrupts can occur for the
 * EMACPS.
 * @param	EmacPsInstancePtr is a pointer to the instance of the EmacPs
 *		driver.
 * @param	EmacPsIntrId is the Interrupt ID and is typically
 *		XPAR_<EMACPS_instance>_INTR value from xparameters.h.
 *
 * @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
 *
 * @note		None.
 *
 *****************************************************************************/
static LONG emacPsSetupIntrSystem(XEmacPs *EmacPsInstancePtr, uint16_t EmacPsIntrId)
{
   LONG Status;
   XScuGic *IntcInstancePtr = interrupt_get_intc();

   /*
    * Connect a device driver handler that will be called when an
    * interrupt for the device occurs, the device driver handler performs
    * the specific interrupt processing for the device.
    */
   Status = XScuGic_Connect(IntcInstancePtr, EmacPsIntrId,
         (Xil_InterruptHandler) XEmacPs_IntrHandler,
         (void *) EmacPsInstancePtr);
   if (Status != XST_SUCCESS) {
      printf("GIC: Unable to connect ISR to interrupt controller\n");
      return(XST_FAILURE);
   }

   /*
    * Enable interrupts from the hardware
    */
   XScuGic_Enable(IntcInstancePtr, EmacPsIntrId);
   printf("GIC: SCU GIC enabled\n");

   /*
    * Enable interrupts in the processor
    */
   //	Xil_ExceptionEnable();

   printf("GIC: Interrupts enabled\n");
   return(XST_SUCCESS);
}

uint16_t ethernet_send_frame(uint16_t frame_size) {
   XEmacPs* EmacPsInstancePtr = &EmacPsInstance;
   XEmacPs_Bd *BdTxPtr;

   if (ethernet_task_state != ETH_TASK_READY) {
      return(1);
   }

   uint32_t old_frames_tx = FramesTx;

//   Xil_DCacheInvalidateRange((UINTPTR)TxFrame, sizeof(EthernetFrame));
	Xil_L1DCacheFlushRange((UINTPTR)TxFrame, sizeof(EthernetFrame));
	Xil_L2CacheFlushRange((UINTPTR)TxFrame, sizeof(EthernetFrame));

   //	printf("ethernet_send_frame: %lu %d\n",old_frames_tx,frame_size);
   /*
	for (int y=0; y<frame_size; y++) {
		printf("%02x",TxFrame[y]);
		if (y%4==3) printf(" ");
		if (y%32==31) printf("\n");
	}
	printf("\n==========================================\n");
    */
   LONG Status = XEmacPs_BdRingAlloc(&(XEmacPs_GetTxRing(EmacPsInstancePtr)), 1, &BdTxPtr);

   if (Status != XST_SUCCESS) {
      printf("ERROR: BdRingAlloc error: %ld\n",Status);

      // lets unstick this
      //init_ethernet_buffers();
      return(2);
   }

   XEmacPs_BdSetAddressTx(BdTxPtr, TxFrame);
   XEmacPs_BdSetLength(BdTxPtr, frame_size);
   XEmacPs_BdClearTxUsed(BdTxPtr);
   XEmacPs_BdSetLast(BdTxPtr);

   Status = XEmacPs_BdRingToHw(&(XEmacPs_GetTxRing(EmacPsInstancePtr)), 1, BdTxPtr);

   if (Status != XST_SUCCESS) {
      printf("ERROR: BdRingToHw error: %ld\n",Status);
      return(3);
   }

   //	Xil_L1DCacheFlushRange((UINTPTR)BdTxPtr, 128);
   //	Xil_L2CacheFlushRange((UINTPTR)BdTxPtr, 128);

   XEmacPs_Transmit(EmacPsInstancePtr);

   DEBUG_ETHERNET("FramesTx:%ld\n",FramesTx);
   DEBUG_ETHERNET("BdTxPtr:0x%08lx\n",(uint32_t)BdTxPtr);
   uint32_t counter = 0;
   while (old_frames_tx == FramesTx) {
      usleep(100);
      counter++;
      // 1ms
      if (counter>10) {
         printf("ERROR: timeout in ethernet_send_frame waiting for tx!\n");
         //init_ethernet_buffers();
         return(4);
      }
   }
   DEBUG_ETHERNET("frame %ld transmitted!\n",FramesTx);

   // all good
   return(0);
}


