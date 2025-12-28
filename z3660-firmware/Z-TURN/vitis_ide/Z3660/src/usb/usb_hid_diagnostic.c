/*
 * USB HID Diagnostic Tools for Z3660
 * Provides detailed analysis of HID devices to troubleshoot communication issues
 */

#include <stdio.h>
#include <string.h>
#include "usb.h"
#include "../main.h"
#include "sleep.h"

#define min(x, y) ((x) < (y) ? (x) : (y))
#define HID_REQ_GET_REPORT_DESC 0x06
#define HID_REPORT_DESCRIPTOR_SIZE 256

// HID Report Descriptor parsing
static void print_hex_dump(const uint8_t *data, int len, const char *prefix)
{
    int i, j;
    for (i = 0; i < len; i += 16) {
        printf("%s%04x: ", prefix, i);
        
        // Print hex bytes
        for (j = 0; j < 16 && (i + j) < len; j++) {
            printf("%02x ", data[i + j]);
        }
        
        // Pad if less than 16 bytes
        for (; j < 16; j++) {
            printf("   ");
        }
        
        printf(" |");
        
        // Print ASCII representation
        for (j = 0; j < 16 && (i + j) < len; j++) {
            char c = data[i + j];
            printf("%c", (c >= 32 && c <= 126) ? c : '.');
        }
        
        printf("|\n");
    }
}

static int analyze_hid_report_descriptor(const uint8_t *desc, int len)
{
    int i = 0;
    int reportid=0;
    printf("[HID] Analyzing Report Descriptor (%d bytes):\n", len);
    print_hex_dump(desc, len, "[HID] ");
    
    printf("[HID] Report Descriptor Analysis:\n");
    
    while (i < len) {
        uint8_t item = desc[i];
        uint8_t type = (item >> 2) & 0x03;
        uint8_t tag = (item >> 4) & 0x0F;
        uint8_t size = item & 0x03;
        
        if (size == 3) size = 4;  // Special case
        
        const char *type_str[] = {"Main", "Global", "Local", "Reserved"};
        if(item!=0 || type!=0 || tag!=0 || size!=0)
        {
           printf("[HID]   Item %02x: Type=%s, Tag=0x%02x, Size=%d",
                 item, type_str[type], tag, size);
        
           if (size > 0) {
              printf(", Data=");
              for (int j = 1; j <= size && (i + j) < len; j++) {
                 printf("%02x ", desc[i + j]);
              }
           }
           printf("\n");
        }
        if(item==0x85 && size==1 && desc[i+1]==0x11)
        {
           reportid=0x11;
           printf("report ID=0x11\n");
        }
        
        // Parse some common items
        if (type == 1) { // Global items
            switch (tag) {
                case 0x04: // Usage Page
                    if (size >= 1) {
                        uint16_t usage_page = desc[i + 1];
                        if (size >= 2) usage_page |= (desc[i + 2] << 8);
                        printf("[HID]     -> Usage Page: 0x%04x", usage_page);
                        switch (usage_page) {
                            case 0x01: printf(" (Generic Desktop)"); break;
                            case 0x02: printf(" (Simulation Controls)"); break;
                            case 0x09: printf(" (Button)"); break;
                            default: printf(" (Unknown)"); break;
                        }
                        printf("\n");
                    }
                    break;
                case 0x05: // Logical Minimum
                    printf("[HID]     -> Logical Minimum\n");
                    break;
                case 0x06: // Logical Maximum
                    printf("[HID]     -> Logical Maximum\n");
                    break;
                case 0x07: // Physical Minimum
                    printf("[HID]     -> Physical Minimum\n");
                    break;
                case 0x08: // Physical Maximum
                    printf("[HID]     -> Physical Maximum\n");
                    break;
                case 0x09: // Report Size
                    if (size >= 1) {
                        printf("[HID]     -> Report Size: %d bits\n", desc[i + 1]);
                    }
                    break;
                case 0x0A: // Report Count
                    if (size >= 1) {
                        printf("[HID]     -> Report Count: %d\n", desc[i + 1]);
                    }
                    break;
            }
        } else if (type == 2) { // Local items
            switch (tag) {
                case 0x08: // Usage
                    if (size >= 1) {
                        uint16_t usage = desc[i + 1];
                        if (size >= 2) usage |= (desc[i + 2] << 8);
                        printf("[HID]     -> Usage: 0x%04x", usage);
                        switch (usage) {
                            case 0x01: printf(" (Pointer)"); break;
                            case 0x02: printf(" (Mouse)"); break;
                            case 0x30: printf(" (X)"); break;
                            case 0x31: printf(" (Y)"); break;
                            case 0x38: printf(" (Wheel)"); break;
                            default: printf(" (Unknown)"); break;
                        }
                        printf("\n");
                    }
                    break;
            }
        } else if (type == 0) { // Main items
            switch (tag) {
                case 0x08: // Input
                    printf("[HID]     -> Input Report\n");
                    break;
                case 0x09: // Output
                    printf("[HID]     -> Output Report\n");
                    break;
                case 0x0B: // Feature
                    printf("[HID]     -> Feature Report\n");
                    break;
                case 0x0A: // Collection
                    printf("[HID]     -> Collection\n");
                    break;
                case 0x0C: // End Collection
                    printf("[HID]     -> End Collection\n");
                    break;
            }
        }
        
        i += size + 1;
    }
    return(reportid);
}

int get_hid_report_descriptor(struct usb_device *dev, int interface_num, uint8_t *buffer, int max_len)
{
    int result;
    
    printf("[HID] Requesting HID Report Descriptor for interface %d...\n", interface_num);
    
    // Get HID Report Descriptor
    // USB_TYPE_STANDARD | USB_RECIP_INTERFACE | USB_DIR_IN
    result = usb_control_msg(dev,
                            usb_rcvctrlpipe(dev, 0),
                            USB_REQ_GET_DESCRIPTOR,
                            USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_INTERFACE,
                            (USB_DT_REPORT << 8) | 0,  // wValue: (descriptor type << 8) | descriptor index
                            interface_num,             // wIndex: interface number
                            buffer,
                            max_len,
                            USB_CNTL_TIMEOUT);
    
    if (result < 0) {
        printf("[HID] Standard request failed, trying class-specific request...\n");
        
        // Try HID class-specific request
        result = usb_control_msg(dev,
                                usb_rcvctrlpipe(dev, 0),
                                HID_REQ_GET_REPORT_DESC,
                                USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                                0,                // wValue
                                interface_num,    // wIndex: interface number
                                buffer,
                                max_len,
                                USB_CNTL_TIMEOUT);
    }
    
    if (result < 0) {
        printf("[HID] Failed to get HID Report Descriptor: %d\n", result);
        return result;
    }
    
    printf("[HID] Successfully retrieved %d bytes of HID Report Descriptor\n", result);
    return result;
}

static void print_device_configuration(struct usb_device *dev)
{
    printf("\n[DEVICE] Complete Device Configuration:\n");
    printf("[DEVICE] Device Descriptor:\n");
    printf("[DEVICE]   bcdUSB: 0x%04x\n", dev->descriptor.bcdUSB);
    printf("[DEVICE]   bDeviceClass: %d\n", dev->descriptor.bDeviceClass);
    printf("[DEVICE]   bDeviceSubClass: %d\n", dev->descriptor.bDeviceSubClass);
    printf("[DEVICE]   bDeviceProtocol: %d\n", dev->descriptor.bDeviceProtocol);
    printf("[DEVICE]   bMaxPacketSize0: %d\n", dev->descriptor.bMaxPacketSize0);
    printf("[DEVICE]   idVendor: 0x%04x\n", dev->descriptor.idVendor);
    printf("[DEVICE]   idProduct: 0x%04x\n", dev->descriptor.idProduct);
    printf("[DEVICE]   bcdDevice: 0x%04x\n", dev->descriptor.bcdDevice);
    printf("[DEVICE]   bNumConfigurations: %d\n", dev->descriptor.bNumConfigurations);
    
    printf("[DEVICE] Configuration Descriptor:\n");
    printf("[DEVICE]   bNumInterfaces: %d\n", dev->config.desc.bNumInterfaces);
    printf("[DEVICE]   bConfigurationValue: %d\n", dev->config.desc.bConfigurationValue);
    printf("[DEVICE]   bmAttributes: 0x%02x\n", dev->config.desc.bmAttributes);
    printf("[DEVICE]   bMaxPower: %d mA\n", dev->config.desc.bMaxPower * 2);
    
    for (int i = 0; i < dev->config.desc.bNumInterfaces; i++) {
        struct usb_interface_descriptor *intf = &dev->config.if_desc[i].desc;
        printf("[DEVICE] Interface %d:\n", i);
        printf("[DEVICE]   bInterfaceNumber: %d\n", intf->bInterfaceNumber);
        printf("[DEVICE]   bAlternateSetting: %d\n", intf->bAlternateSetting);
        printf("[DEVICE]   bNumEndpoints: %d\n", intf->bNumEndpoints);
        printf("[DEVICE]   bInterfaceClass: %d", intf->bInterfaceClass);
        switch (intf->bInterfaceClass) {
            case USB_CLASS_HID: printf(" (HID)"); break;
            case USB_CLASS_HUB: printf(" (Hub)"); break;
            case USB_CLASS_MASS_STORAGE: printf(" (Mass Storage)"); break;
            default: printf(" (Unknown)"); break;
        }
        printf("\n");
        printf("[DEVICE]   bInterfaceSubClass: %d\n", intf->bInterfaceSubClass);
        printf("[DEVICE]   bInterfaceProtocol: %d", intf->bInterfaceProtocol);
        if (intf->bInterfaceClass == USB_CLASS_HID) {
            switch (intf->bInterfaceProtocol) {
                case USB_PROT_HID_NONE: printf(" (None)"); break;
                case USB_PROT_HID_KEYBOARD: printf(" (Keyboard)"); break;
                case USB_PROT_HID_MOUSE: printf(" (Mouse)"); break;
                default: printf(" (Unknown HID)"); break;
            }
        }
        printf("\n");
        
        // Print endpoint information
        for (int j = 0; j < intf->bNumEndpoints; j++) {
            struct usb_endpoint_descriptor *ep = &dev->config.if_desc[i].ep_desc[j];
            printf("[DEVICE]   Endpoint %d:\n", j);
            printf("[DEVICE]     bEndpointAddress: 0x%02x", ep->bEndpointAddress);
            if (ep->bEndpointAddress & 0x80) {
                printf(" (IN)");
            } else {
                printf(" (OUT)");
            }
            printf(" EP%d\n", ep->bEndpointAddress & 0x0f);
            
            printf("[DEVICE]     bmAttributes: 0x%02x", ep->bmAttributes);
            switch (ep->bmAttributes & 0x03) {
                case USB_ENDPOINT_XFER_CONTROL: printf(" (Control)"); break;
                case USB_ENDPOINT_XFER_ISOC: printf(" (Isochronous)"); break;
                case USB_ENDPOINT_XFER_BULK: printf(" (Bulk)"); break;
                case USB_ENDPOINT_XFER_INT: printf(" (Interrupt)"); break;
            }
            printf("\n");
            
            printf("[DEVICE]     wMaxPacketSize: %d\n", ep->wMaxPacketSize);
            printf("[DEVICE]     bInterval: %d\n", ep->bInterval);
        }
    }
}

static void send_hid_commands(struct usb_device *dev, int interface_num)
{
    int result;
    uint8_t data[8] __attribute__((aligned(USB_DMA_MINALIGN)));
    
    printf("\n[HID] Sending HID initialization commands for interface %d...\n", interface_num);
    
    // Set Boot Protocol (might help with compatibility)
    printf("[HID] Setting Boot Protocol...\n");
    result = usb_control_msg(dev,
                            usb_sndctrlpipe(dev, 0),
                            USB_REQ_SET_PROTOCOL,
                            USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                            0,              // wValue: 0 = Boot Protocol, 1 = Report Protocol
                            interface_num,  // wIndex: interface number
                            NULL,
                            0,
                            USB_CNTL_TIMEOUT);
    
    if (result >= 0) {
        printf("[HID] Boot Protocol set successfully\n");
    } else {
        printf("[HID] Boot Protocol setting failed: %d\n", result);
    }
    
    // Set Idle to 0 (get all reports)
    printf("[HID] Setting Idle to 0...\n");
    result = usb_control_msg(dev,
                            usb_sndctrlpipe(dev, 0),
                            USB_REQ_SET_IDLE,
                            USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                            0,              // wValue: (duration << 8) | report ID
                            interface_num,  // wIndex: interface number
                            NULL,
                            0,
                            USB_CNTL_TIMEOUT);
    
    if (result >= 0) {
        printf("[HID] Idle set successfully\n");
    } else {
        printf("[HID] Idle setting failed: %d\n", result);
    }
    
    // Try to get current report
    printf("[HID] Attempting to get current input report...\n");
    memset(data, 0, sizeof(data));
    result = usb_control_msg(dev,
                            usb_rcvctrlpipe(dev, 0),
                            USB_REQ_GET_REPORT,
                            USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                            (1 << 8) | 0,   // wValue: (report type << 8) | report ID (1=Input, 2=Output, 3=Feature)
                            interface_num,  // wIndex: interface number
                            data,
                            sizeof(data),
                            USB_CNTL_TIMEOUT);
    
    if (result >= 0) {
        printf("[HID] Current input report (%d bytes): ", result);
        for (int i = 0; i < result; i++) {
            printf("%02x ", data[i]);
        }
        printf("\n");
    } else {
        printf("[HID] Get current input report failed: %d\n", result);
    }
    
    // Try Report Protocol instead of Boot Protocol
    printf("[HID] Setting Report Protocol...\n");
    result = usb_control_msg(dev,
                            usb_sndctrlpipe(dev, 0),
                            USB_REQ_SET_PROTOCOL,
                            USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                            1,              // wValue: 1 = Report Protocol
                            interface_num,  // wIndex: interface number
                            NULL,
                            0,
                            USB_CNTL_TIMEOUT);
    
    if (result >= 0) {
        printf("[HID] Report Protocol set successfully\n");
    } else {
        printf("[HID] Report Protocol setting failed: %d\n", result);
    }
}

static struct usb_device *find_hid_device(int *hid_interface_num)
{
    struct usb_device *dev;
    int i, j;

    printf("[DIAGNOSTIC] Scanning for HID devices...\n");
    
    for (i = 0; i < USB_MAX_DEVICE; i++) {
        dev = usb_get_dev_index(i);
        if (dev == NULL)
            continue;
            
        printf("[DIAGNOSTIC] Device %d with %d interfaces\n", i, dev->config.desc.bNumInterfaces);
        
        // Check all interfaces of this device
        for (j = 0; j < dev->config.desc.bNumInterfaces; j++) {
            printf("[DIAGNOSTIC] Device %d Interface %d: Class=%d, SubClass=%d, Protocol=%d\n", 
                   i, j, dev->config.if_desc[j].desc.bInterfaceClass,
                   dev->config.if_desc[j].desc.bInterfaceSubClass,
                   dev->config.if_desc[j].desc.bInterfaceProtocol);
            
            // Look for HID class devices
            if (dev->config.if_desc[j].desc.bInterfaceClass == USB_CLASS_HID) {
                printf("[DIAGNOSTIC] Found HID device at index %d interface %d\n", i, j);
                *hid_interface_num = j;
                return dev;
            }
        }
    }
    return NULL;
}

int extract_report_ids(const uint8_t *desc, int len, uint8_t *report_ids, int max_ids) {
    int i = 0, found = 0;
    while (i < len && found < max_ids) {
        uint8_t item = desc[i];
        uint8_t type = (item >> 2) & 0x03;
        uint8_t tag = (item >> 4) & 0x0F;
        uint8_t size = item & 0x03;
        if (size == 3) size = 4;
        // Report ID is a Global item, tag 0x08
        if (type == 1 && tag == 0x08 && size >= 1) {
            uint8_t report_id = desc[i + 1];
            // Evita duplicados
            int duplicate = 0;
            for (int j = 0; j < found; j++)
                if (report_ids[j] == report_id) duplicate = 1;
            if (!duplicate) {
                report_ids[found++] = report_id;
                printf("[HID] Detected Report ID: 0x%02x\n", report_id);
            }
        }
        i += size + 1;
    }
    // Si no se encuentra ningún Report ID, por defecto es 0
    if (found == 0) {
        report_ids[0] = 0;
        found = 1;
    }
    return found;
}
uint8_t report_desc[HID_REPORT_DESCRIPTOR_SIZE] __attribute__((aligned(USB_DMA_MINALIGN)));

int usb_hid_diagnostic(void)
{
    struct usb_device *hid_dev;
    int desc_len;
    int reportid=0;
    int hid_interface_num = 0;

    printf("\n=== USB HID Diagnostic Tool ===\n");

    hid_dev = find_hid_device(&hid_interface_num);

    if (!hid_dev) {
        printf("[DIAGNOSTIC] No USB HID device found.\n");
        return(reportid);
    }

    printf("[DIAGNOSTIC] USB HID device found!\n");
    printf("[DIAGNOSTIC] Using interface number: %d\n", hid_interface_num);
    printf("[DIAGNOSTIC] Manufacturer: %s\n", hid_dev->mf);
    printf("[DIAGNOSTIC] Product: %s\n", hid_dev->prod);
    printf("[DIAGNOSTIC] Serial: %s\n", hid_dev->serial);

    // Print complete device configuration
    print_device_configuration(hid_dev);

    // Get and analyze HID Report Descriptor
    memset(report_desc, 0, sizeof(report_desc));
    desc_len = get_hid_report_descriptor(hid_dev, hid_interface_num, report_desc, sizeof(report_desc));
    
    if (desc_len > 0) {
        reportid=analyze_hid_report_descriptor(report_desc, desc_len);
    } else {
        printf("[DIAGNOSTIC] Could not retrieve HID Report Descriptor\n");
    }
    
    // Send various HID commands to try to activate the device
    send_hid_commands(hid_dev, hid_interface_num);
    
    printf("\n=== End of HID Diagnostic ===\n");
    return(reportid);
}
