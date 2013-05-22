
  o---------------------------------------------------o
  | Send Data Points to Etherios Device Cloud Example |
  o---------------------------------------------------o

  Compatible platforms
  --------------------
  * TWRK53N512 Board
  * TWRK60N512 Board
  * TWRK70F120M Board

  Introduction
  ------------
  This sample sends streaming data points to the Etherios Device Cloud simulating a
  sampling device. On the other handDevice Cloud (with Data Stream enabled) collects
  these data and the data shows up under Data Streams page inside the Data Service tab.
  
  This example uploads one Data Point per second simulating a "sawtooth" signal.

  To learn more about Device Cloud Data Streams, see Chapter 12 Device Cloud Data
  Streams in the Device Cloud User's Guide:
        http://www.digi.com/support/productdetail?pid=5575

  Requirements
  ------------
  To run this example you will need:
    * Freescale Tower Kit with Ethernet connection.
    * An Etherios account with the device registered in.

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
        #define CONNECTOR_DEVICE_TYPE                     "Data Points example"

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

  While it is running, the application will upload data points with the configured
  rate. You can see the console output on the terminal window for the device status.

  To verify that the Data Points have been uploaded to the Device Cloud successfully,
  follow these steps:
    1) Open your web browser and navigate to your Device Cloud by Etherios page 
       (login.etherios.com or login.etherios.co.uk) and log in with your username 
       and password.

    2) Once there, go to the 'Data Services' section from the top menu.
	
	3) Click on the 'Data Streams' tab.

    4) You should see a Stream called "Device_ID/SawtoothSignal". For example:
	   00000000-00000000-000AAFFF-FFBBCCDD/SawtoothSignal

       NOTE: Use the table in 'Device Manager' section to find out the Device ID of 
       your board.

    5) Clicking on the stream, will show the uploaded data in graphic and table 
	   representations.

  Tested On
  ---------
  TWRK53N512 Board
  TWRK60N512 Board
  TWRK70F120M Board

