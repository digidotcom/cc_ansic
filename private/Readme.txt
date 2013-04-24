                        Release Notes PN _RELEASE_NOTES_PARTNO_
                   Etherios Cloud Connector for Embedded
                               v_CONNECTOR_SW_VERSION_ 

INTRODUCTION

        Cloud Connector for Embedded is a software development package used to
        enable a device to exchange information with Device Cloud.  Cloud
        Connector supports application to device data interaction (messaging),
        application and device data storage, and remote management of devices.
        Devices are associated with Device Cloud through the Internet or other 
        wide area network connections, which allows for communication between 
        the device and customer applications, via Device Cloud.

SUPPORTED PRODUCTS

        Etherios Cloud Connector for Embedded

ENHANCEMENTS

    v2.0.0

        This is the initial public release and a replacement product for the 
        iDigi Connector for Embedded.  
        
        With respect to iDigi Connector v1.2, the Cloud Connector includes 
        optimized Remote Configuration support with radically smaller memory 
        foot print and reduced network bandwidth needs; Support for Data 
        Streams, and support for Short Messaging over UDP.

BUG FIXES

    v2.0.0

        Corrected ARM misalignment warnings, which cause compilation errors
        due to -Werror.  The casting was between byte and word pointers, 
        which verified safe in the original code.  A cast to void * was used 
        to avoid the warning.  [IC4C-49] 

KNOWN LIMITATIONS

    None 
    
HISTORY

    None 
    
    