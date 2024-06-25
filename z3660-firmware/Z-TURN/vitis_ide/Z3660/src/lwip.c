#include "lwip/altcp_tcp.h"
#include "lwip/dns.h"
#include "lwip/debug.h"
#include "lwip/mem.h"
#include "lwip/altcp_tls.h"
#include "lwip/init.h"
#include "lwip/tcp.h"
#include "lwip/dhcp.h"
#include "lwip/init.h"
#include "lwip/inet.h"
#include "sleep.h"
#include "lwip/priv/tcp_priv.h"
#include "netif/xadapter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lwip/apps/http_client.h"
#include "xscutimer.h"
#include "xscugic.h"
#include "lwip.h"
DOWNLOAD_DATA download_data;

void txrx_loop(void);

void lwip_run(void)
{
	txrx_loop();
}

char url[500];
char domain_name[300];
httpc_connection_t conn_settings;
httpc_state_t *connection;
err_t RecvHttpHeaderCallback (httpc_state_t *connection, void *arg, struct pbuf *hdr, u16_t hdr_len, u32_t content_len);
void HttpClientResultCallback (void *arg, httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err);
err_t RecvBOOTbinCallback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
err_t RecvVersionCallback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
err_t RecvVersionScsiRomCallback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
err_t RecvScsiRomCallback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

ip_addr_t google_dnsserver;

#include "lwip/dns.h"
char version_string[5000];
char version_string_export[50];
char version_scsirom_string_export[50];
volatile int received=0;
struct netif *netif;
volatile int TcpFastTmrFlag = 0;
volatile int TcpSlowTmrFlag = 0;
volatile int dhcp_timoutcntr = 24;
void dhcp_fine_tmr();
void dhcp_coarse_tmr();
#define INTC_DEVICE_ID         XPAR_SCUGIC_SINGLE_DEVICE_ID
#define TIMER_DEVICE_ID        XPAR_SCUTIMER_DEVICE_ID
#define INTC_BASE_ADDR         XPAR_SCUGIC_0_CPU_BASEADDR
#define INTC_DIST_BASE_ADDR    XPAR_SCUGIC_0_DIST_BASEADDR
#define TIMER_IRPT_INTR        XPAR_SCUTIMER_INTR
#define PLATFORM_EMAC_BASEADDR XPAR_XEMACPS_0_BASEADDR

#define RESET_RX_CNTR_LIMIT	400
struct netif server_netif;

void txrx_loop(void)
{
	if (TcpFastTmrFlag) {
		tcp_fasttmr();
		TcpFastTmrFlag = 0;
	}
	if (TcpSlowTmrFlag) {
		tcp_slowtmr();
		TcpSlowTmrFlag = 0;
	}
	xemacif_input(netif);
}
static int ResetRxCntr = 0;

void timer_callback(XScuTimer * TimerInstance)
{
	/* we need to call tcp_fasttmr & tcp_slowtmr at intervals specified
	 * by lwIP. It is not important that the timing is absoluetly accurate.
	 */
	static int odd = 1;
#if LWIP_DHCP==1
    static int dhcp_timer = 0;
#endif
	 TcpFastTmrFlag = 1;

	odd = !odd;
#ifndef USE_SOFTETH_ON_ZYNQ
	ResetRxCntr++;
#endif
	if (odd) {
		TcpSlowTmrFlag = 1;
#if LWIP_DHCP==1
		dhcp_timer++;
		dhcp_timoutcntr--;
		dhcp_fine_tmr();
		if (dhcp_timer >= 120) {
			dhcp_coarse_tmr();
			dhcp_timer = 0;
		}
#endif
	}

	/* For providing an SW alternative for the SI #692601. Under heavy
	 * Rx traffic if at some point the Rx path becomes unresponsive, the
	 * following API call will ensures a SW reset of the Rx path. The
	 * API xemacpsif_resetrx_on_no_rxdata is called every 100 milliseconds.
	 * This ensures that if the above HW bug is hit, in the worst case,
	 * the Rx path cannot become unresponsive for more than 100
	 * milliseconds.
	 */
#ifndef USE_SOFTETH_ON_ZYNQ
	if (ResetRxCntr >= RESET_RX_CNTR_LIMIT) {
		xemacpsif_resetrx_on_no_rxdata(&server_netif);
		ResetRxCntr = 0;
	}
#endif
	XScuTimer_ClearInterruptStatus(TimerInstance);
}
static XScuTimer TimerInstance;

void platform_setup_timer(void)
{
	int Status = XST_SUCCESS;
	XScuTimer_Config *ConfigPtr;
	int TimerLoadValue = 0;

	ConfigPtr = XScuTimer_LookupConfig(TIMER_DEVICE_ID);
	Status = XScuTimer_CfgInitialize(&TimerInstance, ConfigPtr,
			ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {

		printf("In %s: Scutimer Cfg initialization failed...\n",
		__func__);
		return;
	}

	Status = XScuTimer_SelfTest(&TimerInstance);
	if (Status != XST_SUCCESS) {
		printf("In %s: Scutimer Self test failed...\n",
		__func__);
		return;

	}

	XScuTimer_EnableAutoReload(&TimerInstance);
	/*
	 * Set for 250 milli seconds timeout.
	 */
	TimerLoadValue = XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ / 8;

	XScuTimer_LoadTimer(&TimerInstance, TimerLoadValue);
	return;
}

void platform_setup_interrupts(void)
{
	Xil_ExceptionInit();

	XScuGic_DeviceInitialize(INTC_DEVICE_ID);

	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
			(Xil_ExceptionHandler)XScuGic_DeviceInterruptHandler,
			(void *)INTC_DEVICE_ID);
	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	XScuGic_RegisterHandler(INTC_BASE_ADDR, TIMER_IRPT_INTR,
					(Xil_ExceptionHandler)timer_callback,
					(void *)&TimerInstance);
	/*
	 * Enable the interrupt for scu timer.
	 */
	XScuGic_EnableIntr(INTC_DIST_BASE_ADDR, TIMER_IRPT_INTR);

	return;
}
void platform_enable_interrupts()
{
	/*
	 * Enable non-critical exceptions.
	 */
	Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);
	XScuTimer_EnableInterrupt(&TimerInstance);
	XScuTimer_Start(&TimerInstance);
	return;
}
#define DEFAULT_IP_ADDRESS	"192.168.1.10"
#define DEFAULT_IP_MASK		"255.255.255.0"
#define DEFAULT_GW_ADDRESS	"192.168.1.1"

static void assign_default_ip(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{
	int err;

	printf("Configuring default IP %s \n", DEFAULT_IP_ADDRESS);

	err = inet_aton(DEFAULT_IP_ADDRESS, ip);
	if (!err)
		printf("Invalid default IP address: %d\n", err);

	err = inet_aton(DEFAULT_IP_MASK, mask);
	if (!err)
		printf("Invalid default IP MASK: %d\n", err);

	err = inet_aton(DEFAULT_GW_ADDRESS, gw);
	if (!err)
		printf("Invalid default gateway address: %d\n", err);
}

int lwip_connect(void)
{
	unsigned char mac_ethernet_address[] = {
		0x00, 0x0a, 0x35, 0x00, 0x01, 0x02 };

	netif = &server_netif;

	platform_setup_timer();
	platform_setup_interrupts();

	lwip_init();
	if (!xemac_add(netif, NULL, NULL, NULL, mac_ethernet_address,
				PLATFORM_EMAC_BASEADDR)) {
		printf("Error adding N/W interface\n");
		return 0;
	}

	netif_set_default(netif);
	platform_enable_interrupts();
	netif_set_up(netif);

	dhcp_start(netif);
	dhcp_timoutcntr = 24;
	while (((netif->ip_addr.addr) == 0) && (dhcp_timoutcntr > 0))
		xemacif_input(netif);

	if (dhcp_timoutcntr <= 0) {
		if ((netif->ip_addr.addr) == 0) {
			printf("ERROR: DHCP request timed out\n");
			assign_default_ip(&(netif->ip_addr),
					&(netif->netmask), &(netif->gw));
			return(0);
		}
	}
	return(1);
}
void hdmi_tick(int clean);
int lwip_get_update_version(char* file_version_loc,int alfa)
{
	conn_settings.use_proxy = 0;
	conn_settings.headers_done_fn = RecvHttpHeaderCallback;
	conn_settings.result_fn = HttpClientResultCallback;

	strcpy(domain_name, "shanshe.mooo.com");
	err_t error;

	received=0;
	sprintf(url,"/z3660/%s%s",alfa?"alfa/":"",file_version_loc);

	google_dnsserver.addr=0x08080808;

	dns_setserver(0, &google_dnsserver);

	error = httpc_get_file_dns(domain_name, 80, url, &conn_settings,
			RecvVersionCallback, NULL, &connection);
	if(error==0)
	{
		int timeout=5000000; // 5 seconds of timeout
		while(received==0 && timeout>0)
		{
			timeout--;
			txrx_loop(); // it executes at a rate of 250 and 500 ms
			if((timeout%1000000)==0)
				hdmi_tick(0); // 0 = roll the bar
			else
				usleep(1);
		}
		if(timeout==0)
		{
			printf("\nTimeout\n");
			hdmi_tick(2); // 2 = timeout
			return(0);
		}
		else
		{
			hdmi_tick(1); // 1 = clean
			return(1);
		}
	}
	return(0);
}
int lwip_get_update_version_scsirom(char* file_version_loc,int alfa)
{
	conn_settings.use_proxy = 0;
	conn_settings.headers_done_fn = RecvHttpHeaderCallback;
	conn_settings.result_fn = HttpClientResultCallback;

	strcpy(domain_name, "shanshe.mooo.com");
	err_t error;

	received=0;
	sprintf(url,"/z3660/%s%s",alfa?"alfa/":"",file_version_loc);

	google_dnsserver.addr=0x08080808;

	dns_setserver(0, &google_dnsserver);

	error = httpc_get_file_dns(domain_name, 80, url, &conn_settings,
			RecvVersionScsiRomCallback, NULL, &connection);
	if(error==0)
	{
		int timeout=5000000; // 5 seconds of timeout
		while(received==0 && timeout>0)
		{
			timeout--;
			txrx_loop(); // it executes at a rate of 250 and 500 ms
			if((timeout%1000000)==0)
				hdmi_tick(0); // 0 = roll the bar
			else
				usleep(1);
		}
		if(timeout==0)
		{
			printf("\nTimeout\n");
			hdmi_tick(2); // 2 = timeout
			return(0);
		}
		else
		{
			hdmi_tick(1); // 1 = clean
			return(1);
		}
	}

	return(0);
}
int lwip_get_update(char *filename_loc,int alfa)
{
	conn_settings.use_proxy = 0;
	conn_settings.headers_done_fn = RecvHttpHeaderCallback;
	conn_settings.result_fn = HttpClientResultCallback;

	strcpy(domain_name, "shanshe.mooo.com");
	err_t error;

	received=0;
	sprintf(url,"/z3660/%s%s",alfa?"alfa/":"",filename_loc);
	printf("url:\n%s\n", url);

	google_dnsserver.addr=0x08080808;

	dns_setserver(0, &google_dnsserver);

	error = httpc_get_file_dns(domain_name, 80, url, &conn_settings,
			RecvBOOTbinCallback, NULL, &connection);
	download_data.IncomingBytes = 0;
	download_data.PacketCnt = 0;

	if(error==0)
	{
		long int timeout=10000000; // 10 seconds of timeout
		while(received==0 && timeout>0)
		{
			timeout--;
			txrx_loop(); // it executes at a rate of 250 and 500 ms
			if((timeout%1000000)==0)
				hdmi_tick(0); // 0 = roll the bar
			else
				usleep(1);
		}
		if(timeout==0)
		{
			printf("\nTimeout\n");
			hdmi_tick(2); // 2 = timeout
			return(0);
		}
		else
		{
			printf("Done...\n");
			hdmi_tick(1); // 1 = clean
			return(1);
		}
	}
	return(0);
}
int lwip_get_update_scsirom(char *filename_loc,int alfa)
{
	conn_settings.use_proxy = 0;
	conn_settings.headers_done_fn = RecvHttpHeaderCallback;
	conn_settings.result_fn = HttpClientResultCallback;

	strcpy(domain_name, "shanshe.mooo.com");
	err_t error;

	received=0;
	sprintf(url,"/z3660/%s%s",alfa?"alfa/":"",filename_loc);
	printf("url:\n%s\n", url);

	google_dnsserver.addr=0x08080808;

	dns_setserver(0, &google_dnsserver);

	error = httpc_get_file_dns(domain_name, 80, url, &conn_settings,
			RecvScsiRomCallback, NULL, &connection);
	download_data.IncomingBytes = 0;
	download_data.PacketCnt = 0;

	if(error==0)
	{
		long int timeout=10000000; // 10 seconds of timeout
		while(received==0 && timeout>0)
		{
			timeout--;
			txrx_loop(); // it executes at a rate of 250 and 500 ms
			if((timeout%1000000)==0)
				hdmi_tick(0); // 0 = roll the bar
			else
				usleep(1);
		}
		if(timeout==0)
		{
			printf("\nTimeout\n");
			hdmi_tick(2); // 2 = timeout
			return(0);
		}
		else
		{
			printf("Done...\n");
			hdmi_tick(1); // 1 = clean
			return(1);
		}
	}
	return(0);
}


err_t RecvHttpHeaderCallback (httpc_state_t *connection, void *arg, struct pbuf *hdr, u16_t hdr_len, u32_t content_len) {
//   printf("RecvHttpHeaderCallback has been called at: %u\n", GetUpTimeMs());
   return ERR_OK;
}


void HttpClientResultCallback (void *arg, httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err) {
//   printf("HttpClientResultCallback has been called at: %u\n", GetUpTimeMs());
//   printf("httpc_result: %u\n", httpc_result);
//   printf("received number of bytes: %u\n", rx_content_len);
}


uint8_t *DATA;
err_t RecvBOOTbinCallback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
	if (p == NULL) {
		printf("pbuf==NULL TCP packet has arrived\n");
	} else {
		memcpy(&DATA[download_data.IncomingBytes], p->payload, p->len);
		download_data.IncomingBytes = download_data.IncomingBytes + p->len;
		download_data.PacketCnt++;
		if(download_data.PacketCnt%100==0 || download_data.IncomingBytes>download_data.filesize-5000)
			printf("Total length %ld bytes     \r", download_data.IncomingBytes);

		tcp_recved(tpcb, p->tot_len);
		pbuf_free(p);

		if (download_data.IncomingBytes >= download_data.filesize) {
			printf("\nincoming bytes (%ld) >= filesize (%ld)\n", download_data.IncomingBytes, download_data.filesize);
			printf("Last packed recieved -> closing the tcp connection...\n");
			download_data.IncomingBytes = 0;
			download_data.PacketCnt = 0;
			received=1;
			return tcp_close(tpcb);
		}
	}
	return ERR_OK;
}
err_t RecvScsiRomCallback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
	if (p == NULL) {
		printf("pbuf==NULL TCP packet has arrived\n");
	} else {
		memcpy(&DATA[download_data.IncomingBytes], p->payload, p->len);
		download_data.IncomingBytes = download_data.IncomingBytes + p->len;
		download_data.PacketCnt++;
		if(download_data.PacketCnt%100==0 || download_data.IncomingBytes>download_data.filesize_scsirom-5000)
			printf("Total length %ld bytes     \r", download_data.IncomingBytes);

		tcp_recved(tpcb, p->tot_len);
		pbuf_free(p);

		if (download_data.IncomingBytes >= download_data.filesize_scsirom) {
			printf("\nincoming bytes (%ld) >= filesize (%ld)\n", download_data.IncomingBytes, download_data.filesize_scsirom);
			printf("Last packed recieved -> closing the tcp connection...\n");
			download_data.IncomingBytes = 0;
			download_data.PacketCnt = 0;
			received=1;
			return tcp_close(tpcb);
		}
	}
	return ERR_OK;
}
uint32_t hextoi(char *str);
err_t RecvVersionCallback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
	if (p == NULL) {
		printf("pbuf==NULL TCP packet has arrived\n");
	} else {
		printf("Packet received\n");

		memcpy(&version_string, p->payload, p->len);
		char *version=strstr(version_string,"version=");
		if(version!=NULL && version==version_string) // version is in the downloaded text, and also is the first work
		{
			version=version_string+8;
			char *plen=strchr(version_string,'\r');
			*plen=0;
			printf("Version: %s\n", version);
			strcpy(version_string_export,version);
			*plen='\r';
			plen=strstr(version_string,"length=");
			char *pcheck=strchr(plen,'\r');
			*pcheck=0;
			download_data.filesize=atoi(plen+7);
			printf("File size: %ld\n",download_data.filesize);
			*pcheck='\r';
			pcheck=strstr(version_string,"checksum32=");
			char *last=strchr(pcheck,'\r');
			*last=0;
			download_data.checksum32=hextoi(pcheck+11);

			tcp_recved(tpcb, p->tot_len);
			pbuf_free(p);
			received=1;
		}
		else
		{
			printf("The update server is down.\nPlease let us know about this on the z3660 discord channel.\n");
		}
        return tcp_close(tpcb);
    }
    return ERR_OK;
}
err_t RecvVersionScsiRomCallback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
	if (p == NULL) {
		printf("pbuf==NULL TCP packet has arrived\n");
	} else {
		printf("Packet received\n");

		memcpy(&version_string, p->payload, p->len);
		char *version=strstr(version_string,"version=");
		if(version!=NULL && version==version_string) // version is in the downloaded text, and also is the first work
		{
			version=version_string+8;
			char *plen=strchr(version_string,'\r');
			*plen=0;
			printf("Version: %s\n", version);
			strcpy(version_scsirom_string_export,version);
			*plen='\r';
			plen=strstr(version_string,"length=");
			char *pcheck=strchr(plen,'\r');
			*pcheck=0;
			download_data.filesize_scsirom=atoi(plen+7);
			printf("File size: %ld\n",download_data.filesize_scsirom);
			*pcheck='\r';
			pcheck=strstr(version_string,"checksum32=");
			char *last=strchr(pcheck,'\r');
			*last=0;
			download_data.checksum32_scsirom=hextoi(pcheck+11);

			tcp_recved(tpcb, p->tot_len);
			pbuf_free(p);
			received=1;
		}
		else
		{
			printf("The update server is down.\nPlease let us know about this on the z3660 discord channel.\n");
		}
        return tcp_close(tpcb);
    }
    return ERR_OK;
}
/*
err_t RecvVersionScsiRomVersionCallback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
	if (p == NULL) {
		printf("pbuf==NULL TCP packet has arrived\n");
	} else {
		printf("Packet received\n");

		memcpy(&version_string, p->payload, p->len);
		int i=0;
		do{
			if(version_string[i]=='$')
			{
				if(version_string[i+1]=='V' && version_string[i+2]=='E' && version_string[i+3]=='R')
				{
					break;
				}
			}
			i++;
		}while(i<100);
		if(i==100)
		{
			printf("Can't find $VER info\n");
	        return tcp_close(tpcb);
		}
		i+=4;
		do{
			if(version_string[i]=='S')
			{
				if(version_string[i+1]=='C' && version_string[i+2]=='S' &&
				   version_string[i+3]=='I' && version_string[i+4]==' ' &&
				   version_string[i+5]=='v')
				{
					break;
				}
			}
			i++;
		}while(i<100);
		if(i==100)
		{
			printf("Can't find SCSI v info\n");
	        return tcp_close(tpcb);
		}
		i+=6;
		int j=0;
		do{
			version_scsi_string_export[j]=version_string[i++];
			if(version_scsi_string_export[j]==' ')
				break;
			j++;
		}while(j<20);
		version_scsi_string_export[j]=0;
		tcp_recved(tpcb, p->tot_len);
		pbuf_free(p);
		received=1;
        return tcp_close(tpcb);
    }
    return ERR_OK;
}

 */
