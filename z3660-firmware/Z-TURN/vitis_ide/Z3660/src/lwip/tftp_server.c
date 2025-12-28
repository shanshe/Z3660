/*
 * tftp_server.c
 *
 *  Created on: 20 Jan 2024
 *      Author: Efe Tunca
 */

#include "tftp_server.h"
#include "web_utils.h"
//#include "qspi.h"

#include <string.h>
#include "xil_printf.h"

#include "lwip/inet.h"
#include "lwip/udp.h"

extern struct netif server_netif;
//static int checkBootFileFlag = 0;
static char* filename = "";

static err_t TFTP_sendPacket(struct udp_pcb *pcb, ip_addr_t *addr, int port, char *buf, int buflen)
{
   err_t err;
   struct pbuf *p_buf = pbuf_alloc(PBUF_TRANSPORT, buflen, PBUF_POOL);

   if (!p_buf) {
      xil_printf("Error allocating pbuf\r\n");
      return ERR_MEM;
   }

   memcpy(p_buf->payload, buf, buflen);

   /* sending packet */
   err = udp_sendto(pcb, p_buf, addr, port);
   if (err != ERR_OK)
      xil_printf("UDP send error!\r\n");

   /* free pbuf */
   pbuf_free(p_buf);

   return err;
}

static int TFTP_sendError(struct udp_pcb *pcb, ip_addr_t *ip, int port, TFTP_errCode err)
{
   /* TFTP_errCode error strings */
   static const char *TFTP_errCodeString[] = {
         "not defined",
         "file not found",
         "access violation",
         "disk full",
         "illegal operation",
         "unknown transfer id",
         "file already exists",
         "no such user"
   };
   char buf[MAX_ERR_MSG_LEN] = {0};
   int len;

   setOpCode(buf, TFTP_ERR);
   setErrCode(buf, err);
   setErrMsg(buf, TFTP_errCodeString[err]);

   /* total packet length */
   len = TFTP_PACKET_HDR_LEN + strlen(TFTP_errCodeString[err]) + 1;

   return TFTP_sendPacket(pcb, ip, port, buf, len);
}

static int TFTP_sendDataPacket(struct udp_pcb *pcb, ip_addr_t *ip,
      int port, int block, char *buf, int buflen)
{
   char packet[MAX_MSG_LEN] = {0};

   setOpCode(packet, TFTP_DATA);
   setBlockValue(packet, block);
   setData(packet, buf, buflen);

   return TFTP_sendPacket(pcb, ip, port, packet, buflen + TFTP_PACKET_HDR_LEN);
}

static int TFTP_sendACK(struct udp_pcb *pcb, ip_addr_t *ip, int port, int block)
{
   char packet[MAX_ACK_LEN] = {0};

   setOpCode(packet, TFTP_ACK);
   setBlockValue(packet, block);

   return TFTP_sendPacket(pcb, ip, port, packet, MAX_ACK_LEN);
}

static void TFTP_cleanup(struct udp_pcb *pcb, tftp_arg *args)
{
   /* cleaning up the args */
   f_close(&args->file);
   mem_free(args);

   /* closing the connection */
   udp_remove(pcb);
}

static void TFTP_sendNextBlock(struct udp_pcb *pcb, tftp_arg *args, ip_addr_t *ip, u16 port)
{
   FRESULT Res;

   Res = f_read(&args->file, args->data, DATA_PACKET_MSG_LEN, &args->dataLen);
   if (Res) {
      xil_printf("Closing connection! Err: %d\r\n", args->dataLen);
      return TFTP_cleanup(pcb, args);
   }

   /* sending the data */
   TFTP_sendDataPacket(pcb, ip, port, args->block, args->data, args->dataLen);
}

static void TFTP_readReqRecvCallback(void *_args, struct udp_pcb *upcb,
      struct pbuf *p_buf, ip_addr_t *addr, u16 port)
{
   tftp_arg *args = (tftp_arg *)_args;

   if ((getOpCode(p_buf->payload) == TFTP_ACK) &&
      (args->block == getBlockValue(p_buf->payload))) {

      /* incrementing block number */
      args->block++;
   }
   else {
      /*
       * program could not receive the expected ACK,
       * so the block number will not be updated
       * and current block will resend
       */
      xil_printf("TFTP RRQ: Incorrect ACK received, resending...\r\n");
      TFTP_sendDataPacket(upcb, addr, port, args->block, args->data, args->dataLen);
      pbuf_free(p_buf);
      return;
   }

   pbuf_free(p_buf);

   /*
    * if the last read returned less than the requested number of bytes,
    * then program will send the whole file so it can quit
    */
   if (args->dataLen < DATA_PACKET_MSG_LEN) {
      xil_printf("TFTP RRQ: Transfer completed!\r\n\n");
      return TFTP_cleanup(upcb, args);
   }

   TFTP_sendNextBlock(upcb, args, addr, port);
}

static int TFTP_readProcess(struct udp_pcb *pcb, ip_addr_t *ip, int port, char *fname)
{
   tftp_arg *conn;
   FIL file;
   FRESULT Res;

   Res = f_open(&file, fname, FA_READ);
   if (Res) {
      xil_printf("Unable to open file: %s\r\n", fname);
      TFTP_sendError(pcb, ip, port, ERR_FILE_NOT_FOUND);
      udp_remove(pcb);
      return -1;
   }

   conn = mem_malloc(sizeof *conn);
   if (!conn) {
      xil_printf("Unable to allocate memory for TFTP connection!\r\n");
      TFTP_sendError(pcb, ip, port, ERR_DISK_FULL);
      udp_remove(pcb);
      f_close(&file);
      return -1;
   }

   memcpy(&conn->file, &file, sizeof(file));

   /* setting callback for receiving operations on this pcb */
   udp_recv(pcb, (udp_recv_fn) TFTP_readReqRecvCallback, conn);

   /*
    * initiating the transaction by sending the first block of data.
    * further blocks will be sent when ACKs are received.
    */
   conn->block = 1;

   TFTP_sendNextBlock(pcb, conn, ip, port);

   return 0;
}

static void TFTP_writeReqRecvCallback(void *_args, struct udp_pcb *upcb,
      struct pbuf *p_buf, ip_addr_t *addr, u16 port)
{
   ip_addr_t ip = *addr;
   tftp_arg *args = (tftp_arg *)_args;

   if (p_buf->len != p_buf->tot_len) {
      xil_printf("TFTP WRQ: TFTP Server does not support chained pbufs!\r\n");
      pbuf_free(p_buf);
      return;
   }

   if ((p_buf->len >= TFTP_PACKET_HDR_LEN) &&
      (getBlockValue(p_buf->payload) == (u16)(args->block + 1))) {

      /* writing received data to the file */
      unsigned int numBytesWritten;

      f_write(&args->file, p_buf->payload + TFTP_PACKET_HDR_LEN,
            p_buf->len - TFTP_PACKET_HDR_LEN, &numBytesWritten);

      if (numBytesWritten != (unsigned int)p_buf->len - TFTP_PACKET_HDR_LEN) {
         xil_printf("TFTP WRQ: Write to file error\r\n");
         TFTP_sendError(upcb, &ip, port, ERR_DISK_FULL);
         pbuf_free(p_buf);
         return TFTP_cleanup(upcb, args);
      }
      args->block++;
   }
   TFTP_sendACK(upcb, &ip, port, args->block);

   /*
    * if the last read returned less than the requested number of bytes,
    * then program will send the whole file so it can quit
    */
   if (p_buf->len < DATA_PACKET_MSG_LEN) {
      xil_printf("TFTP WRQ: Transfer completed!\r\n\n");
      TFTP_cleanup(upcb, args);
      setTimestamp(filename);
/*
      if (checkBootFileFlag) {
         checkBootFile();
         doQspiFlash("BOOT.BIN");
         f_chdir("/..");
         checkBootFileFlag = 0;
      }
*/
      listDirectory("0:");
      createIndexFileTree("0:");
      return;
   }
   pbuf_free(p_buf);
}

static int TFTP_writeProcess(struct udp_pcb *pcb, ip_addr_t *ip, int port, char *fname)
{
   tftp_arg *conn;
   FIL file;
   FRESULT Res;

   filename = fname;
#if 0
   if (!strncmp(fname, BOOT_FILE_NAME, sizeof(BOOT_FILE_NAME))) {
      /*
       * In order for the f_chdir function to work,
       * the 'set_fs_rpath' value must be set to '2'
       * in the BSP settings.
       */
      fname = BOOT_FILE_NAME_TEMP;
      f_chdir("/firmwares");
      checkBootFileFlag = 1;
   }
#endif
   Res = f_open(&file, fname, FA_CREATE_ALWAYS | FA_WRITE);
   if (Res) {
      xil_printf("Unable to open file %s for writing [%d]\r\n", fname, Res);
      TFTP_sendError(pcb, ip, port, ERR_DISK_FULL);
      udp_remove(pcb);
      return -1;
   }

   conn = mem_malloc(sizeof *conn);
   if (!conn) {
      xil_printf("Unable to allocate memory for TFTP connection!\r\n");
      TFTP_sendError(pcb, ip, port, ERR_DISK_FULL);
      udp_remove(pcb);
      return -1;
   }

   memcpy(&conn->file, &file, sizeof(file));
   conn->block = 0;

   /* setting callback for receiving operations on this pcb */
   udp_recv(pcb, (udp_recv_fn) TFTP_writeReqRecvCallback, conn);

   /* initiating the transaction by sending the first ACK */
   TFTP_sendACK(pcb, ip, port, conn->block);

   return 0;
}

static void TFTP_recvCallback(void *arg, struct udp_pcb *upcb, struct pbuf *p_buf, ip_addr_t *ip, u16_t port)
{
   (void)arg;
   (void)upcb;
   TFTP_opCode op;
   err_t err;
   char fname[512];
   struct udp_pcb *pcb;

   op = getOpCode(p_buf->payload);

   pcb = udp_new();
   if (!pcb) {
      xil_printf("Error creating PCB. Out of Memory!\r\n");
      goto cleanup;
   }

   /* binding to port 0 to receiving the next available free port */
   err = udp_bind(pcb, IP_ADDR_ANY, 0);
   if (err != ERR_OK) {
      xil_printf("Unable to bind to port %d [%d]\r\n", port, err);
      goto cleanup;
   }

   switch(op) {
   case TFTP_RRQ:
      /* getting the file name from request payload */
      strcpy(fname, p_buf->payload + FILE_NAME_OFFSET);
      xil_printf("TFTP RRQ: %s\r\n", fname);
      TFTP_readProcess(pcb, ip, port, fname);
      break;
   case TFTP_WRQ:
      /* getting the file name from request payload */
      strcpy(fname, p_buf->payload + FILE_NAME_OFFSET);
      xil_printf("TFTP WRQ: %s\r\n", fname);
      TFTP_writeProcess(pcb, ip, port, fname);
      break;
   default:
      /* sending a generic access violation message */
      TFTP_sendError(pcb, ip, port, ERR_ILLEGALOP);
      xil_printf("TFTP unknown request: %d\r\n", op);
      udp_remove(pcb);
      break;
   }

cleanup:
   pbuf_free(p_buf);
}

/* This function make things much less complicated
 * when printing addresses with 'printIPSettings()' function.
 */
static void printIP(char *msg, ip_addr_t *ip)
{
   xil_printf("%s", msg);
   xil_printf("%d.%d.%d.%d\r\n", ip4_addr1(ip), ip4_addr2(ip), ip4_addr3(ip), ip4_addr4(ip));
}

/* Printing the IP settings */
void printIPSettings(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{
   xil_printf("===========================================\r\n");
   printIP("Board IP Address          :  ", ip);
   printIP("Board IP Netmask Address  :  ", mask);
   printIP("Board Gateway Address     :  ", gw);
   xil_printf("===========================================\r\n\r\n");
}

/* Assigning default IP and related addresses */
void assignDefaultIP(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{
   int err;

   xil_printf("\r\nConfiguring default IP    :  %s\r\n\r\n", DEFAULT_IP_ADDRESS);

   err = inet_aton(DEFAULT_IP_ADDRESS, ip);
   if (!err)
      xil_printf("Invalid default IP address: %d\r\n", err);

   err = inet_aton(DEFAULT_IP_MASK, mask);
   if (!err)
      xil_printf("Invalid default IP mask: %d\r\n", err);

   err = inet_aton(DEFAULT_GW_ADDRESS, gw);
   if (!err)
      xil_printf("Invalid default gateway address: %d\r\n", err);
}

void startApplication()
{
   struct udp_pcb *pcb;
   err_t err;

   /* creating new UDP PCB structure */
   pcb = udp_new();
   if (!pcb) {
      xil_printf("Error while creating PCB. Out of Memory!");
      return;
   }

   /* binding the UDP to port 69 */
   err = udp_bind(pcb, IP_ADDR_ANY, TFTP_PORT);
   if (err != ERR_OK) {
      xil_printf("Unable to binding to port %d: error = %d\r\n", TFTP_PORT, err);
      udp_remove(pcb);
      return;
   }

   udp_recv(pcb, (udp_recv_fn) TFTP_recvCallback, NULL);
}

/*
 * This function initializes the file system
 * with the given path name and if specified,
 * it formats the SD card.
 */
int initFileSystem(const char *path, int formatDrive)
{
   (void)formatDrive;
   static FATFS fatfs;
   FRESULT Res;
//   BYTE work[FF_MAX_SS];

   /* Register volume work area, initialize device */
   Res = f_mount(&fatfs, path, 1);
   if (Res != FR_OK) {
      xil_printf("Failed to mount SD card!\r\n");
      return -1;
   }
#if 0
   /*
    * If SD card wanted to be formatted,
    * then format it with creating a file system,
    * and then create 'firmwares' and 'logs' folder.
    *
    * After creating each folder,
    * their time attributes must be set
    * with setTimestamp function.
    */
   if (formatDrive) {
      Res = f_mkfs(path, FM_FAT32, 0, work, sizeof work);
      if (Res != FR_OK) {
         xil_printf("Failed to format SD card!\r\n");
         return -1;
      }

      Res = f_mkdir("logs");
      if (Res != FR_OK) {
         xil_printf("Failed to create \"logs\" directory");
         return -1;
      }
      setTimestamp("/logs");

      Res = f_mkdir("firmwares");
      if (Res != FR_OK) {
         xil_printf("Failed to create \"firmwares\" directory");
         return -1;
      }
      setTimestamp("/firmwares");
   }
#endif
   return 0;
}
