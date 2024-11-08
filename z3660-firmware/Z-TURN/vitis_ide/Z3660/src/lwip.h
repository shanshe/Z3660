
void lwip_init(void);
void lwip_run(void);
int lwip_get_update_version(char* file_version_loc,int alfa);
int lwip_get_update_version_scsirom(char* file_version_loc,int alfa);
int lwip_get_update_version_jed(char* file_version_loc,int alfa);
int lwip_get_update(char* filename_loc,int alfa);
int lwip_get_update_scsirom(char* filename_loc,int alfa);
int lwip_get_update_jed(char* filename_loc,int alfa);
int lwip_connect(void);
typedef struct{
	uint32_t filesize;
	uint32_t checksum32;
	uint32_t PacketCnt;
	uint32_t IncomingBytes;
	uint32_t filesize_scsirom;
	uint32_t checksum32_scsirom;
   uint32_t filesize_jed;
   uint32_t checksum32_jed;
} DOWNLOAD_DATA;
extern uint8_t *DATA;
int lwip_get_update_version_scsirom(char * file_version_loc,int alfa);
//int lwip_get_update_checksum_scsirom(int alfa);
