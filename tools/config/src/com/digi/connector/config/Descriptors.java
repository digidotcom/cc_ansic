package com.digi.connector.config;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.URL;
import java.util.Map;
import java.util.LinkedList;
import java.util.Base64;
import java.util.Collection;
import java.util.HashMap;

import javax.net.ssl.HttpsURLConnection;

public class Descriptors {

    private final String RCI_VERSION = "1.1";

    private final String RCI_DESCRIPTORS = "<descriptor element=`rci_request` desc=`Remote Command Interface request`>\n"
            + "<attr name=`version` desc=`RCI version of request.  Response will be returned in this versions response format` default=`"
            + RCI_VERSION
            + "`>\n"
            + "<value value=`"
            + RCI_VERSION
            + "` desc=`Version " + RCI_VERSION + "`/></attr>\n";

    private final String SETTING_DESCRIPTOR_DESCRIPTION = "device configuration";
    private final String STATE_DESCRIPTOR_DESCRIPTION = "device state";

    private final String username;
    private final String password;
    private static String deviceType;
    private static Long vendorId;
    private final long fwVersion;
    private int responseCode;

    private final static Config config = Config.getInstance();
    private final static ConfigGenerator options = ConfigGenerator.getInstance();

    public Descriptors(final String username, final String password,
                       final Long vendorId, final String deviceType,
                       final long version) throws IOException  {
        this.username = username;
        this.password = password;
        Descriptors.deviceType = deviceType;
        this.fwVersion = version;

        if(!options.noUploadOption())
            validateUrlName();

        Descriptors.vendorId = vendorId;
        if (vendorId == null) {
        	getVendorId();
        } else {
            options.debug_log(String.format("Vendor ID = 0x%08X (%d)\n", vendorId, vendorId));
        }
        this.responseCode = 0;
    }

    // From: https://en.wikipedia.org/wiki/List_of_XML_and_HTML_character_entity_references#Predefined_entities_in_XML
    public static String encodeEntities(final String string) {
    	final StringBuilder result = new StringBuilder();
	 
		 for (char character: string.toCharArray()) {
			 if (character == '\"') {
				 result.append("&quot;");
			 }
			 else if (character == '&') {
				 result.append("&amp;");
			 }
			 else if (character == '\'') {
				 result.append("&apos;");
			 }
			 else if (character == '<') {
				 result.append("&lt;");
			 }
			 else if (character == '>') {
				 result.append("&gt;");
			 }
			 else {
				 result.append(character);
			 }
		 }
		 return result.toString();
    }

    public void processDescriptors() throws Exception {
    	final boolean uploading = !options.noUploadOption();
    	Map<String, String> descriptors = new HashMap<String, String>(); 
    	
    	options.log("\nProcessing Descriptors, please wait...");
        
        int id = 1;
        for (Group.Type type : Group.Type.values()) {
            Collection<Group> groups = config.getTable(type).groups();

            /* Descriptors must be uploaded even if the group is empty */
            String config_type = type.toLowerName();

            descriptors.put("descriptor/query_" + config_type, generateQueryDescriptor(type, groups, id));
            descriptors.put("descriptor/set_" + config_type, generateSetDescriptor(type, groups, id + 1));
            id += 2;
        }

        if (options.rciLegacyEnabled()){
            descriptors.put("descriptor/reboot", generateRebootDescriptor());
            descriptors.put("descriptor/do_command", generateDoCommandDescriptor());
            descriptors.put("descriptor/set_factory_default", generateSetFactoryDefaultDescriptor());
        }

        descriptors.put("descriptor", generateRootDescriptor());

        if (uploading) {
           deleteDescriptors();
        }
        
        for (Map.Entry<String, String> entry : descriptors.entrySet()) {
        	String name = entry.getKey();
        	String content = entry.getValue();
        	
        	if (uploading) {
        		uploadDescriptor(name, content);
        	} else {
        		saveDescriptor(name, content);
        	}
        }
        final String done = uploading ? "uploaded" : "saved";
        options.log("\nDescriptors were " + done + " successfully.");
    }

    public void deleteDescriptors()
    {
        String target = String.format("/ws/DeviceMetaData?condition=dvVendorId=%d and dmDeviceType=\'%s\' and dmVersion=%d",
                vendorId, deviceType, fwVersion);

        String response = sendCloudData(target.replace(" ", "%20"), "DELETE", null);
        if (responseCode != 0)
        {
            options.debug_log("Response from " + options.getUrlName());
            switch (responseCode)
            {
            case 401:
            options.log("Unauthorized: verify username and password are valid\n");
            break;

            case 403:
            options.log("Forbidden: deleting previous RCI descriptors failed, verify that vendor ID is valid and is owned by your account.\n");
            break;

            default:
            options.log("Response status: " + response);
            break;
            }

            System.exit(1);
        }
        options.debug_log("Deleted target: " + target);
        options.debug_log(String.format("Deleted: 0x%X/%s", vendorId, deviceType));
        options.debug_log(response);
    }

    private String getErrorDescriptors(final int start, final Map<String, String> errors) {
        String descriptors = "";
        int id = start;

        for (String value : errors.values()) {
            descriptors += String.format("<error_descriptor id=`%d` ", id);
            if (value != null)
                descriptors += String.format("desc=`%s` ", Descriptors.encodeEntities(value));

            descriptors += "/>\n";
            id++;
        }

        return descriptors;
    }

    private String generateDoCommandDescriptor() {

        String do_command_descriptor = "<descriptor element=`do_command` bin_id=`6` > ";
        do_command_descriptor +=       "  <error_descriptor id=`1` desc=`Invalid arguments` />";
        do_command_descriptor +=       "  <attr name=`target` type=`string` max=`" + config.getMaxAttributeLength()
                              + "` desc=`The subsystem that the command is forwarded` bin_id=`0` />";
        do_command_descriptor +=       "</descriptor>";

        do_command_descriptor = do_command_descriptor.replace('`', '"');

        return do_command_descriptor;
    }

    private String generateRebootDescriptor() {

        String reboot_descriptor = "<descriptor element=`reboot` desc=`Reboot the device` bin_id=`7` >";
        reboot_descriptor +=       "  <error_descriptor id=`1` desc=`Reboot failed` />";
        reboot_descriptor +=       "</descriptor>\n";

        reboot_descriptor = reboot_descriptor.replace('`', '"');

        return reboot_descriptor;
    }

    private String generateSetFactoryDefaultDescriptor() {

        String set_factory_default_descriptor = "<descriptor element=`set_factory_default` desc=`Set device configuration to factory defaults` bin_id=`8` >";
        set_factory_default_descriptor +=       "  <error_descriptor id=`1` desc=`Set Factory Default failed` />";
        set_factory_default_descriptor +=       "</descriptor>\n";

        set_factory_default_descriptor = set_factory_default_descriptor.replace('`', '"');

        return set_factory_default_descriptor;
    }

    private String itemDescriptors(String prefix, ItemList list) throws Exception {
    	String result = "";
    	LinkedList<Item> items = list.getItems(); 

        for (Item item : items) {
            assert (item instanceof Element) || (item instanceof ItemList);

            int item_id = list.getItems().indexOf(item);
            
            result += item.toString(item_id);

            if (item instanceof Element) {
                Element element = (Element) item;
                
                switch (element.getType()) {
                case ENUM:
                    for (Value value: element.getValues()) {
                        Integer value_id = element.getValues().indexOf(value);

                        result += value.toString(value_id);
                    }
                    result += "</element>\n";
                    break;
                case REF_ENUM:
                    for (Value value: element.getValues()) {
                        result += value.toString(null);
                    }
                    for (Reference ref: element.getRefs()) {
                        result += ref.toString(null);
                    }
                    result += "</element>\n";
                	break;
                default:
                	break;
                }
            } else {
                ItemList subitems = (ItemList) item;
                String subitems_prefix = prefix + "_" + subitems.getName();

                result += itemDescriptors(subitems_prefix, subitems);
                result += "</element>\n";
            }
        }       
    	return result;
    }
    
    private String generateQueryDescriptor(Group.Type type, Collection<Group> groups, int id) throws Exception {
        String desc = (type == Group.Type.SETTING)
        	? SETTING_DESCRIPTOR_DESCRIPTION
        	: STATE_DESCRIPTOR_DESCRIPTION;
        String config_type = type.toLowerName();
        final String rciUserGlobalErrors = getErrorDescriptors(config.getGlobalUserErrorsOffset(), config.getGlobalUserErrors());
        	
        String query_descriptors = String.format("<descriptor element=`query_%s` desc=`Retrieve %s` format=`all_%ss_groups` bin_id=`%d`>\n",
                                                  config_type, desc, config_type, id);

        if (type == Group.Type.SETTING)
            query_descriptors += "  <attr name=`source` type=`enum` desc=`Source of settings returned` bin_id=`0` default=`current` >"
                               + "      <value value=`current` desc=`Current settings` bin_id=`0` />"
                               + "      <value value=`stored` desc=`Settings stored in flash` bin_id=`1` />"
                               + "      <value value=`defaults` desc=`Device defaults` bin_id=`2` />"
                               + "  </attr>"
                               + "  <attr name=`compare_to` type=`enum` desc=`Return only differences from this source` bin_id=`1` default=`none` >"
                               + "      <value value=`none` desc=`Return all settings` bin_id=`0` />"
                               + "      <value value=`current` desc=`Current settings` bin_id=`1` />"
                               + "      <value value=`stored` desc=`Settings stored in flash` bin_id=`2` />"
                               + "      <value value=`defaults` desc=`Device defaults` bin_id=`3` />"
                               + "  </attr>";

        query_descriptors += String.format("<format_define name=`all_%ss_groups`>\n", config_type);

        /*
         * get all errors for query command descriptor. 1. common errors. 2.
         * command errors. 3. user global errors
         *
         * We must offset the error_id for command errors.
         */
        query_descriptors += rciUserGlobalErrors;

        /*
         * get all errors for set command descriptor. 1. common errors. 2.
         * command errors. 3. user global errors
         *
         * We must offset the error_id for command errors.
         */

        int gid = 0;
        for (Group group : groups) {
            String prefix = "group_" + config_type + "_" + group.getName();
            
            query_descriptors += group.toString(gid);
            
            /*
             * Write errors for individual groups
             * 
             *  1. common errors
             *  2. group errors
             *  3. user global error
             *  4. user group error
             *
             * We must offset the error id for group errors.
             */
            query_descriptors += rciUserGlobalErrors;
            query_descriptors += getErrorDescriptors(config.getGroupErrorsOffset(), group.getErrors());
            query_descriptors += itemDescriptors(prefix, group);
            query_descriptors +=  "</descriptor>";
            gid++;
        }
        query_descriptors += "</format_define>\n</descriptor>\n";

        query_descriptors = query_descriptors.replace('`', '"');

        return query_descriptors;
    }

    private String generateSetDescriptor(Group.Type type, Collection<Group> groups, int id) throws Exception {
        String desc = (type == Group.Type.SETTING)
        	? SETTING_DESCRIPTOR_DESCRIPTION
        	: STATE_DESCRIPTOR_DESCRIPTION;
        String config_type = type.toLowerName();
        final String rciUserGlobalErrors = getErrorDescriptors(config.getGlobalUserErrorsOffset(), config.getGlobalUserErrors());
        	
        String set_descriptors = String.format("<descriptor element=`set_%s` desc=`Set %s` format=`all_%ss_groups` bin_id=`%d`>\n",
                                                config_type, desc, config_type, id);

        /*
         * get all errors for set command descriptor. 1. common errors. 2.
         * command errors. 3. user global errors
         *
         * We must offset the error_id for command errors.
         */
        set_descriptors += rciUserGlobalErrors + "</descriptor>";

        set_descriptors = set_descriptors.replace('`', '"');

        return set_descriptors;
    }

    private String generateRootDescriptor() throws Exception {
        String descriptors = RCI_DESCRIPTORS;

        for (Group.Type type : Group.Type.values()) {
            Collection<Group> groups = config.getTable(type).groups();

            String configType = type.toLowerName();

            if (!groups.isEmpty()) {
                descriptors += String.format("<descriptor element=`query_%s` dscr_avail=`true` />\n", configType)
                               + String.format("<descriptor element=`set_%s` dscr_avail=`true` />\n", configType);
            }
        }

        if(options.rciLegacyEnabled()){
            descriptors += String.format("<descriptor element=`reboot` dscr_avail=`true` />\n");

            descriptors += String.format("<descriptor element=`do_command` dscr_avail=`true` />\n");

            descriptors += String.format("<descriptor element=`set_factory_default` dscr_avail=`true` />\n");
        }

        descriptors += getErrorDescriptors(config.getGlobalFatalProtocolErrorsOffset(), config.getGlobalFatalProtocolErrors());
        descriptors += getErrorDescriptors(config.getGlobalProtocolErrorsOffset(), config.getGlobalProtocolErrors())
                + "</descriptor>";

        descriptors = descriptors.replace('`', '"');
        
        return descriptors;
    }

    private String sendCloudData(String target, String method, String message) {
        String response = "";
        String cloud = "https://" + options.getUrlName() + target;
        String credential = username + ":" + password;
        String encodedCredential = Base64.getEncoder().encodeToString(credential.getBytes());
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
                options.log("ERROR: Invalid Device Cloud URL\n");
                options.debug_log("Invalid URL: " + options.getUrlName());
                System.exit(1);
            }
        }

        return response;
    }

    private void getVendorId() {

        options.debug_log("Query vendor ID");

        // Request an existing vendor ID
        String response = sendCloudData("/ws/DeviceVendor", "GET", null);

        if (responseCode == 0) {
            int startIndex = response.indexOf("<dvVendorId>");

            if (startIndex == -1 || startIndex != response.lastIndexOf("<dvVendorId>")) {
                options.debug_log("No vendor ID is found.");

            } else {
                startIndex += "<dvVendorId>".length();
                vendorId = Long.parseUnsignedLong(response.substring(startIndex, response.indexOf("</dvVendorId>")));
            }

        } else {
            options.debug_log("Response from " + options.getUrlName());
            switch (responseCode) {
            case 401:
                options.log("Unauthorized: verify username and password are valid\n");
                break;

            case 403:
                options.log("Forbidden: verify that your account has the \'Embedded Device Customization\' service subscribed.\n");
                break;

            default:
                options.log("Response status: " + response);
                break;
            }
            System.exit(1);
        }

        if (vendorId == null) {
            // Request to create a new Vendor ID
            options.debug_log("Create a new vendor ID");
            sendCloudData("/ws/DeviceVendor", "POST", "<DeviceVendor></DeviceVendor>");

            response = sendCloudData("/ws/DeviceVendor", "GET", null);

            if (responseCode != 0) {
                options.log("Response status: " + response);
            }

            int startIndex = response.indexOf("<dvVendorId>");
            if (startIndex == -1) {
                options.log(
                	"Cannot create a new vendor ID for "
                    + username
                    + "user. User needs to manually create one. Refer to \"Setup your Device Cloud Acount\" section of the Getting started guide to obtain one.");
                System.exit(1);
            }

            if (startIndex != response.lastIndexOf("<dvVendorId>")) {
                options.log(username + " has more than one vendor ID, so please specify the correct one.");
                System.exit(1);
            }

            startIndex += "<dvVendorId>".length();
            vendorId = Long.parseUnsignedLong(response.substring(startIndex, response.indexOf("</dvVendorId>")));
        }

        if (vendorId != null)
            options.log(String.format("Device Cloud registered vendor ID: 0x%X", vendorId));
    }


    private void validateUrlName() {
        options.debug_log("Start validating device cloud url " + options.getUrlName());
        String response = sendCloudData("/ws/UserInfo", "GET", null);

        if (responseCode != 0)
        {
            options.debug_log("Response from " + options.getUrlName());
            switch (responseCode)
            {
                case 401:
                    options.log("Unauthorized: verify username and password are valid\n");
                    break;

                case 403:
                    options.log("Forbidden: Failed to get user info.\n");
                    break;

                default:
                    options.log("Response status: " + response);
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

    private void saveDescriptor(String descName, String buffer) {
        try {
            BufferedWriter fileWriter = new BufferedWriter(new FileWriter(descName.replace("/", "_")+".xml"));
	        String message = "<DeviceMetaData>";
	        message += tagMessageSegment("dvVendorId", vendorId.toString());
	        message += tagMessageSegment("dmDeviceType", deviceType);
	        message += tagMessageSegment("dmVersion", String.format("%d", fwVersion));
	        message += tagMessageSegment("dmName", descName);
	        message += tagMessageSegment("dmData", buffer);
	        message += "</DeviceMetaData>";

			fileWriter.write(message);
			fileWriter.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
    }

    private void uploadDescriptor(String descName, String buffer) {
        options.debug_log("Uploading descriptor:" + descName);

        String message = "<DeviceMetaData>";
        message += tagMessageSegment("dvVendorId", vendorId.toString());
        message += tagMessageSegment("dmDeviceType", deviceType);
        message += tagMessageSegment("dmVersion", String.format("%d", fwVersion));
        message += tagMessageSegment("dmName", descName);
        message += tagMessageSegment("dmData", replaceXmlEntities(buffer));
        message += "</DeviceMetaData>";

        String response = sendCloudData("/ws/DeviceMetaData", "POST", message);
        if (responseCode != 0)
        {
            options.debug_log("Response from " + options.getUrlName());
            switch (responseCode)
            {
                case 401:
                    options.log("Unauthorized: verify username and password are valid\n");
                    break;

                case 403:
                    options.log("Forbidden: Uploading " + descName + " failed, verify that vendor ID is valid and is owned by your account.\n");
                    break;

                default:
                    options.log("Response status: " + response);
                    break;
            }

            System.exit(1);
        }

        options.debug_log("Created: " + vendorId + "/" + deviceType + "/" + descName);
        options.debug_log(response);
        /* prevent error HTTP/1.1 429 Too Many Requests */
        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    public static Long vendorId() {
        return vendorId;
    }

    public static String deviceType() {
        return deviceType;
    }
}
