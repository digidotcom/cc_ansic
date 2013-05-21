
  o--------------------------------------------o
  | Send Data to Etherios Device Cloud Example |
  o--------------------------------------------o

  Compatible platforms
  --------------------
  * TWRK53N512 Board
  * TWRK60N512 Board
  * TWRK70F120M Board

  Introduction
  ------------
  This sample demonstrates how one can use the Device Cloud by Etherios to control their remote
  device.
  
  The sample allows user to turn ON or OFF the TWR boards' LEDs (LED1 or LED2).

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
        #define CONNECTOR_DEVICE_TYPE                     "Device Request Example"

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

  When the application is running, you can send Device Requests to change the status of the LEDs.
  To do this from the Etherios webpage, follow these steps:
    1) Open your web browser and navigate to your Device Cloud by Etherios page 
       (login.etherios.com or login.etherios.co.uk) and log in with your username 
       and password.
    2) Once there, go to the 'Documentation' section.
    3) Select 'SCI Targets' to open the 'Select devices to be used in examples'
       dialog. Use the 'Add Targets' combo box to look for the right Device ID,
       click 'Add' and then 'OK'.

       NOTE: Use the table in 'Device Manager > Devices' section to find
       out the Device ID of your board.

    4) Select the option 'Examples > SCI > Data Service > Send Request', on the
       left panel a SCI request will be automatically generated.
    5) Make sure the Device ID tag was automatically set to match your Device's ID.
    6) Replace the target with "LED1" or "LED2" and the payload with ON or
       OFF, i.e., to turn on LED1 the request should be:

       <sci_request version="1.0">
          <data_service>
             <targets>
                <device id="00000000-00000000-XXXXXXFF-FFXXXXXX"/>
             </targets>
             <requests>
                <device_request target_name="LED1">
                   ON
                </device_request>
             </requests>
          </data_service>
       </sci_request>

    7) Select 'Send' to see the request coming in the device. You should see
       the console output on the terminal window for the received request:

       Turning ON LED1
       LED1 action is Success

    8) Observe the response returned from the device on the 'Web Services
       Responses' on the right. Click 'Click to examine' to see both request
       and response, the last will be something similar to:

       <sci_reply version="1.0">
          <data_service>
             <device id="00000000-00000000-XXXXXXFF-FFXXXXXX">
                <requests>
                   <device_request target_name="LED1" status="1">Success</device_request>
                </requests>
             </device>
          </data_service>
       </sci_reply>

  Optionally, a Python 2.7 script is provided with a very basic web services script to 
  send a device request to Device Cloud. To run the script, open a command line, navigate 
  to [ETHERIOS_CC_4_KINETIS_PATH]\Python Scripts\ and run:

    python device_request.py <username> <password> <device_id> <target_led> <state>'

  For Example:
  
    pyhton device_request.py username password 00000000-00000000-001234FF-FF567890 LED2 ON
  
  Tested On
  ---------
  TWRK53N512 Board
  TWRK60N512 Board
  TWRK70F120M Board

