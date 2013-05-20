/* 
 * This file contains all the configurations of the Cloud Connector.
 */

/* General category */
//#define CONNECTOR_MAC_ADDRESS                         {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
//#define CONNECTOR_VENDOR_ID                           0x00000000
//#define CONNECTOR_CLOUD_URL                           "login.etherios.com"
#define CONNECTOR_DEVICE_TYPE                         	"MQX 4.0 on Kinetis"

/* Services category */
#define CONNECTOR_MSG_MAX_TRANSACTION                 1
//#define CONNECTOR_DATA_POINTS
//#define CONNECTOR_DATA_SERVICE


//#define CONNECTOR_FILE_SYSTEM
//#define CONNECTOR_FILE_SYSTEM_MAX_PATH_LENGTH   256
//#define APPLICATION_FILE_SYSTEM_USB 		/* Define only one */
//#define APPLICATION_FILE_SYSTEM_SDCARD	/* Define only one */
//#define APPLICATION_IGNORE_SDCARD_PROTECTION /* Define if compiling for TWRK60N512 RevD */

#define CONNECTOR_RCI_SERVICE
#define CONNECTOR_RCI_MAXIMUM_CONTENT_LENGTH 	256

//#define CONNECTOR_FIRMWARE_SERVICE
//#define APPLICATION_FIRMWARE_SERVICE_STUB
//#define APPLICATION_FIRMWARE_SERVICE_FULL

/* Advanced category */
#define CONNECTOR_TX_KEEPALIVE_IN_SECONDS             75
#define CONNECTOR_RX_KEEPALIVE_IN_SECONDS             75
#define CONNECTOR_WAIT_COUNT                          5
#define CONNECTOR_NO_COMPRESSION
/* * * * * * * * * * * * * * * * * * * * */
#ifndef CONNECTOR_MAC_ADDRESS
#error "Please define a MAC address in connector_config.h"
#endif

#ifndef CONNECTOR_VENDOR_ID
#error "Please define the Vendor ID in connector_config.h"
#endif

#ifndef CONNECTOR_CLOUD_URL
#error "Please define the Device Cloud URL connector_config.h"
#endif

#if defined CONNECTOR_FIRMWARE_SERVICE
#if !defined APPLICATION_FIRMWARE_SERVICE_STUB && !defined APPLICATION_FIRMWARE_SERVICE_FULL
#error "You must define APPLICATION_FIRMWARE_SERVICE_STUB or APPLICATION_FIRMWARE_SERVICE_FULL in connector_config.h to support Firmware Updates"
#endif
#endif

/* Network category */
#if !defined CONNECTOR_CONNECTION_TYPE
#define CONNECTOR_CONNECTION_TYPE                     connector_connection_type_lan
#endif

#if !defined CONNECTOR_TRANSPORT_TCP
#define CONNECTOR_TRANSPORT_TCP
#endif

/* Platform definitions */
#if !defined CONNECTOR_LITTLE_ENDIAN
#define CONNECTOR_LITTLE_ENDIAN
#endif