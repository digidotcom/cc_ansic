
  o----------------------------------------o
  | Basic Remote Configuration RCI Example |
  o----------------------------------------o

  Compatible platforms
  --------------------
  * TWRK53N512 Board
  * TWRK60N512 Board
  * TWRK70F120M Board

  Introduction
  ------------
  This sample demonstrates how to perform a remote configuration of the device through 
  Device Cloud using the remote configuration (RCI) service.

  Requirements
  ------------
  To run this example you will need:
    * Freescale Tower Kit with Ethernet connection.
    * A Device Cloud account with the device registered in.

  Example setup
  -------------
  1) Make sure the hardware is set up correctly:
       a) The device is powered on.
       b) The device is connected to the Local Area Network (LAN) by the 
          Ethernet cable and has Internet access.

  2) Configure the Etherios settings at the project's connector_config.h file.
     Define the following macros with your device MAC address (from the TWR-SER module)
     and the Device type (user-defined string):
        #define CONNECTOR_MAC_ADDRESS                     {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC}
        #define CONNECTOR_DEVICE_TYPE                     "EtheriosSample"

     And the following with your account Vendor ID and Cloud URL:
        #define CONNECTOR_VENDOR_ID                       0xAABBCCDD
        #define CONNECTOR_CLOUD_URL                       "login.etherios.com"

  3) Open and configure a Serial Terminal with the following parameters:
         Baudrate: 115200
         Parity:   None
         Bits:     8

     The Port number depends on your PC configuration and on wether you are using
     the Tower USB serial port or the TWR-ELEV DB9 connector.
     
	 **NOTE: To recognize the USB port as a Serial port, the minimum Firmware 
             version should be 31.21.
             Use the P&E Terminal Utility for inferior versions; or update 
             your device firmware version using the P&E Firmware Updater 
             Utility.

  Running the example
  -------------------
  Once updated the connector_config.h settings, all you need to do is to build and
  launch the application.

  While it is running, you can press the push button 'SW1' to send raw data to
  the cloud. You can see the console output on the terminal window for the
  device status.

  To verify that that the Remote configuration support is working properly, 
  follow these steps:
    1) Open your web browser and navigate to your Device Cloud by Etherios page 
       (login.etherios.com or login.etherios.co.uk) and log in with your username 
       and password.
    2) Once there, go to the 'Device Manager' section.
    3) Select 'Devices' to display the list of devices in your Device Cloud 
       inventory.
    4) Double-click on your device to access its settings remotely.
    5) Open the 'System' group and try to change the 'Location' setting. Then 
       click 'Save' to save this setting remotely.

  Tested On
  ---------
  TWRK53N512 Board
  TWRK60N512 Board
  TWRK70F120M Board

