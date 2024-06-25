
void lwip_init(void);
void lwip_run(void);
int lwip_get_update_version(void);
int lwip_get_update(void);
int lwip_connect(void);
typedef struct{
	uint32_t filesize;
	uint32_t checksum32;
} DOWNLOAD_DATA;
extern uint8_t *DATA;
