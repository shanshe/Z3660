/*
 * Improved USB Mouse Test for Z3660
 * Based on HID report descriptor analysis with proper buffer alignment
 */

#include <stdio.h>
#include <string.h>
#include "usb.h"
#include "../main.h"
#include "sleep.h"

// Function declarations
static void test_mouse_manual_polling(struct usb_device *mouse_dev, int endpoint, int maxpacketsize);

/*
 * Manual polling fallback when interrupt queues fail
 * This reads from the endpoint directly without interrupt queues
 */
static void test_mouse_manual_polling(struct usb_device *mouse_dev, int endpoint, int maxpacketsize)
{
    printf("\n[MOUSE MANUAL] === Manual Polling Fallback ===\n");
    printf("[MOUSE MANUAL] Reading directly from endpoint %d without interrupts\n", endpoint);
    
    uint8_t buffer[8];
    int data_received = 0;
    int timeout_ms = 50;
    
    for (int i = 0; i < 30; i++) { // Try 30 times
        int ret = usb_int_msg(NULL, mouse_dev, usb_rcvintpipe(mouse_dev, endpoint),
                             buffer, maxpacketsize, timeout_ms);
        
        if (ret > 0) {
            data_received++;
            printf("[MOUSE MANUAL] %03d: Data received (%d bytes): ", i + 1, ret);
            for (int j = 0; j < ret && j < 8; j++) {
                printf("%02X ", buffer[j]);
            }
            printf("\n");
            
            // Check if this is actual mouse movement data
            if (buffer[0] & 0x01) { // Check if left button pressed or movement detected
                int8_t dx = (int8_t)buffer[1];
                int8_t dy = (int8_t)buffer[2];
                printf("[MOUSE MANUAL] MOVEMENT: dx=%d, dy=%d\n", dx, dy);
            }
        } else if (ret == 0) {
            printf("[MOUSE MANUAL] %03d: No data (timeout)\n", i + 1);
        } else {
            printf("[MOUSE MANUAL] %03d: Error reading data (ret=%d)\n", i + 1, ret);
        }
        
        usleep(100000); // 100ms delay between polls
    }
    
    printf("[MOUSE MANUAL] Manual polling completed. Total packets: %d\n", data_received);
}

#include <xil_cache.h>

#define min(x, y) ((x) < (y) ? (x) : (y))

/* Configurable test parameters */
#define MOUSE_INT_QUEUE_SIZE    64  /* Single TD is sufficient for mouse polling */
#define MOUSE_TEST_ITERATIONS   100

// Based on HID Report Descriptor analysis:
// Report ID 0x11: Mouse report with 1 byte buttons + 3 bytes X,Y,Wheel + additional scroll
struct usb_mouse_report_with_id {
   uint8_t report_id;    // Should be 0x11 for mouse
   uint8_t buttons;      // 5 bits buttons + 3 bits padding
   int8_t x;             // X movement
   int8_t y;             // Y movement
   int8_t wheel;         // Wheel movement
   int8_t h_scroll;      // Horizontal scroll (based on descriptor)
} __attribute__((aligned(USB_DMA_MINALIGN)));

// Alternative format without report ID (for Boot Protocol)
struct usb_mouse_report_boot {
   uint8_t buttons;
   int8_t x;
   int8_t y;
   int8_t wheel;
} __attribute__((aligned(USB_DMA_MINALIGN)));

struct int_queue *create_int_queue(struct usb_device *dev,
      unsigned long pipe, int queuesize, int elementsize,
      void *buffer, int interval);
void *poll_int_queue(struct usb_device *dev, struct int_queue *queue);
int destroy_int_queue(struct usb_device *dev, struct int_queue *queue);

static struct usb_device *find_mouse_device(int *mouse_interface_num)
{
   struct usb_device *dev;
   int i, j;

   printf("[MOUSE TEST] Scanning for HID devices...\n");

   for (i = 0; i < USB_MAX_DEVICE; i++) {
      dev = usb_get_dev_index(i);
      if (dev == NULL)
         continue;
         
      printf("[MOUSE TEST] Device %d with %d interfaces\n", i, dev->config.desc.bNumInterfaces);
      
      // Check all interfaces of this device
      for (j = 0; j < dev->config.desc.bNumInterfaces; j++) {
         printf("[MOUSE TEST] Device %d Interface %d: Class=%d, SubClass=%d, Protocol=%d\n",
                i, j, dev->config.if_desc[j].desc.bInterfaceClass,
                dev->config.if_desc[j].desc.bInterfaceSubClass,
                dev->config.if_desc[j].desc.bInterfaceProtocol);

         // Look for HID class devices
         if (dev->config.if_desc[j].desc.bInterfaceClass == USB_CLASS_HID) {
            // Accept mouse protocol or generic HID (some mice don't report mouse protocol)
            if (dev->config.if_desc[j].desc.bInterfaceProtocol == USB_PROT_HID_MOUSE ||
                  dev->config.if_desc[j].desc.bInterfaceProtocol == USB_PROT_HID_NONE) {
               printf("[MOUSE TEST] Found potential mouse device at index %d interface %d\n", i, j);
               *mouse_interface_num = j;
               return dev;
            }
         }
      }
   }
   return NULL;
}

static void test_boot_protocol(struct usb_device *mouse_dev, int interface_num, int endpoint, unsigned int maxpacketsize, int interval)
{
    void *int_queue;
    void *buffer;
    int i;
    struct usb_mouse_report_boot report __attribute__((aligned(USB_DMA_MINALIGN)));

    printf("\n[MOUSE TEST] === Testing Boot Protocol ===\n");

    // Set boot protocol
    printf("[MOUSE TEST] Setting Boot Protocol for interface %d...\n", interface_num);
    usb_set_protocol(mouse_dev, interface_num, 0);
    usb_set_idle(mouse_dev, interface_num, 0, 0);

    // Use the actual mouse interval for proper timing
    int actual_interval = interval;
    printf("[MOUSE TEST] Creating interrupt queue (Boot Protocol)...\n");
    printf("[MOUSE TEST] Using polling interval: %d ms (device specification)\n", 
           actual_interval);

    printf("[MOUSE TEST] Activating endpoint %d for interrupts...\n", endpoint);
    
    // Force interrupt activation by trying to read endpoint configuration
    printf("[MOUSE TEST] Checking endpoint configuration...\n");

    int_queue = create_int_queue(mouse_dev, usb_rcvintpipe(mouse_dev, endpoint),
                                8, maxpacketsize, NULL, actual_interval);

    if (!int_queue) {
        printf("[MOUSE TEST] ❌ Failed to create interrupt queue - interrupts may not be working\n");
        printf("[MOUSE TEST] Trying manual polling method...\n");
        
        // Fallback to manual polling without interrupt queue
        test_mouse_manual_polling(mouse_dev, endpoint, maxpacketsize);
        return;
    }
    printf("[MOUSE TEST] ✅ Interrupt queue created successfully\n");

    printf("[MOUSE TEST] Reading mouse reports (Boot Protocol) for 3 seconds...\n");

    // Clear any stale data first
    for (i = 0; i < 5; i++) {
        buffer = poll_int_queue(mouse_dev, int_queue);
        if (!buffer) break;
        usleep(10000); // 10ms
    }

    for (i = 0; i < 30; i++) {
        buffer = poll_int_queue(mouse_dev, int_queue);

        if (buffer) {
            // Debug: show raw buffer data
            uint8_t *raw = (uint8_t *)buffer;
            printf("[%03d] BOOT Raw: %02x %02x %02x %02x %02x %02x (addr=%p)\n", 
                   i, raw[0], raw[1], raw[2], raw[3], raw[4], raw[5], buffer);

            // Clear buffer after reading to detect stale data
            memcpy(&report, buffer, min(maxpacketsize, sizeof(report)));
            memset(buffer, 0xFF, maxpacketsize); // Clear with 0xFF to make stale data obvious

            // Parse boot protocol format
            printf("[%03d] BOOT Mouse: X=%3d, Y=%3d, Buttons=0x%02x, Wheel=%3d\n",
                   i, report.x, report.y, report.buttons, report.wheel);

        } else {
            if (i % 5 == 0) {
                printf("[%03d] BOOT No mouse data\n", i);
            }
        }

        usleep(100000); // 100ms
    }

    printf("[MOUSE TEST] Destroying Boot Protocol interrupt queue...\n");
    destroy_int_queue(mouse_dev, int_queue);
    printf("[MOUSE TEST] Boot Protocol interrupt queue destroyed successfully.\n");
    
    // Give the hardware some time to settle
    usleep(100000); // 100ms
    printf("[MOUSE TEST] Boot Protocol test completed.\n");
}

struct mouse_thread_data{
   struct usb_device *mouse_dev;
   int endpoint;
   int maxpacketsize;
   int interval;
   int reportid;
   struct usb_mouse_report_with_id *report_11;
   struct usb_mouse_report_boot *report_boot;
} mouse_thd;

void mouse_thread_run(struct mouse_thread_data *md)
{
   static void *current_queue = NULL;
   static int poll_count = 0;
   void *buffer;
   int data_received = 0;
   int idle=1;
   
   poll_count++;
   
   // Create MOUSE_INT_QUEUE_SIZE-element queue for each poll cycle
   if(current_queue==NULL) {
      printf("[DEBUG] Creating initial interrupt queue...\n");
      current_queue = create_int_queue(md->mouse_dev, usb_rcvintpipe(md->mouse_dev, md->endpoint),
         MOUSE_INT_QUEUE_SIZE, md->maxpacketsize, NULL, md->interval);
   }

   if (!current_queue) {
      printf("Failed to create %d-element interrupt queue\n", MOUSE_INT_QUEUE_SIZE);
      usleep(5000); // Just 5ms on failure
      return;
   }
   
   // Debug every 100 polls
   if (poll_count % 100 == 0) {
      printf("[DEBUG] Poll cycle %d, checking for data...\n", poll_count);
   }

   if (md->reportid == 0x11) {
      while ((buffer = poll_int_queue(md->mouse_dev, current_queue)) != NULL) {
         data_received++;
         md->report_11 = (struct usb_mouse_report_with_id *)buffer;
         int output=0;
         if(md->report_11->x==0 && md->report_11->y==0 && md->report_11->buttons==0 && md->report_11->wheel==0)
         {
            if(idle==0)
            {
               idle=1;
               output=1;
            }
         }
         else
         {
            idle=0;
            output=1;
         }
         if(output)
            printf("[%03d] Mouse: X=%3d, Y=%3d, Btn=0x%02x, Wheel=%3d\n",
                  data_received, md->report_11->x, md->report_11->y, md->report_11->buttons, md->report_11->wheel);
      }
   } else if (md->reportid == 0) {
      while ((buffer = poll_int_queue(md->mouse_dev, current_queue)) != NULL) {
         data_received++;
         md->report_boot = (struct usb_mouse_report_boot *)buffer;
         int output=0;
         if(md->report_boot->x==0 && md->report_boot->y==0 && md->report_boot->buttons==0 && md->report_boot->wheel==0)
         {
            if(idle==0)
            {
               idle=1;
               output=1;
            }
         }
         else
         {
            idle=0;
            output=1;
         }
         if(output)
            printf("[%03d] Mouse: X=%3d, Y=%3d, Btn=0x%02x, Wheel=%3d\n",
                  data_received, md->report_boot->x, md->report_boot->y, md->report_boot->buttons, md->report_boot->wheel);
      }
   }
   else
   {
      while ((buffer = poll_int_queue(md->mouse_dev, current_queue)) != NULL) {
         data_received++;
         uint8_t *raw=(uint8_t *)buffer;
         printf("[%03d] Raw: %02X %02X %02X %02X %02X %02X\n",
               data_received, raw[0], raw[1], raw[2], raw[3], raw[4], raw[5]);
      }
   }
   
   // Always show raw data regardless of reportid to debug what device sends
   while ((buffer = poll_int_queue(md->mouse_dev, current_queue)) != NULL) {
      data_received++;
      uint8_t *raw=(uint8_t *)buffer;
      printf("[%03d] RAW DATA: %02X %02X %02X %02X %02X %02X (reportid=0x%02x)\n",
            data_received, raw[0], raw[1], raw[2], raw[3], raw[4], raw[5], md->reportid);
   }

   // Always destroy the queue after processing all its events
   destroy_int_queue(md->mouse_dev, current_queue);
   current_queue = create_int_queue(md->mouse_dev, usb_rcvintpipe(md->mouse_dev, md->endpoint),
      MOUSE_INT_QUEUE_SIZE, md->maxpacketsize, NULL, md->interval);
}
static void test_report_protocol_limited(struct usb_device *mouse_dev, int interface_num, int endpoint, int maxpacketsize, int interval, int reportid)
{
   void *int_queue;
   void *buffer;
   int i;
   int data_received = 0;

   printf("\n[MOUSE TEST] === Testing Report Protocol (reportid=0x%02x) ===\n", reportid);

   // Set report protocol
   printf("[MOUSE TEST] Setting Report Protocol for interface %d...\n", interface_num);
   int result = usb_set_protocol(mouse_dev, interface_num, 1);  // 1 = Report Protocol
   if (result != 0) {
      printf("[MOUSE TEST] WARNING: usb_set_protocol returned error %d\n", result);
   } else {
      printf("[MOUSE TEST] usb_set_protocol successful\n");
   }
   
   result = usb_set_idle(mouse_dev, interface_num, 0, reportid);
   if (result != 0) {
      printf("[MOUSE TEST] WARNING: usb_set_idle returned error %d\n", result);
   } else {
      printf("[MOUSE TEST] usb_set_idle successful\n");
   }

   // Create interrupt queue
   printf("[MOUSE TEST] Creating interrupt queue (Report Protocol)...\n");
   int_queue = create_int_queue(mouse_dev, usb_rcvintpipe(mouse_dev, endpoint),
         MOUSE_INT_QUEUE_SIZE, maxpacketsize, NULL, interval);

   if (!int_queue) {
       printf("[MOUSE TEST] Failed to create interrupt queue!\n");
       return;
   }

   printf("[MOUSE TEST] Reading mouse reports (Report Protocol, reportid=0x%02x) for 3 seconds...\n", reportid);

   // Clear any stale data first
   for (i = 0; i < 5; i++) {
       buffer = poll_int_queue(mouse_dev, int_queue);
       if (!buffer) break;
       usleep(10000); // 10ms
   }

   int no_data_count = 0;
   int queue_recreate_count = 0;
   
   for (i = 0; i < 30; i++) {
       buffer = poll_int_queue(mouse_dev, int_queue);

       if (buffer) {
           data_received++;
           no_data_count = 0;  // Reset no-data counter
           // Debug: show raw buffer data
           uint8_t *raw = (uint8_t *)buffer;
           printf("[%03d] REPORT Raw: %02x %02x %02x %02x %02x %02x (reportid=0x%02x)\n", 
                  i, raw[0], raw[1], raw[2], raw[3], raw[4], raw[5], reportid);

           // Parse based on reportid
           if (reportid == 0x11 && raw[0] == 0x11) {
               printf("[%03d] REPORT Mouse: X=%3d, Y=%3d, Buttons=0x%02x, Wheel=%3d\n",
                      i, (int8_t)raw[2], (int8_t)raw[3], raw[1], (int8_t)raw[4]);
           } else if (reportid == 0) {
               printf("[%03d] REPORT Mouse: X=%3d, Y=%3d, Buttons=0x%02x, Wheel=%3d\n",
                      i, (int8_t)raw[1], (int8_t)raw[2], raw[0], (int8_t)raw[3]);
           }

       } else {
           no_data_count++;
           if (i % 5 == 0) {
               printf("[%03d] REPORT No mouse data\n", i);
           }
           
           // If we've had no data for 3 consecutive polls, recreate the queue
           if (no_data_count >= 3 && queue_recreate_count < 5) {
               printf("[%03d] REPORT Queue seems exhausted, recreating... (attempt %d)\n", i, queue_recreate_count + 1);
               destroy_int_queue(mouse_dev, int_queue);
               usleep(50000); // Small delay
               int_queue = create_int_queue(mouse_dev, usb_rcvintpipe(mouse_dev, endpoint),
                                          8, maxpacketsize, NULL, interval);
               if (!int_queue) {
                   printf("[%03d] REPORT Failed to recreate interrupt queue!\n", i);
                   return;
               }
               queue_recreate_count++;
               no_data_count = 0;
               printf("[%03d] REPORT Queue recreated successfully\n", i);
           }
       }

       usleep(100000); // 100ms
   }

   printf("[MOUSE TEST] Total data packets received: %d\n", data_received);
   printf("[MOUSE TEST] Destroying Report Protocol interrupt queue...\n");
   destroy_int_queue(mouse_dev, int_queue);
   printf("[MOUSE TEST] Report Protocol test completed.\n");
}
#if 0
static void test_report_protocol_fast_polling(struct usb_device *mouse_dev, int interface_num, int endpoint, int maxpacketsize, int interval, int reportid)
{
   printf("\n[MOUSE TEST] === Testing Report Protocol (%d-element Queue Fast Polling) ===\n",MOUSE_INT_QUEUE_SIZE);

   // Set report protocol
   printf("[MOUSE TEST] Setting Report Protocol for interface %d...\n", interface_num);
   int result = usb_set_protocol(mouse_dev, interface_num, 1);  // 1 = Report Protocol
   if (result != 0) {
      printf("[MOUSE TEST] WARNING: usb_set_protocol returned error %d\n", result);
   } else {
      printf("[MOUSE TEST] usb_set_protocol successful\n");
   }
   
   result = usb_set_idle(mouse_dev, interface_num, 0, reportid);
   if (result != 0) {
      printf("[MOUSE TEST] WARNING: usb_set_idle returned error %d\n", result);
   } else {
      printf("[MOUSE TEST] usb_set_idle successful\n");
   }

   printf("[MOUSE TEST] Using %d-element queues, processing all events per cycle\n",MOUSE_INT_QUEUE_SIZE);
   printf("[MOUSE TEST] Move the mouse and click buttons to see immediate responses\n");
   mouse_thd.mouse_dev=mouse_dev;
   mouse_thd.endpoint=endpoint;
   mouse_thd.maxpacketsize=maxpacketsize;
   mouse_thd.interval=interval;
   mouse_thd.reportid=reportid;

   printf("Mouse config: endpoint %d maxpacketsize %d interval %d reportid 0x%x\n",
         mouse_thd.endpoint, mouse_thd.maxpacketsize, mouse_thd.interval, mouse_thd.reportid);
   while(1) {
      mouse_thread_run(&mouse_thd);
      // Polling interval - balance between responsiveness and CPU usage
      usleep(10000); // 10ms between polling cycles
   }
   
   printf("[MOUSE TEST] %d-element queue test completed.\n",MOUSE_INT_QUEUE_SIZE);
}
#endif
void usb_mouse_test_improved(int reportid)
{
   struct usb_device *mouse_dev;
   int endpoint = 0;
   int interval = 0;
   unsigned int maxpacketsize = 0;
   int mouse_interface_num = 0;
   int i;
//Xil_DCacheDisable();
   printf("\n--- Starting Improved USB Mouse Test ---\n");

   mouse_dev = find_mouse_device(&mouse_interface_num);

   if (!mouse_dev) {
      printf("[MOUSE TEST] No USB mouse found.\n");
      return;
   }

   printf("[MOUSE TEST] USB Mouse found!\n");
   printf("[MOUSE TEST] Using interface number: %d\n", mouse_interface_num);
   printf("[MOUSE TEST] Manufacturer: %s\n", mouse_dev->mf);
   printf("[MOUSE TEST] Product: %s\n", mouse_dev->prod);
   printf("[MOUSE TEST] Serial: %s\n", mouse_dev->serial);
   printf("[MOUSE TEST] Vendor: 0x%04x, Product: 0x%04x\n",
         mouse_dev->descriptor.idVendor, mouse_dev->descriptor.idProduct);

   // Find the interrupt IN endpoint for the correct interface
   for (i = 0; i < mouse_dev->config.if_desc[mouse_interface_num].no_of_ep; i++) {
      if ((mouse_dev->config.if_desc[mouse_interface_num].ep_desc[i].bmAttributes & 0x03) == USB_ENDPOINT_XFER_INT &&
            (mouse_dev->config.if_desc[mouse_interface_num].ep_desc[i].bEndpointAddress & 0x80)) {
         endpoint = mouse_dev->config.if_desc[mouse_interface_num].ep_desc[i].bEndpointAddress & 0x0f;
         interval = mouse_dev->config.if_desc[mouse_interface_num].ep_desc[i].bInterval;
         maxpacketsize = mouse_dev->config.if_desc[mouse_interface_num].ep_desc[i].wMaxPacketSize;
         printf("[MOUSE TEST] Found interrupt endpoint: %d, interval=%d, maxpacket=%d\n",
               endpoint, interval, maxpacketsize);
         break;
      }
   }

   if (endpoint == 0) {
      printf("[MOUSE TEST] No interrupt endpoint found!\n");
      return;
   }

   // Test both protocols to see which one works better
   printf("[MOUSE TEST] Testing both HID protocols...\n");
   printf("[MOUSE TEST] Device expects 6-byte reports based on maxpacket size\n");

   // Test Boot Protocol first
//   test_boot_protocol(mouse_dev, mouse_interface_num, endpoint, maxpacketsize, interval);

   // Small delay between tests
   usleep(500000); // 500ms

   // Test Report Protocol with Report ID first (limited time)
   test_report_protocol_limited(mouse_dev, mouse_interface_num, endpoint, maxpacketsize, interval, reportid);
   // test_report_protocol_fast_polling(mouse_dev, mouse_interface_num, endpoint, maxpacketsize, interval, reportid);

   printf("--- Improved USB Mouse Test Finished ---\n");
}
