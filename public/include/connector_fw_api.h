/*
 * Copyright (c) 2013 Digi International Inc.,
 * All rights not expressly granted are reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
 * =======================================================================
 */

#ifndef CONNECTOR_FW_API_H_
#define CONNECTOR_FW_API_H_

/**
* @defgroup connector_request_id_firmware_t Firmware Requests
* @{
*/
/**
* Firmware Facility Request ID passed to the application's callback for firmware update interface.
* The class id for this connector_firmware_request_t is connector_class_id_firmware.
*/
typedef enum {
    connector_request_id_firmware_target_count,         /**< Requesting callback to return number of supported target for firmware update */
    connector_request_id_firmware_info,                 /**< Requesting callback to return the version number, maximum code size, description and regular expression for firmware update image name of specific target */
    connector_request_id_firmware_download_start,       /**< Requesting callback to start firmware update of specific target */
    connector_request_id_firmware_download_data,        /**< Callback is passed with image data for firmware update. This is called for each chunk of image data */
    connector_request_id_firmware_download_complete,    /**< Callback is called to complete firmware update. */
    connector_request_id_firmware_download_abort,       /**< Requesting callback to abort firmware update */
    connector_request_id_firmware_target_reset          /**< Requesting callback to reset the target */
} connector_request_id_firmware_t;
/**
* @}
*/

/**
* @defgroup connector_firmware_status_t Firmware download return status
* @{
*/
/**
* Return status code for firmware update. These status codes are used for @ref connector_firmware_download_request,
* @see @ref connector_firmware_binary_block and @ref connector_firmware_download_abort callbacks.
*/
typedef enum {
   connector_firmware_status_success,                        /**< No error */
   connector_firmware_status_download_denied,                /**< Callback denied firmware update */
   connector_firmware_status_download_invalid_size,          /**< Callback returns invalid size */
   connector_firmware_status_download_invalid_version,       /**< Callback returns invalid version */
   connector_firmware_status_download_unauthenticated,       /**< The server has not been authenticated */
   connector_firmware_status_download_not_allowed,           /**< The server is not allowed to provided updates */
   connector_firmware_status_download_configured_to_reject,  /**< Callback rejects firmware update */
   connector_firmware_status_encountered_error,              /**< Callback encountered an error that precludes the firmware update */
   connector_firmware_status_user_abort,                     /**< User aborted firmware update */
   connector_firmware_status_device_error,                   /**< Device or server encountered an error in the download data */
   connector_firmware_status_invalid_offset,                 /**< connector_firmware_binary_block callback found invalid offset. */
   connector_firmware_status_invalid_data,                   /**< connector_firmware_binary_block callback found invalid data block.*/
   connector_firmware_status_hardware_error                  /**< Callback found permanent hardware error */
} connector_firmware_status_t;
/**
* @}
*/

/**
* @defgroup connector_fw_download_complete_status_t Firmware complete status codes
* @{
*/
/**
* Firmware Update Complete status. These status codes are used in @see connector_firmware_download_complete callback.
*/
typedef enum {
   connector_firmware_download_success,               /**< Callback returns this for firmware download finished successfully and calculated checksum matched the checksum sent in the callback */
   connector_firmware_download_checksum_mismatch,     /**< Callback returns this for download completed successfully, but the calculated checksum did not match the checksum sent in the callback */
   connector_firmware_download_not_complete           /**< Callback did not complete download successfully */
} connector_firmware_download_status_t;
/**
* @}
*/

/**
* @defgroup connector_firmware_info_t Firmware information
* @{
*/
/**
* Firmware information structure for @ref connector_request_id_firmware_info callback to return
* information about the specific target.
*/
typedef struct {
    uint8_t target_number;          /**< Target number which target the firmware information is for*/

    struct {
        uint32_t version;           /**< Version number of the target number */
        uint32_t code_size;         /**< Code size of the target number. If size is unknown, set 0xFFFFFFFF */
        char * description;         /**< An ASCII description string of the target number */
        char * filename_spec;       /**< Regular expression for the firmware image name fo the target number */
    } info;                         /**< Callback writes the firmware information */
} connector_firmware_info_t;

/**
* @defgroup connector_firmware_download_start_t Download Request
* @{
*/
/**
* Firmware download request structure for @ref connector_firmware_download_request callback which
* is called when server requests firmware download.
*/
typedef struct {
    uint8_t target_number;        /**< Target number which target the firmware download is for */

    struct {
        uint32_t code_size;       /**< size of the code that is ready to be sent to the target */
        char * filename;    /**< Pointer to filename of the image to be downloaded */
    } image_info;                      /**< Contains the firmware download image information */

    connector_firmware_status_t status; /** Callback writes error status if error is encountered */

} connector_firmware_download_start_t;
/**
* @}
*/

/**
* @defgroup connector_firmware_download_data_t Image Data
* @{
*/
/**
* Firmware download image data structure for connector_firmware_binary_block callback which
* is called when iDigi connector receives a block of image data for firmware download.
*/
typedef struct {
    uint8_t target_number;  /**< Target number which firmware target the image data is for */

    struct {
        uint32_t offset;        /**< Offset value where this particular block of image data fits into the download */
        uint8_t * data;         /**< Pointer binary image data */
        size_t length;          /**< Length of binary image data in bytes */
    } image_data;               /**< Contains the firmware image data */

    connector_firmware_status_t status; /** Callback writes error status if error is encountered */

} connector_firmware_download_data_t;
/**
* @}
*/

/**
* @defgroup connector_firmware_download_complete_t Download complete
* @{
*/
/**
* Firmware download complete request structure containing information about firmware image data
* for connector_firmware_download_complete callback which is called when the iDigi Device Cloud is done
* sending all image data.
*/
typedef struct {
    uint8_t target_number;  /**< Target number which firmware target is completed firmware download */

    struct {
        uint32_t code_size;     /**< Code size of the entire image data sent */
        uint32_t checksum;      /**< CRC-32 value computed from offset 0 to code size. If it's 0, no checksum is required */
    } image_data;               /**< Contains information of the image data sent */

    struct {
        uint32_t version;                                /**< Version number of the downloaded image */
        connector_firmware_download_status_t status;     /**< Status code regarding the download completion */
    } image_status;                                      /**< Callback writes the new image status */

} connector_firmware_download_complete_t;
/**
* @}
*/

/**
* @defgroup connector_firmware_download_abort_t Download Abort
* @{
*/
/**
* Firmware download abort structure for connector_firmware_abort callback which
* is called when server aborts firmware download process.
*/
typedef struct {
    uint8_t target_number;             /**< Target number which target the firmware download is aborted */
    connector_firmware_status_t status;   /**< Abort reason or status */
} connector_firmware_download_abort_t;
/**
* @}
*/


#endif /* CONNECTOR_FW_API_H_ */
