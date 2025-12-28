/*
 * tftp_server.h
 *
 *  Created on: 20 Jan 2024
 *      Author: Efe Tunca
 */

#ifndef SRC_TFTP_SERVER_H_
#define SRC_TFTP_SERVER_H_

#include "ff.h"
#include "lwip/ip.h"

#define DEFAULT_IP_ADDRESS     "192.168.1.10"
#define DEFAULT_IP_MASK        "255.255.255.0"
#define DEFAULT_GW_ADDRESS     "192.168.1.1"

#define TFTP_PORT          69

#define MAX_MSG_LEN           600
#define MAX_ACK_LEN           4
#define MAX_ERR_MSG_LEN       30
#define DATA_PACKET_MSG_LEN      512

#define TFTP_PACKET_HDR_LEN      4
#define TFTP_DATA_PACKET_LEN  (DATA_PACKET_MSG_LEN + TFTP_PACKET_HDR_LEN)

/* TFTP packets offsets */
#define OPCODE_OFFSET         0
#define FILE_NAME_OFFSET      2
#define ERRCODE_OFFSET        2
#define BLOCK_OFFSET       2
#define DATA_OFFSET           4

/* Packet form macros */
#define getOpCode(buf) \
      ntohs(*((u16 *)(buf + OPCODE_OFFSET)))

#define setOpCode(buf, opcode) \
      *((u16 *)(buf + OPCODE_OFFSET)) = htons(opcode)

#define getFilename(buf, fname) \
      strcpy(fname, buf + FILE_NAME_OFFSET)

#define setFilename(buf, fname) \
      strcpy(buf + FILE_NAME_OFFSET, fname)

#define setMode(buf, mode, offset) \
      strcpy(buf + offset, mode)

#define getBlockValue(buf) \
      ntohs(*((u16 *)(buf + BLOCK_OFFSET)))

#define setBlockValue(buf, value) \
      *((u16 *)(buf + BLOCK_OFFSET)) = htons(value)

#define getErrCode(buf) \
      ntohs(*((u16 *)(buf + ERRCODE_OFFSET)))

#define setErrCode(buf, err) \
      *((u16 *)(buf + ERRCODE_OFFSET)) = htons(err)

#define setErrMsg(buf, errmsg) \
      strcpy(buf + DATA_OFFSET, errmsg)

#define setData(pkt, buf, len) \
      memcpy(pkt + DATA_OFFSET, buf, len)

typedef enum {
   TFTP_RRQ = 1,
   TFTP_WRQ,
   TFTP_DATA,
   TFTP_ACK,
   TFTP_ERR
} TFTP_opCode;

typedef enum {
   ERR_NOT_DEFINED,
   ERR_FILE_NOT_FOUND,
   ERR_ACCESS_VIOLATION,
   ERR_DISK_FULL,
   ERR_ILLEGALOP,
   ERR_UNKNOWN_TRANSFER_ID,
   ERR_FILE_ALREADY_EXISTS,
   ERR_NO_SUCH_USER
} TFTP_errCode;

typedef struct {
   FIL file;

   /* last block read */
   char data[MAX_MSG_LEN];
   UINT dataLen;

   /* next block number */
   u16 block;
} tftp_arg;

void printIPSettings(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw);
void printAppHeader(int port);
void assignDefaultIP(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw);
void startApplication(void);
int initFileSystem(const char *path, int formatDrive);

#endif /* SRC_TFTP_SERVER_H_ */
