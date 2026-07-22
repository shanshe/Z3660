#include "lwip/altcp_tcp.h"
#include "lwip/debug.h"
#include "lwip/mem.h"
#include "lwip/altcp_tls.h"
#include "lwip/init.h"
#include "lwip/tcp.h"
#include "lwip/dhcp.h"
#include "lwip/inet.h"
#include "xc3_usleep.h"
#include "mobotest.h"
#include "lwip/priv/tcp_priv.h"
#include "netif/xadapter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lwip/http_client.h"
#include "xscutimer.h"
#include "xscugic.h"
#include "lwip.h"
#include "lwip/dns.h"

#ifdef USE_RTOS
#define THREAD_STACKSIZE 1024
#define DEFAULT_THREAD_PRIO 2
#include "FreeRTOS.h"
#include "task.h"
#include "lwip/sys.h"
#endif

DOWNLOAD_DATA download_data;

void print_error(err_t err);
void hdmi_tick(int clean);
void print_hdmi_ln(int xpos, char *message, int line_inc);
#ifndef USE_RTOS
void txrx_loop(void);

void lwip_run(void)
{
   txrx_loop();
}
#endif
char url[500];
char accept[40];
char domain_name[300];
long int timeout_temp;
long int timeout_init;
httpc_connection_t conn_settings;
httpc_state_t *connection;
err_t RecvHttpHeaderCallback (httpc_state_t *connection, void *arg, struct pbuf *hdr, u16_t hdr_len, u32_t content_len);
void HttpClientResultCallback (void *arg, httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err);
void HttpClientResultBOOTjsonCallback (void *arg, httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err);
err_t RecvBOOTbinCallback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err);
err_t RecvBOOTbinjsonCallback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err);
err_t RecvVersionCallback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err);
err_t RecvVersionScsiRomCallback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err);
err_t RecvVersionJedCallback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err);
err_t RecvScsiRomCallback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err);
err_t RecvJedCallback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err);
err_t RecvJsonCallback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err);

ip_addr_t google_dnsserver;
extern server_t server;

#include "lwip/dns.h"
char version_string[5000];
char version_string_export[50];
char version_scsirom_string_export[50];
char version_jed_string_export[50];
volatile int received=0;
struct netif *netif;
volatile int TcpFastTmrFlag = 0;
volatile int TcpSlowTmrFlag = 0;
void dhcp_fine_tmr();
void dhcp_coarse_tmr();
#define PLATFORM_EMAC_BASEADDR XPAR_XEMACPS_0_BASEADDR
#ifndef USE_RTOS
volatile int dhcp_timoutcntr = 24;
#define INTC_DEVICE_ID         XPAR_SCUGIC_SINGLE_DEVICE_ID
#define TIMER_DEVICE_ID        XPAR_SCUTIMER_DEVICE_ID
#define INTC_BASE_ADDR         XPAR_SCUGIC_0_CPU_BASEADDR
#define INTC_DIST_BASE_ADDR    XPAR_SCUGIC_0_DIST_BASEADDR
#define TIMER_IRPT_INTR        XPAR_SCUTIMER_INTR
#define RESET_RX_CNTR_LIMIT    400
#endif

#define HTTP_PORT 80
#define HTTPS_PORT 443
#if LWIP_ALTCP_TLS == 0
#define PORT HTTP_PORT
#else
#define PORT HTTPS_PORT
#endif

struct netif server_netif;
void printAppHeader(int port)
{
   char message[300];
   sprintf(message,"TFTP server is running on port %d", port);
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   sprintf(message,"Connect to this address on your TFTP application: %s", inet_ntoa(server_netif.ip_addr));
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
}
#ifndef USE_RTOS
uint32_t miliseconds=0;
u32_t sys_now(void)
{
   return(miliseconds);
}
void sys_check_timeouts(void);
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
   sys_check_timeouts();
}
static int ResetRxCntr = 0;

void timer_callback(XScuTimer * TimerInstance)
{
   /* we need to call tcp_fasttmr & tcp_slowtmr at intervals specified
    * by lwIP. It is not important that the timing is absoluetly accurate.
    */
   miliseconds+=250;
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
static XScuTimer TimerInstance={.IsReady=-1};

void platform_setup_timer(void)
{
   int Status = XST_SUCCESS;
   XScuTimer_Config *ConfigPtr;
   int TimerLoadValue = 0;

   if((long int)TimerInstance.IsReady==-1)
   {
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
       // XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ
extern unsigned int _cpufreq;
      TimerLoadValue = _cpufreq * 1000000L / 8;

      XScuTimer_LoadTimer(&TimerInstance, TimerLoadValue);
   }
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
extern uint8_t EmacPsMAC[6];
extern char ip_address[30];
int z3660_lwip_connect(void)
{
   netif = &server_netif;

   platform_setup_timer();
   platform_setup_interrupts();
   xc3_usleep(1000000);
   lwip_init();
   if (!xemac_add(netif, NULL, NULL, NULL, EmacPsMAC,
         PLATFORM_EMAC_BASEADDR)) {
      printf("Error adding N/W interface\n");
      return 0;
   }

   printf("Mac address: %02X:%02X:%02X:%02X:%02X:%02X\n",
         EmacPsMAC[0],EmacPsMAC[1],EmacPsMAC[2],
         EmacPsMAC[3],EmacPsMAC[4],EmacPsMAC[5]);

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
   snprintf(ip_address,20,ipaddr_ntoa(&netif->ip_addr));
   char message[100];
   sprintf(message,"IP Address : %s", ip_address);
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   return(1);
}
#define TIMEOUTS_USTEPS 100 // 10 us steps
#else //USE_RTOS
extern uint8_t EmacPsMAC[6];
void print_ip(char *msg, ip_addr_t *ip)
{
   printf(msg);
   printf("%d.%d.%d.%d\n\r", ip4_addr1(ip), ip4_addr2(ip),
         ip4_addr3(ip), ip4_addr4(ip));
}
void print_ip_settings(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{
   print_ip("Board IP: ", ip);
   print_ip("Netmask : ", mask);
   print_ip("Gateway : ", gw);
}
void network_thread(void *p)
{
   /* the mac address of the board. this should be unique per board */
   ip_addr_t ipaddr, netmask, gw;
   int mscnt = 0;

   netif = &server_netif;

   ipaddr.addr = 0;
   gw.addr = 0;
   netmask.addr = 0;
   printf("Starting network_thread\n");

   /* Add network interface to the netif_list, and set it as default */
   if (!xemac_add(netif, &ipaddr, &netmask, &gw, EmacPsMAC,
         PLATFORM_EMAC_BASEADDR)) {
      printf("Error adding N/W interface\n");
      return;
   }

   printf("Mac address: %02X:%02X:%02X:%02X:%02X:%02X\n",
         EmacPsMAC[0],EmacPsMAC[1],EmacPsMAC[2],
         EmacPsMAC[3],EmacPsMAC[4],EmacPsMAC[5]);

   netif_set_default(netif);

   /* specify that the network if is up */
   netif_set_up(netif);

   /* start packet receive thread - required for lwIP operation */
   sys_thread_new("xemacif_input_thread", (void(*)(void*))xemacif_input_thread, netif,
         THREAD_STACKSIZE,
         DEFAULT_THREAD_PRIO+2);

   dhcp_start(netif);
   while (1) {
      vTaskDelay(DHCP_FINE_TIMER_MSECS / portTICK_RATE_MS);
      dhcp_fine_tmr();
      mscnt += DHCP_FINE_TIMER_MSECS;
      if (mscnt >= DHCP_COARSE_TIMER_SECS*1000) {
         dhcp_coarse_tmr();
         mscnt = 0;
      }
   }
   return;
}

int z3660_lwip_connect(void)
{
   int mscnt = 0;
   printf("Starting lwip_init\n");
   lwip_init();
   printf("Finished lwip_init\n");
   sys_thread_new("NW_THRD", network_thread, NULL,
         THREAD_STACKSIZE,
         DEFAULT_THREAD_PRIO);
   while (1) {
      vTaskDelay(DHCP_FINE_TIMER_MSECS / portTICK_RATE_MS);
      if (server_netif.ip_addr.addr) {
         printf("DHCP request success\n");
         print_ip_settings(&(server_netif.ip_addr), &(server_netif.netmask), &(server_netif.gw));
         return(1);
      }
      mscnt += DHCP_FINE_TIMER_MSECS;
      if (mscnt >= DHCP_COARSE_TIMER_SECS * 2000) {
         printf("ERROR: DHCP request timed out\n");
         return(0);
      }
   }
   return(0);
}
#define TIMEOUTS_USTEPS 100000 // 100000 us (10ms) steps
#endif
#define TIMEOUTS 5   // 5 seconds
#define TIMEOUTS_TOTAL_US ((TIMEOUTS*1000000L)/TIMEOUTS_USTEPS)
#define TIMEOUTS_1S_US ((1*1000000L)/TIMEOUTS_USTEPS)
void dns_setserver(u8_t numdns, const ip_addr_t *dnsserver);
int z3660_lwip_get(char * file, int alfa, void* callback,long int timeout, int get_json)
{
   conn_settings.use_proxy = 0;
   conn_settings.headers_done_fn = RecvHttpHeaderCallback;
   conn_settings.result_fn = HttpClientResultCallback;
   download_data.IncomingBytes = 0;
   download_data.PacketCnt = 0;
   err_t error;
   received=0;

   if(server==SHANSHE_SERVER)
   {
      strcpy(domain_name, "shanshe.mooo.com");
      sprintf(url,"/z3660/%s%s",alfa?"alfa/":"",file);
      sprintf(accept,"*/*");
      get_json=0;
   }
   else //if(server==GITHUB_SERVER)
   {
// https://api.github.com/repos/shanshe/Z3660/contents/update_system_files/alfa/BOOT.BIN
      strcpy(domain_name, "api.github.com");
      sprintf(url,"/repos/shanshe/Z3660/contents/update_system_files/%s%s",alfa?"alfa/":"",file);

// https://raw.githubusercontent.com/shanshe/Z3660/main/update_system_files/alfa/BOOT.BIN
//      strcpy(domain_name, "raw.githubusercontent.com"); // This doesn't respond...
//      sprintf(url,"/shanshe/Z3660/main/update_system_files/%s%s",alfa?"alfa/":"",file);
      if(get_json)
         sprintf(accept,"application/json");
      else
         sprintf(accept,"application/vnd.github.v3.raw");
   }
//   char message[600];
//   sprintf(message,"url: https://%s%s",domain_name,url);
//   print_hdmi_ln(0,message,1);
//   printf("%s\n",message);

   google_dnsserver.addr=0x08080808;
   dns_setserver(0, &google_dnsserver);
   error = httpc_get_file_dns(domain_name, PORT, url, accept, &conn_settings,
         callback, NULL, &connection);
   timeout_temp=timeout;
   timeout_init=timeout;
   if(error==ERR_OK)
   {
      while((received==0) && (timeout_temp>0))
      {
         timeout_temp--;
#ifndef USE_RTOS
         txrx_loop(); // it executes at a rate of 250 and 500 ms
         if((timeout_temp%TIMEOUTS_1S_US)==0)
            hdmi_tick(0); // 0 = roll the bar
         xc3_usleep(TIMEOUTS_USTEPS);
#else
         vTaskDelay(1000/portTICK_PERIOD_MS);//
         hdmi_tick(0); // 0 = roll the bar
#endif
      }
      if((received==0) && (timeout_temp==0))
      {
//         printf("\nTimeout\n");
         hdmi_tick(2); // 2 = timeout
         return(0);
      }
      else
      {
         hdmi_tick(1); // 1 = clean
         if(get_json)
            goto getjson;
         return(1);
      }
   }
   else
   {
      char message[50];
      sprintf(message,"Error httpc_get_file_dns %d",error);
      print_hdmi_ln(0,message,1);
//      printf("%s\n",message);
      print_error(error);
   }

   return(0);
getjson:
// "git_url": "https://api.github.com/
//printf("%s\n",DATA);
   ;
   char *data=strstr((char *)DATA,"git_url");
   if(data!=NULL)
   {
      data=data+8;
      data=strstr(data,"https://api.github.com/");
      if(data!=NULL)
      {
         data=data+22;
         char *end=strstr(data,"\"");
         end[0]=0;
         sprintf(url,"%s",data);
         sprintf(accept,"application/vnd.github.raw");
//         printf("url %s\n",url);
      }
      else
      {
         printf("\" not found!!!\n");
         return(0);
      }
   }
   else
   {
      printf("git_url not found!!!\n");
      return(0);
   }

   char message[600];
   sprintf(message,"url: https://%s%s",domain_name,url);
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);

   download_data.IncomingBytes = 0;
   download_data.PacketCnt = 0;
   received=0;
   conn_settings.result_fn = HttpClientResultBOOTjsonCallback;

   google_dnsserver.addr=0x08080808;
   dns_setserver(0, &google_dnsserver);
   error = httpc_get_file_dns(domain_name, PORT, url, accept, &conn_settings,
         RecvBOOTbinjsonCallback, NULL, &connection);
   timeout_temp=timeout;
   timeout_init=timeout;
   if(error==ERR_OK)
   {
      while((received==0) && (timeout_temp>0))
      {
         timeout_temp--;
#ifndef USE_RTOS
         txrx_loop(); // it executes at a rate of 250 and 500 ms
         if((timeout_temp%TIMEOUTS_1S_US)==0)
            hdmi_tick(0); // 0 = roll the bar
         xc3_usleep(TIMEOUTS_USTEPS);
#else
         vTaskDelay(1000/portTICK_PERIOD_MS);
         hdmi_tick(0); // 0 = roll the bar
#endif
      }
      if((received==0) && (timeout_temp==0))
      {
//         printf("\nTimeout\n");
         hdmi_tick(2); // 2 = timeout
         return(0);
      }
      else
      {
         hdmi_tick(1); // 1 = clean
         return(1);
      }
   }
   else
   {
      char message[50];
      sprintf(message,"Error httpc_get_file_dns %d",error);
      print_hdmi_ln(0,message,1);
//      printf("%s\n",message);
      print_error(error);
   }
   return(0);
}
int lwip_get_update_version(char* file,int alfa)
{
   return(z3660_lwip_get(file, alfa, RecvVersionCallback, TIMEOUTS_TOTAL_US, 0)); // timeout 5 seconds
}
int lwip_get_update_version_scsirom(char* file,int alfa)
{
   return(z3660_lwip_get(file, alfa, RecvVersionScsiRomCallback, TIMEOUTS_TOTAL_US, 0)); // timeout 5 seconds
}
int lwip_get_update_version_jed(char* file,int alfa)
{
   return(z3660_lwip_get(file, alfa, RecvVersionJedCallback, TIMEOUTS_TOTAL_US, 0)); // timeout 5 seconds
}
int lwip_get_update(char *file,int alfa)
{
   if(server==SHANSHE_SERVER)
      return(z3660_lwip_get(file, alfa, RecvBOOTbinCallback, TIMEOUTS_TOTAL_US*20/5, 0)); // timeout 20 seconds
   else
      return(z3660_lwip_get(file, alfa, RecvJsonCallback, TIMEOUTS_TOTAL_US*20/5, 1)); // timeout 20 seconds
}
int lwip_get_update_scsirom(char *file,int alfa)
{
   return(z3660_lwip_get(file, alfa, RecvScsiRomCallback, TIMEOUTS_TOTAL_US, 0)); // timeout 5 seconds
}
int lwip_get_update_jed(char *file,int alfa)
{
   return(z3660_lwip_get(file, alfa, RecvJedCallback, TIMEOUTS_TOTAL_US, 0)); // timeout 5 seconds
}

err_t RecvHttpHeaderCallback (httpc_state_t *connection, void *arg, struct pbuf *hdr, u16_t hdr_len, u32_t content_len) {
   (void)connection;
   (void)arg;
   (void)hdr;
   (void)hdr_len;
   (void)content_len;
//   printf("RecvHttpHeaderCallback has been called at: %u\n", 0);//GetUpTimeMs());
//   printf("\n----cut here-------\n%s\n----cut here-------\n",(char *)hdr->payload);
   return ERR_OK;
}
void HttpClientResultCallback (void *arg, httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err) {
   (void)arg;
   (void)srv_res;
   (void)err;
   const char str_table[][60]={
         "HTTPC_RESULT_OK",
         "HTTPC_RESULT_ERR_UNKNOWN",
         "Connection to server failed",
         "Failed to resolve server hostname",
         "Connection unexpectedly closed by remote server",
         "Connection timed out (server didn't respond in time)",
         "Server responded with an error code",
         "Local memory error",
         "Local abort",
         "Content length mismatch"
   };
//   printf("HttpClientResultCallback has been called at: %u\n", 0);//GetUpTimeMs());
   printf("httpc_result: %u %s\n", httpc_result, str_table[httpc_result]);
   printf("received number of bytes: %u\n", rx_content_len);
}

typedef struct {
    uint32_t chunk_size;      // tamaño actual del chunk
    uint32_t chunk_read;      // bytes leídos del chunk actual
    uint8_t state;            // 0=leer tamaño,1=leer chunk,2=leer \r\n final,3=fin
} chunked_parser_t;
chunked_parser_t CP;

void chunked_parser_init(chunked_parser_t *p) {
    p->chunk_size = 0;
    p->chunk_read = 0;
    p->state = 0;
}

long int chunked_parser_feed(chunked_parser_t *p, const uint8_t *input, long int input_len, uint8_t *outbuf) {
    long int in_pos = 0;
    long int out_pos = 0;

    while (in_pos < input_len) {
        if (p->state == 0) {
            // Read a line with the chunk size in hexadecimal (finished with \r\n)
            static char size_line[32];
            static long int size_line_pos = 0;

            while (in_pos < input_len) {
                char c = input[in_pos++];
                if (c == '\r') continue; // ignore \r
                if (c == '\n') {
                    size_line[size_line_pos] = 0;
                    p->chunk_size = (uint32_t)strtoul(size_line, NULL, 16);
                    size_line_pos = 0;
                    if (p->chunk_size == 0) {
                        p->state = 3; // end
                        return out_pos;
                    }
                    p->chunk_read = 0;
                    p->state = 1; // read chunk
                    break;
                }
                if (size_line_pos < (int)(sizeof(size_line) - 1)) {
                    size_line[size_line_pos++] = c;
                } else {
                    return -1; // line too long
                }
            }
        } else if (p->state == 1) {
            // Read chunk bytes
            int32_t to_read = p->chunk_size - p->chunk_read;
            int32_t can_read = input_len - in_pos;
            if (can_read > to_read) can_read = to_read;

            memcpy(outbuf + out_pos, input + in_pos, can_read);
            out_pos += can_read;
            in_pos += can_read;
            p->chunk_read += can_read;

            if (p->chunk_read == p->chunk_size) {
                p->state = 2; // read final \r\n
            }
        } else if (p->state == 2) {
            // Read final chunk \r\n
            if (input[in_pos] == '\r') {
                in_pos++;
            }
            if (in_pos < input_len && input[in_pos] == '\n') {
                in_pos++;
                p->state = 0; // next chunk
            } else {
                return -1; // invalid format
            }
        } else if (p->state == 3) {
            // fin
            break;
        }
    }
    return out_pos;
}

void HttpClientResultBOOTjsonCallback (void *arg, httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err) {
   (void)arg;
   (void)httpc_result;
   (void)srv_res;
   (void)err;
   if (rx_content_len >= download_data.filesize) {
      printf("\nIncoming bytes (%ld) >= filesize (%ld)\n", download_data.IncomingBytes, download_data.filesize);
      uint8_t *DATA2=malloc(download_data.filesize);
      chunked_parser_init(&CP);
      long int ret = chunked_parser_feed(&CP, DATA, download_data.IncomingBytes, DATA2);
      if(ret!=(int32_t)download_data.filesize)
         printf("ret != download_data.filesize %ld != %ld\n",ret,download_data.filesize);
/*
      uint32_t offset=0,index=0;
      for(long int i=0,k=0;i<download_data.filesize;i++,k++)
      {
         if(k==offset)
         {
            k=0;
            if(DATA[i+index]=='\r') index++;
            if(DATA[i+index]=='\n') index++;
            if(DATA[i+index]=='\r') index++;
            if(DATA[i+index]=='\n') index++;
            offset=0;
            while(DATA[i+index]!='\r' && DATA[i+index]!='\n')
            {
               if(DATA[i+index]>='0' && DATA[i+index]<='9')
                  offset=(offset<<4)+(uint32_t)(DATA[i+index]-'0');
               else if(DATA[i+index]>='A' && DATA[i+index]<='F')
                  offset=(offset<<4)+(uint32_t)(DATA[i+index]-'A'+10);
               else if(DATA[i+index]>='a' && DATA[i+index]<='f')
                  offset=(offset<<4)+(uint32_t)(DATA[i+index]-'a'+10);
               index++;
            }
            if(DATA[i+index]=='\r') index++;
            if(DATA[i+index]=='\n') index++;
            if(DATA[i+index]=='\r') index++;
            if(DATA[i+index]=='\n') index++;
//               printf("offset %08lX\n",offset);
         }
         DATA2[i]=DATA[i+index];
      }
*/
      memcpy(DATA,DATA2,download_data.filesize);
      free(DATA2);

      download_data.IncomingBytes = 0;
      download_data.PacketCnt = 0;
      received=1;
   }

}
void print_error(err_t err)
{
   switch(err)
   {
   case ERR_MEM:
      printf("Out of memory error\n");
      break;
   case ERR_BUF:
      printf("Buffer error\n");
      break;
   case ERR_TIMEOUT:
      printf("Timeout\n");
      break;
   case ERR_RTE:
      printf("Routing problem\n");
      break;
   case ERR_INPROGRESS:
      printf("Operation in progress\n");
      break;
   case ERR_VAL:
      printf("Illegal value\n");
      break;
   case ERR_WOULDBLOCK:
      printf("Operation would block\n");
      break;
   case ERR_USE:
      printf("Address in use\n");
      break;
   case ERR_ALREADY:
      printf("Already connecting\n");
      break;
   case ERR_ISCONN:
      printf("Conn already established\n");
      break;
   case ERR_CONN:
      printf("Not connected\n");
      break;
   case ERR_IF:
      printf("Low-level netif error\n");
      break;
   case ERR_ABRT:
      printf("Connection aborted\n");
      break;
   case ERR_RST:
      printf("Connection reset\n");
      break;
   case ERR_CLSD:
      printf("Connection closed\n");
      break;
   case ERR_ARG:
      printf("Illegal argument\n");
      break;
   default:
      printf("Error %d not defined\n",err);
      break;
   }
}

uint8_t *DATA;
err_t genericRecvCallback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err, uint32_t filesize) {
   (void)arg;
   if(err!=ERR_OK) print_error(err);
   if (p == NULL) {
      printf("pbuf==NULL TCP packet has arrived\n");
   } else {
      timeout_temp=timeout_init;
#ifdef USE_RTOS
      hdmi_tick(0); // 0 = roll the bar
#endif
      memcpy(&DATA[download_data.IncomingBytes], p->payload, p->len);
      download_data.IncomingBytes = download_data.IncomingBytes + p->len;
      download_data.PacketCnt++;
      if(download_data.PacketCnt%2==0 || download_data.IncomingBytes>filesize-5000)
         printf("Total length %ld bytes     \r", download_data.IncomingBytes);

      altcp_recved(tpcb, p->tot_len);
      pbuf_free(p);

      if (download_data.IncomingBytes >= filesize) {
         printf("\nIncoming bytes (%ld) >= filesize (%ld)\n", download_data.IncomingBytes, filesize);
         printf("Last packed received -> closing the tcp connection...\n");
         download_data.IncomingBytes = 0;
         download_data.PacketCnt = 0;
         received=1;
         return altcp_close(tpcb);
      }
   }
   return ERR_OK;
}
err_t RecvBOOTbinjsonCallback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err) {
   (void)arg;
   if(err!=ERR_OK) print_error(err);
   if (p == NULL) {
      printf("pbuf==NULL TCP packet has arrived\n");
   } else {
      timeout_temp=timeout_init;
#ifdef USE_RTOS
      hdmi_tick(0); // 0 = roll the bar
#endif
      memcpy(&DATA[download_data.IncomingBytes], p->payload, p->len);
      download_data.IncomingBytes = download_data.IncomingBytes + p->len;
      download_data.PacketCnt++;
//      if(download_data.PacketCnt%2==0 || download_data.IncomingBytes>download_data.filesize-5000)
         printf("Total length %ld bytes     \r", download_data.IncomingBytes);

      altcp_recved(tpcb, p->tot_len);
      pbuf_free(p);
      // continue to download the rest of data. Finish on timeout
   }
   return ERR_OK;
}
err_t RecvBOOTbinCallback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err) {
   return(genericRecvCallback(arg,tpcb,p,err,download_data.filesize));
}
err_t RecvScsiRomCallback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err) {
   return(genericRecvCallback(arg,tpcb,p,err,download_data.filesize_scsirom));
}
err_t RecvJedCallback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err) {
   return(genericRecvCallback(arg,tpcb,p,err,download_data.filesize_jed));
}

uint32_t hextoi(char *str);
err_t genericRecvVersionCallback(uint32_t *filesize,uint32_t *checksum32,char *ver_string_export,void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err) {
   (void)arg;
   if(err!=ERR_OK) print_error(err);
   if (p == NULL) {
      printf("pbuf==NULL TCP packet has arrived\n");
   } else {
//      printf("Packet received\n");
      timeout_temp=timeout_init;
#ifdef USE_RTOS
      hdmi_tick(0); // 0 = roll the bar
#endif
      memcpy(&version_string, p->payload, p->len);
      char *version=strstr(version_string,"version=");
      if(version!=NULL && version==version_string) // version is in the downloaded text, and also is the first work
      {
         version=version_string+8;
         char *plen=strchr(version_string,'\r');
         *plen=0;
         printf("Version: %s\n", version);
         strcpy(ver_string_export,version);
         *plen='\r';
         plen=strstr(version_string,"length=");
         char *pcheck=strchr(plen,'\r');
         *pcheck=0;
         *filesize=atoi(plen+7);
         printf("File size: %ld\n",*filesize);
         *pcheck='\r';
         pcheck=strstr(version_string,"checksum32=");
         char *last=strchr(pcheck,'\r');
         *last=0;
         *checksum32=hextoi(pcheck+11);

         altcp_recved(tpcb, p->tot_len);
         pbuf_free(p);
         received=1;
      }
      else
      {
         printf("The update server may be down.\nPlease let us know about this on the z3660 discord channel.\n");
      }
      return altcp_close(tpcb);
   }
   return ERR_OK;
}
err_t RecvVersionCallback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err) {
   return (genericRecvVersionCallback(&download_data.filesize,&download_data.checksum32,version_string_export,arg, tpcb, p, err));
}
err_t RecvVersionScsiRomCallback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err) {
   return (genericRecvVersionCallback(&download_data.filesize_scsirom,&download_data.checksum32_scsirom,version_scsirom_string_export,arg, tpcb, p, err));
}
err_t RecvVersionJedCallback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err) {
   return (genericRecvVersionCallback(&download_data.filesize_jed,&download_data.checksum32_jed,version_jed_string_export,arg, tpcb, p, err));
}

err_t genericRecvJsonCallback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err) {
   (void)arg;
   if(err!=ERR_OK) print_error(err);
   if (p == NULL) {
      printf("pbuf==NULL TCP packet has arrived\n");
   } else {
//      printf("Packet received\n");
#ifdef USE_RTOS
      hdmi_tick(0); // 0 = roll the bar
#endif
      memcpy(&DATA[download_data.IncomingBytes], p->payload, p->len);
      download_data.IncomingBytes = download_data.IncomingBytes + p->len;
      char *data=strstr((char *)DATA,"_links");
      if(data)
      {
         DATA[download_data.IncomingBytes]=0;
         altcp_recved(tpcb, p->tot_len);
         pbuf_free(p);
         received=1;
         return altcp_close(tpcb);
      }
   }
   return ERR_OK;
}
err_t RecvJsonCallback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err) {
   return (genericRecvJsonCallback(arg, tpcb, p, err));
}
