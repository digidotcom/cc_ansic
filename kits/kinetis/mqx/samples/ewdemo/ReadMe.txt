
  o--------------------------------o
  | Kinetis Tower Demo Example |
  o--------------------------------o

  Compatible platforms
  --------------------
  * TWRK53N512 Board
  * TWRK60N512 Board

  Introduction
  ------------
  Freescale Kinetis TWRK60N512 and TWRK53N512 Demo application.

  Requirements
  ------------
  To run this example you will need:
    * One Freescale Tower Kit with Ethernet connection.
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

     Finally, uncomment ONE of the following lines depending on your platform:
	    //#define TWR_K60N512
        //#define TWR_K53N512

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
  
  To verify that the Freescale Kinetis Tower Demo is running successfully, 
  follow these steps:
  
    1) To access the Freescale Kinetis K60 Tower Demo from a browser on your
       desktop (Google Chrome is recommended), use 
       
       http://kinetis-connector.appspot.com
       
    2) You have to use the Device Cloud server, login and username of your 
       Device Cloud account. The login will also be using the MAC Address you 
       selected.
  
  The Freescale Kinetis Tower Demo application will be continuously sending
  files to the Device Cloud server that will be interpreted and displayed in the
  browser or your mobile app. You will see the following:
  
    1) Go to 'I/O > Attitude Indicator' and tilt the Kinetis K60 tower. You will
       see the accelerometer data displayed on a GUI.
    2) Go to 'I/O > LEDs' and use the 'Stop', 'Slow' and 'Fast' buttons to 
       change the LED blinking rate.
    3) Go to 'I/O > Potentiometer' and move the dial of the potentiometer, and 
       see the values displayed in the GUI.
    4) Go to 'I/O > GPIO'. The duty cycle of the GPIO pulse generator will be 
       rendered in a GUI. You will be able to change the duty cycle by moving 
       the slider.
       To correctly view the current duty cycle, you will need to connect the 
       Primary Elevator pins J8-30 (A side expansion port) to J9-21 (B side 
       expansion port).
       If you do not tie these together, you will see the failures to Detect 
       GPIO to A/D converter counter continually increment.
    5) Select the 'Info' tab to view some data: Platform, OS, Uptime, 
       CPU Usage, number of active tasks, Network statistics, LED override and 
       the last reset type.
          a) CPU usage:
               i) Press 'SW1' button to create a utility thread. The CPU usage 
                  will go up.
              ii) Press 'SW1' again to create a second utility thread. The CPU 
                  usage will go up again. 
             iii) Press 'SW1' a third time and both threads will be killed.
                  The CPU usage will go back to its starting value, and you 
                  will see a change to the number of startups and terminations.
          b) Watchdog reset:
             'SW2' button was to invoke a watchdog reset. The info page will 
             display what caused the last reset.
          c) LED Override:
             Touch one of the touchpads under the LEDs. The blink rate will 
             change and you will see the value of the 'LED Override' field 
             change on the 'Info' page.
    6) Under the 'Log' tab, you will a log of the different actions you have
       been taking.

  Tested On
  ---------
  TWRK53N512 Board
  TWRK60N512 Board

