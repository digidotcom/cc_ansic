                      Release Notes for _RELEASE_NOTES_PARTNO_
                     Etherios Cloud Connector v_CONNECTOR_SW_VERSION_
                             _RELEASE_DATE_ 

ENHANCEMENTS

    v1.2
     
        Support for Remote configuration (using the rci_request payload mechanism
        within the send_message operation) has been added.  This allows iDigi 
        Connector applications to develop configurations that can be remotely 
        managed (read/write) through iDigi Manager Pro or a web services client.  
        Note this feature requires a C99 Compiler.
        
        Support for Remote File Systems (using the file_system operation) has 
        been added.  This allows remotely connected devices to easily connect
        to standard file system commands (i.e., open, close, read, write, stat) 
        and have the file system contents displayed through iDigi Manager Pro or 
        through a web services client. 
        
        Additional examples demonstrating secure connections using OpenSSL.  This 
        allows sensitive applications to easily include greater levels of network 
        security.  
        
    v1.1

        Added support for device requests from the iDigi device cloud, the IIK
        now supports bidirectional data transfers.

        Updated the original User Guide with an online software centric 
        HTML-based User Guide with more hot links and tighter integration with 
        software distribution.

        Reduced the amount of dynamic memory required to execute the IIK.  With 
        added support for the connector_config_max_transaction callback (or the 
        compile time CONNECTOR_MSG_MAX_TRANSACTION definition), a limited memory 
        approach is employed instead of an unbounded model.  For minimal memory 
        usage, connector_config_max_transaction (or CONNECTOR_MSG_MAX_TRANSACTION) 
        should be set to 1.   
       
DEPRECATED FUNCTIONS AND CHANGES

    v1.1
     
        Deprecated the v1.0 Data Service call routine methodology, which used
        several connector_initiate_action() calls, with the request argument cast to 
        an connector_data_request_t structure and the flag field set to 
        CONNECTOR_DATA_REQUEST_START, CONNECTOR_DATA_REQUEST_LAST, etc.  This now uses
        a single connector_initiate_action() call, with a CONNECTOR_DATA_PUT_ARCHIVE or
        CONNECTOR_DATA_PUT_APPEND flag, along with a several
        connector_data_service_device_request application callbacks.
        
        Note legacy applications can choose to compile the IIK library without
        the CONNECTOR_VERSION number defined, which will compile in the required
        compatibility sections.

KNOWN ISSUES

    None 
    
BUG FIXES

    v1.2.2
     
        Corrected ARM misalignment warnings, which cause compilation errors
        due to -Werror.  The casting was between byte and word pointers, 
        which verified safe in the original code.  A cast to void * was used 
        to avoid the warning.  [IC4C-49] 

    v1.2.1
     
        Corrected boundary condition defect related to RCI responses that are of
        size N*497 bytes, where N is an integer.  [44612] 

        Corrected rendering of XML entity characters contained in RCI 
        configuration pages.  [45364, 45365]
         
        Corrected iDigi Manager Pro problems when RCI configuration input files  
        used a '0xhex' type or the keyword 'unit'.  The corrected type value 
        is now '0x_hex32' and the corrected keyword is now 'units'.  [44959]
        
        Corrected intermittent problem with idigi_file_system_close callback on 
        the Linux platform when app_process_file_close() returns busy.  [44755] 

    v1.2
     
        Corrected all platforms to include handling for unrecognized iDigi 
        Callback cases with an connector_callback_unrecognized return value.  This 
        was missing in v1.1 and will allow v1.2 applications to port correctly 
        to v1.3 and beyond. 
