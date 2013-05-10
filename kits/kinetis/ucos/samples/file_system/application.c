#include <includes.h>

#include <connector.h>

static void connector_status(connector_error_t const status, char const * const status_message)
{
    APP_TRACE_INFO(("connector_status: status update %d [%s]\n", status, status_message));
}

CPU_BOOLEAN set_clk(void)
{
    CPU_BOOLEAN ret;
    
    CLK_DATE_TIME date_time;
    CLK_YR Yr;
    CLK_DAY Day;
    CLK_MONTH Month;
    CLK_HR Hr;
    CLK_MIN Min;
    CLK_SEC Sec;
    CLK_TZ_SEC tz_sec;
           
    ret = Clk_GetDateTime(&date_time);
    if (ret == DEF_OK)
    {
        APP_TRACE_INFO(("Current Date/Time:\n"));
        APP_TRACE_INFO(("Date = %u/%u/%u\n", date_time.Month, date_time.Day, date_time.Yr));
        APP_TRACE_INFO(("Time = %u:%u:%u\n", date_time.Hr, date_time.Min, date_time.Sec));
    }
    else
        APP_TRACE_INFO(("Error in Clk_GetDateTime\n"));
    
    //TODO: Ask user for time
#if 0
#else
    // Set a fix Clk_SetDateTime so Files created on the RAM Filesystem have a date/time
    Yr = 2013;
    Month = 1;
    Day = 27;
    Hr = 10;
    Min = 10;
    Sec = 10;
    tz_sec = 0;
#endif
    
    ret = Clk_DateTimeMake(&date_time, Yr, Month, Day, Hr, Min, Sec, tz_sec);
    if (ret != DEF_OK)
        APP_TRACE_INFO(("Clk_DateTimeMake Failed\n"));
    ret = Clk_SetDateTime(&date_time);
    if (ret != DEF_OK)
        APP_TRACE_INFO(("Clk_GetDateTime Failed\n"));
    
    ret = Clk_GetDateTime(&date_time);
    if (ret == DEF_OK)
    {
        APP_TRACE_INFO(("New Date/Time:\n"));
        APP_TRACE_INFO(("Date = %u/%u/%u\n", date_time.Month, date_time.Day, date_time.Yr));
        APP_TRACE_INFO(("Time = %u:%u:%u\n", date_time.Hr, date_time.Min, date_time.Sec));
    }
    else
        APP_TRACE_INFO(("Error in Clk_GetDateTime\n"));
    
    return ret;
}

void fill_fs(void)
{
    FS_FILE     *p_file;
    FS_ERR       fs_err;
    OS_ERR       err_os;
    CPU_CHAR     p_src_buf[] = "Hello World\r\n";
    CPU_CHAR     p_dest_buf[100];
       
    APP_TRACE_INFO(("Create some files and folders on Ram Filesystem for evaluation\n"));
    
    p_file = FSFile_Open("ram:0:\\ucos_file1.txt", FS_FILE_ACCESS_MODE_RDWR | FS_FILE_ACCESS_MODE_CREATE, &fs_err);
    if (fs_err != FS_ERR_NONE)
        goto done;
                                                                /* Write to the file.                                   */
    FSFile_Wr(p_file, p_src_buf, Str_Len(p_src_buf) + 1, &fs_err);
    if (fs_err != FS_ERR_NONE)
        goto done;

                                                                /* Reset the file position to 0.                        */
    FSFile_PosSet(p_file, 0, FS_FILE_ORIGIN_START, &fs_err);
    if (fs_err != FS_ERR_NONE)
        goto done;
                                                                /* Read back the data.                                  */
    FSFile_Rd(p_file, p_dest_buf, Str_Len(p_src_buf) + 1, &fs_err);
    if (fs_err != FS_ERR_NONE)
        goto done;

    APP_TRACE_INFO(("Read from ram:0:\\ucos_file1.txt: %s", p_dest_buf));

    /* Close the file.                                      */
    FSFile_Close(p_file, &fs_err);
    if (fs_err != FS_ERR_NONE)
        goto done;
    
    // Create a dir
    FSEntry_Create ("ram:0:\\ucos_dir",
                        FS_ENTRY_TYPE_DIR,
                        DEF_YES,
                        &fs_err);
    if (fs_err != FS_ERR_NONE)
        goto done;   
    
    // Introduce a 2 second delay to test file DateTimeWr
    OSTimeDlyHMSM(0, 0, 2, 0, 
                      OS_OPT_TIME_HMSM_STRICT, 
                      &err_os);
    
    p_file = FSFile_Open("ram:0:\\ucos_dir\\ucos_file2.txt", FS_FILE_ACCESS_MODE_RDWR | FS_FILE_ACCESS_MODE_CREATE, &fs_err);
    if (fs_err != FS_ERR_NONE)
        goto done;

    /* Write to the file. */
    FSFile_Wr(p_file, p_src_buf, Str_Len(p_src_buf) + 1, &fs_err);
    if (fs_err != FS_ERR_NONE)
        goto done;
    
    FSFile_Close(p_file, &fs_err);
    if (fs_err != FS_ERR_NONE)
        goto done;
    
done:
}

int application_start(void)
{
    OS_ERR err_os = OS_ERR_NONE;
    connector_error_t ret;
    
    /* Initialize clock module */
    set_clk();
    
    /* Create some files and folders on Ram Filesystem for evaluation */
    fill_fs();

    APP_TRACE_INFO(("application_start: calling connector_start\n"));
    ret = connector_start(connector_status);
    if (ret != connector_error_success)
    {
        APP_TRACE_INFO(("connector_start failed [%d]\n", ret));
        return -1;
    }
      
    BSP_LED_Off(BSP_LED_ALL);                       /* Turn off all LEDs.                                   */
    
	while (DEF_TRUE && (err_os == OS_ERR_NONE)) {   /* Task body, always written as an infinite loop.       */
        BSP_LED_Toggle(2);      
        
        OSTimeDlyHMSM(0, 0, 0, 500, 
                      OS_OPT_TIME_HMSM_STRICT, 
                      &err_os);
    }
    
    return 0;
}