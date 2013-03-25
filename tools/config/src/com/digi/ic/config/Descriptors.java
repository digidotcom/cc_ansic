package com.digi.ic.config;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.URL;
import java.util.LinkedHashMap;
import java.util.LinkedList;

import javax.net.ssl.HttpsURLConnection;
import javax.xml.bind.DatatypeConverter;

public class Descriptors {

    private final String RCI_VERSION = "1.1";

    private final String RCI_DESCRIPTORS = "<descriptor element=\"rci_request\" desc=\"Remote Command Interface request\">\n"
            + "<attr name=\"version\" desc=\"RCI version of request.  Response will be returned in this versions response format\" default=\""
            + RCI_VERSION
            + "\">\n"
            + "<value value=\""
            + RCI_VERSION
            + "\" desc=\"Version " + RCI_VERSION + "\"/></attr>\n";

    private final String SETTING_DESCRIPTOR_DESCRIPTION = "device configuration";
    private final String STATE_DESCRIPTOR_DESCRIPTION = "device state";

    private final String username;
    private final String password;
    private final String deviceType;
    private String vendorId;
    private final long fwVersion;
    private Boolean callDeleteFlag;
    private int responseCode;

    public Descriptors(final String username, final String password,
                       final String vendorId, final String deviceType, 
                       final long version) {
        this.username = username;
        this.password = password;
        this.deviceType = deviceType;
        this.fwVersion = version;

        validateServerName();
        this.vendorId = vendorId;
        if (vendorId == null) getVendorId();
        else {
            if (vendorId.startsWith("0x")) {
                String hex = vendorId.substring(2);
                ConfigGenerator.debug_log(String.format("Vendor ID = 0x%08X (%d)\n", Integer.parseInt(hex, 16), Integer.parseInt(hex, 16)));
            } else {
                String dec = vendorId;
                ConfigGenerator.debug_log(String.format("Vendor ID = 0x%08X (%d)\n", Integer.parseInt(dec, 10), Integer.parseInt(dec, 10)));
            }
        }
        this.callDeleteFlag = true;
        this.responseCode = 0;
    }

    public void processDescriptors(ConfigData configData) throws Exception {

        if (ConfigGenerator.deleteDescriptorOption()) {
            deleteDescriptors();
        } else {
            int id = 1;
            for (ConfigData.ConfigType type : ConfigData.ConfigType.values()) {
                LinkedList<GroupStruct> groups = null;

                String configType = type.toString().toLowerCase();
                
                try {
                    groups = configData.getConfigGroup(configType);

                } catch (Exception e) {
                    /* end of the ConfigData ConfigType */
                    break;
                }

                if (!groups.isEmpty()) sendDescriptors(configType, groups, configData, id);
                /* add 2 because sendDescriptors uploads 2 sets of descriptors (query and set ). */
                id += 2;
            }

            sendRciDescriptors(configData);
        }
    }

    private void deleteDescriptors()
    {
        String target = String.format("/ws/DeviceMetaData?condition=dvVendorId=%s and dmDeviceType=\'%s\' and dmVersion=%d",
                vendorId, deviceType, fwVersion);

        String response = sendCloudData(target.replace(" ", "%20"), "DELETE", null);
        if (responseCode != 0)
        {
            switch (responseCode)
            {
            case 401:
            ConfigGenerator.log("Unauthorized: verify username and password are valid\n");
            break;
            
            case 403:
            ConfigGenerator.log("Forbidden: deleting previous RCI descriptors failed, verify that vendor ID is valid and is owned by your account.\n");
            break;
            
            default:
            ConfigGenerator.log("Response status: " + response);
            break;
            }
            
            System.exit(1);
        }
        ConfigGenerator.debug_log("Deleted target: " + target);
        ConfigGenerator.debug_log(String.format("Deleted: 0x%X/%s", Integer.parseInt(vendorId), deviceType));
        ConfigGenerator.debug_log(response);
    }
    
    private String getErrorDescriptors(int id, LinkedHashMap<String, String> errorMap) {
        String descriptor = "";
        int errorId = id;

        for (String errorName : errorMap.keySet()) {
            descriptor += String.format("<error_descriptor id=\"%d\" ", errorId);
            if (errorMap.get(errorName) != null)
                descriptor += String.format("desc=\"%s\" ", errorMap.get(errorName));
            
            descriptor += "/>\n";
                
            errorId++;
        }

        return descriptor;
    }

    private void sendDescriptors(String config_type, LinkedList<GroupStruct> groups, ConfigData configData, int id) throws Exception {
        String desc = SETTING_DESCRIPTOR_DESCRIPTION;

        if (config_type.equalsIgnoreCase(ConfigData.ConfigType.STATE.toString()))
            desc = STATE_DESCRIPTOR_DESCRIPTION;

        String query_descriptors = String.format("<descriptor element=\"query_%s\" desc=\"Retrieve %s\" format=\"all_%ss_groups\"\n",
                                                  config_type, desc, config_type);
        
        /* setup query command descriptor */
        if (ConfigGenerator.getBinaryOption()) {
            query_descriptors += String.format(" bin_id=\"%d\">\n", id)
                               + String.format("<format_define name=\"all_%ss_groups\">\n", config_type);
        } else {
            query_descriptors += String.format("> <format_define name=\"all_%ss_groups\">\n", config_type);
        }

        /*
         * get all errors for query command descriptor. 1. common errors. 2.
         * command errors. 3. user global errors
         * 
         * We must offset the error_id for command errors.
         */
        query_descriptors += getErrorDescriptors(configData.getRciCommonErrorsIndex(), configData.getRciCommonErrors())
                             + getErrorDescriptors(configData.getRciCommandErrorsIndex(), configData.getRciCommandErrors())
                             + getErrorDescriptors(configData.getUserGlobalErrorsIndex(), configData.getUserGlobalErrors());

        String set_descriptors = String.format("<descriptor element=\"set_%s\" desc=\"Set %s\" format=\"all_%ss_groups\"\n",
                                                config_type, desc, config_type);
        
        if (ConfigGenerator.getBinaryOption()) {
            set_descriptors += String.format(" bin_id=\"%d\"", id+1);
        }
        set_descriptors += ">\n";
    
        /*
         * get all errors for set command descriptor. 1. common errors. 2.
         * command errors. 3. user global errors
         * 
         * We must offset the error_id for command errors.
         */
        set_descriptors += getErrorDescriptors(configData.getRciCommonErrorsIndex(), configData.getRciCommonErrors())
                           + getErrorDescriptors(configData.getRciCommandErrorsIndex(), configData.getRciCommandErrors())
                           + getErrorDescriptors(configData.getUserGlobalErrorsIndex(), configData.getUserGlobalErrors());

        set_descriptors += "</descriptor>";

        for (GroupStruct group : groups) {
            query_descriptors += group.toString(groups.indexOf(group));
            
            query_descriptors += String.format("<attr name=\"index\" desc=\"%s\" type=\"int32\" min=\"1\" max=\"%d\" />",
                                                    group.getDescription(), group.getInstances());
                
            for (ElementStruct element : group.getElements()) {

                query_descriptors += element.toString(group.getElements().indexOf(element));
                
                if (ElementStruct.ElementType.toElementType(element.getType()) == ElementStruct.ElementType.ENUM) {

                    for (ValueStruct value : element.getValues()) {
                        query_descriptors += value.toString(element.getValues().indexOf(value));
                    }

                    query_descriptors += "</element>\n";

                }
            }

            /*
             * Write error for individual groups 1. common errors 2. group
             * errors 3. user global error 4. user group error
             * 
             * We must offset the error id for group errors.
             */
            query_descriptors += getErrorDescriptors(configData.getRciCommonErrorsIndex(), configData.getRciCommonErrors())
                                 + getErrorDescriptors(configData.getRciGroupErrorsIndex(), configData.getRciGroupErrors())
                                 + getErrorDescriptors(configData.getUserGlobalErrorsIndex(), configData.getUserGlobalErrors());

            query_descriptors += getErrorDescriptors(configData.getAllErrorsSize() + 1, group.getErrors());

            query_descriptors += "</descriptor>";
        }
        query_descriptors += "</format_define>\n</descriptor>\n";

        uploadDescriptor("descriptor/query_" + config_type, query_descriptors);
        uploadDescriptor("descriptor/set_" + config_type, set_descriptors);
    }

    private void sendRciDescriptors(ConfigData configData) throws IOException {
        String descriptors = RCI_DESCRIPTORS;

        for (ConfigData.ConfigType type : ConfigData.ConfigType.values()) {
            LinkedList<GroupStruct> groups = null;

            String configType = type.toString().toLowerCase();

            try {

                groups = configData.getConfigGroup(configType);

            } catch (Exception e) {
                /* end of the ConfigData ConfigType */
                break;
            }
            if (!groups.isEmpty()) {
                descriptors += String.format("<descriptor element=\"query_%s\" dscr_avail=\"true\" />\n", configType)
                               + String.format("<descriptor element=\"set_%s\" dscr_avail=\"true\" />\n", configType);
            }
        }

        descriptors += getErrorDescriptors(configData.getRciCommonErrorsIndex(), configData.getRciCommonErrors())
                       + getErrorDescriptors(configData.getRciGlobalErrorsIndex(), configData.getRciGlobalErrors()) 
                       + "</descriptor>";

        uploadDescriptor("descriptor", descriptors);
    }

    private String sendCloudData(String target, String method, String message) {
        String response = "";
        String cloud = "https://" + ConfigGenerator.getServerName() + target;
        String credential = username + ":" + password;
        String encodedCredential = DatatypeConverter.printBase64Binary(credential.getBytes());
        HttpsURLConnection connection = null;

        responseCode = 0;
        try {
            URL url = new URL(cloud);

            connection = (HttpsURLConnection) url.openConnection();
            connection.setRequestMethod(method);
            connection.setRequestProperty("Content-Type", "text/xml");
            connection.setRequestProperty("Authorization", "Basic " + encodedCredential);

            if (message != null) {
                connection.setDoOutput(true);

                OutputStreamWriter request = new OutputStreamWriter(connection.getOutputStream());
                request.write(message);
                request.close();
            }

            connection.connect();
            BufferedReader reader = new BufferedReader(new InputStreamReader(connection.getInputStream()));
            String respLine;

            while ((respLine = reader.readLine()) != null) {
                response += respLine;
            }
            reader.close();
            connection.disconnect();

        } catch (Exception resp) {
            try
            {
                responseCode = connection.getResponseCode();
                response = connection.getHeaderField(0);
            }
            catch (Exception e)
            {
                ConfigGenerator.log("ERROR: Invalid iDigi Server\n");
                System.exit(1);
            }
        }

        return response;
    }

    private void getVendorId() {

        ConfigGenerator.debug_log("Query vendor ID");

        // Request an existing vendor ID
        String response = sendCloudData("/ws/DeviceVendor", "GET", null);

        if (responseCode == 0) {
            int startIndex = response.indexOf("<dvVendorId>");

            if (startIndex == -1 || startIndex != response.lastIndexOf("<dvVendorId>")) {
                ConfigGenerator.debug_log("No vendor ID is found.");

            } else {
                startIndex += "<dvVendorId>".length();
                vendorId = response.substring(startIndex, response.indexOf("</dvVendorId>"));
            }

        } else {
            switch (responseCode) {
            case 401:
                ConfigGenerator.log("Unauthorized: verify username and password are valid\n");
                break;

            case 403:
                ConfigGenerator
                        .log("Forbidden: verify that your account has the \'Embedded Device Customization\' service subscribed.\n");
                break;

            default:
                ConfigGenerator.log("Response status: " + response);
                break;
            }
            System.exit(1);
        }

        if (vendorId == null) {
            // Request to create a new Vendor ID
            ConfigGenerator.debug_log("Create a new vendor ID");
            sendCloudData("/ws/DeviceVendor", "POST", "<DeviceVendor></DeviceVendor>");

            response = sendCloudData("/ws/DeviceVendor", "GET", null);

            if (responseCode != 0) {
                ConfigGenerator.log("Response status: " + response);
            }

            int startIndex = response.indexOf("<dvVendorId>");
            if (startIndex == -1) {
                ConfigGenerator
                        .log("Cannot create a new vendor ID for "
                                + username
                                + "user. User needs to manually create one. Refer to \"Setup your iDigi Acount\" section of the Getting started guide to obtain one.");
                System.exit(1);
            }

            if (startIndex != response.lastIndexOf("<dvVendorId>")) {
                ConfigGenerator.log(username + " has more than one vendor ID, so please specify the correct one.");
                System.exit(1);
            }

            startIndex += "<dvVendorId>".length();
            vendorId = response.substring(startIndex, response.indexOf("</dvVendorId>"));
        }

        if (vendorId != null)
            ConfigGenerator.log(String.format("The iDigi Device Cloud registered vendor ID: 0x%X", Integer
                    .parseInt(vendorId)));
    }


    private void validateServerName() {
        ConfigGenerator.debug_log("Start validating server");
        String response = sendCloudData("/ws/UserInfo", "GET", null);

        if (responseCode != 0)
        {
            switch (responseCode)
            {
                case 401:
                    ConfigGenerator.log("Unauthorized: verify username and password are valid\n");
                    break;

                case 403:
                    ConfigGenerator.log("Forbidden: Failed to get user info.\n");
                    break;

                default:
                    ConfigGenerator.log("Response status: " + response);
                    break;
            }

            System.exit(1);
        }
    }

    private String tagMessageSegment(String tagName, String value) {
        return "<" + tagName + ">" + value + "</" + tagName + ">";
    }

    private String replaceXmlEntities(String buffer) {
        return buffer.replace("<", "&lt;").replace(">", "&gt;");
    }

    private void uploadDescriptor(String descName, String buffer) {
        ConfigGenerator.debug_log("Uploading description:" + descName);

        if (callDeleteFlag) {
            deleteDescriptors();
            callDeleteFlag = false;
        }

        String message = "<DeviceMetaData>";
        message += tagMessageSegment("dvVendorId", vendorId);
        message += tagMessageSegment("dmDeviceType", deviceType);
        message += tagMessageSegment("dmVersion", String.format("%d", fwVersion));
        message += tagMessageSegment("dmName", descName);
        message += tagMessageSegment("dmData", replaceXmlEntities(buffer));
        message += "</DeviceMetaData>";

        ConfigGenerator.debug_log(message);

        String response = sendCloudData("/ws/DeviceMetaData", "POST", message);
        if (responseCode != 0)
        {
            switch (responseCode)
            {
                case 401:
                    ConfigGenerator.log("Unauthorized: verify username and password are valid\n");
                    break;

                case 403:
                    ConfigGenerator.log("Forbidden: Uploading " + descName + " failed, verify that vendor ID is valid and is owned by your account.\n");
                    break;

                default:
                    ConfigGenerator.log("Response status: " + response);
                    break;
            }

            System.exit(1);
        }

        ConfigGenerator.debug_log("Created: " + vendorId + "/" + deviceType + "/" + descName);
        ConfigGenerator.debug_log(response);
    }

}
