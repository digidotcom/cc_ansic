package com.digi.connector.config;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.LinkedHashMap;
import java.util.LinkedList;

import com.digi.connector.config.ConfigGenerator.UseNames;
import com.digi.connector.config.Element.ElementType;

public abstract class FileGenerator {

    protected static String TOOL_NAME = "RCI Generator";

    protected final static String HEADER_FILENAME = "connector_api_remote.h";

    protected final static String CONNECTOR_PREFIX = "connector";
    protected final static String CCAPI_PREFIX = "ccapi";

    protected final static String DEFINE = "#define ";
    protected final static String INCLUDE = "#include ";
    protected final static String ERROR = "error";

    protected final static String CONNECTOR_GLOBAL_HEADER = "\n\n#include \"connector_api.h\"\n\n";
    protected final static String FLOAT_HEADER = "\"float.h\"\n";

    protected final static String TYPEDEF_ENUM = "typedef enum {\n";

    protected final static String GLOBAL_RCI_ERROR = "connector_rci_error";
    protected final static String GLOBAL_ERROR = "connector_global_error";

    protected final static String CONNECTOR_REMOTE_ALL_STRING = "connector_remote_all_strings";
    protected final static String CONNECTOR_REMOTE_GROUP_TABLE = "connector_group_table";

    protected final static String COUNT_STRING = "COUNT";
    protected final static String OFFSET_STRING = "OFFSET";

    protected final static String CHAR_CONST_STRING = "static char CONST * CONST ";
    protected final static String CONNECTOR_CALLBACK_STATUS = "\nconnector_callback_status_t ";
    protected final static String ID_T_STRING = "_id_t;\n\n";
    protected final static String TYPEDEF_STRUCT = "\ntypedef struct {\n";

    /* Do not change these (if you do, you also need to update connector_remote.h */
    protected final static String RCI_PARSER_USES = "RCI_PARSER_USES_";
    protected final static String RCI_INFO_T = "ccapi_rci_info_t * const info";
    protected final static String RCI_FUNCTION_T = "(ccapi_rci_function_t)";
    protected final static String CCAPI_RCI_FUNCTION_T = "(ccapi_rci_function_t)";

    protected final static String RCI_PARSER_USES_ERROR_DESCRIPTIONS = RCI_PARSER_USES + "ERROR_DESCRIPTIONS\n";
    protected final static String RCI_PARSER_USES_STRINGS = RCI_PARSER_USES + "STRINGS\n";
    protected final static String RCI_PARSER_USES_UNSIGNED_INTEGER = RCI_PARSER_USES + "UNSIGNED_INTEGER\n";

    protected final static String RCI_PARSER_DATA = "CONNECTOR_RCI_PARSER_INTERNAL_DATA";

    protected final static String RCI_LEGACY_DEFINE = "\n#define RCI_LEGACY_COMMANDS\n";
    protected final static String RCI_PARSER_DEFINE = "\n#define RCI_ENUMS_AS_STRINGS\n";
    protected final static String RCI_ERROR_NOT_AVAILABLE = "connector_rci_error_not_available = -1,\n";

    /* Following enum has to be in syncr. with sendDescriptors() function */
    protected final static String RCI_QUERY_COMMAND_ATTRIBUTE_ID_T = "\ntypedef enum {\n" +
    "  rci_query_setting_attribute_id_source,\n" +      /* 'source' attribute is bin_id=0 in the uploaded descriptor for query command */
    "  rci_query_setting_attribute_id_compare_to,\n" +  /* 'compare_to' attribute is bin_id=1 in the uploaded descriptor for query command */
    "  rci_query_setting_attribute_id_count\n" +
    "} rci_query_setting_attribute_id_t;\n";

    protected final static String CONNECTOR_REMOTE_GROUP_TYPE = "\ntypedef enum {\n" +
    "    connector_remote_group_setting,\n" +
    "    connector_remote_group_state\n" +
    "} connector_remote_group_type_t;\n";

    protected final static String CONNECTOR_ELEMENT_ACCESS = "\ntypedef enum {\n" +
    "    connector_element_access_read_only,\n" +
    "    connector_element_access_write_only,\n" +
    "    connector_element_access_read_write\n" +
    "} connector_element_access_t;\n";

    protected final static String RCI_QUERY_SETTING_ATTRIBUTE_SOURCE = "\ntypedef enum {\n" +
    "    rci_query_setting_attribute_source_current,\n" +
    "    rci_query_setting_attribute_source_stored,\n" +
    "    rci_query_setting_attribute_source_defaults\n" +
    "} rci_query_setting_attribute_source_t;\n";

    protected final static String RCI_QUERY_SETTING_ATTRIBUTE_COMPARE_TO = "\ntypedef enum {\n" +
    "    rci_query_setting_attribute_compare_to_none,\n" +
    "    rci_query_setting_attribute_compare_to_current,\n" +
    "    rci_query_setting_attribute_compare_to_stored,\n" +
    "    rci_query_setting_attribute_compare_to_defaults\n" +
    "} rci_query_setting_attribute_compare_to_t;\n";

    protected final static String CONNECTOR_REMOTE_CONFIG_T = "\ntypedef struct {\n" +
    "  void * user_context;\n" +
    "  connector_remote_action_t CONST action;\n" +
    "  connector_remote_attribute_t CONST attribute;\n" +
    "  connector_remote_group_t CONST group;\n" +
    "  connector_remote_list_t CONST list;\n" +
    "  connector_remote_element_t CONST element;\n" +
    "  unsigned int error_id;\n" +
    "\n" +
    "  struct {\n" +
    "      connector_bool_t compare_matches;\n" +
    "      char const * error_hint;\n" +
    "      connector_element_value_t * element_value;\n" +
    "  } response;\n" +
    "} connector_remote_config_t;\n";

    protected final static String CONNECTOR_REMOTE_CONFIG_CANCEL_T = "\ntypedef struct {\n" +
    "  void * user_context;\n" +
    "} connector_remote_config_cancel_t;\n";

    protected final static String CONNECTOR_REMOTE_GROUP_TABLE_T = "\ntypedef struct connector_remote_group_table {\n" +
    "  connector_group_t CONST * groups;\n" +
    "  size_t count;\n" +
    "} connector_remote_group_table_t;\n\n";

    protected String filePath = "";
    protected String generatedFile = "";

    protected BufferedWriter fileWriter;
    protected String configType;
    private int prevRemoteStringLength;
    private Boolean isFirstRemoteString;
    protected String customPrefix;

    public FileGenerator(String directoryPath, String file_name, ConfigGenerator.FileType fileType) throws IOException {

        if (directoryPath != null) {
            filePath = directoryPath;
            /* add a / if last char is not / */
            if (!directoryPath.endsWith("/")) filePath += '/';
        }

        generatedFile = file_name;

        if(!ConfigGenerator.noBackupOption())
            checkPreviousBuild(filePath + generatedFile);

        fileWriter = new BufferedWriter(new FileWriter(filePath + generatedFile));

        writeHeaderComment(fileWriter);
        isFirstRemoteString = true;
        customPrefix = ConfigGenerator.getCustomPrefix();
    }

    protected void checkPreviousBuild(String new_path) throws IOException {

        File new_file = new File(new_path);
        int i = 0;

        while(new_file.isFile()){
            i++;
            new_file = new File(new_path + "_bkp_" + i );
        }
        if(i>0){
            String dest = new_path + "_bkp_" + i;
            InputStream in = new FileInputStream(new_path);
            OutputStream out = new FileOutputStream(dest);
            byte[] buf = new byte[1024];
            int len;
            while ((len = in.read(buf)) > 0) {
                out.write(buf, 0, len);
            }
            in.close();
            out.close();

            ConfigGenerator.log("Existing file " + new_path + " saved as: " + dest);
        }
	}

    abstract void writeHeaderComment(BufferedWriter bufferWriter) throws IOException;
    abstract void generateFile(ConfigData configData) throws Exception;

    private void writeOnOffBooleanEnum() throws IOException {

        String enumString = "";

        for (Element.ElementType type : Element.ElementType.values()) {
            if (type.isSet()) {
                switch (type) {
                case ON_OFF:
                    enumString += "\ntypedef enum {\n"
                                    + "    connector_off,\n"
                                    + "    connector_on\n"
                                    + "} connector_on_off_t;\n";
                    break;

                default:
                  break;
                }
            }
        }

        fileWriter.write(enumString);
    }

    private void writeElementTypeEnum() throws IOException {

        String enumString = "\n\ntypedef enum {\n";
        Boolean isFirstEnum = true;
        int previous = -1;

        for (Element.ElementType type : Element.ElementType.values()) {
            if (type.isSet()) {
            	int current = type.toValue();
            	
                if (!isFirstEnum) {
                    enumString += ",\n";
                } else {
                	isFirstEnum = false;
                }

                enumString += "    connector_element_type_" + type.toName().toLowerCase();

                if (current != (previous + 1)) {
                    enumString += String.format(" = %d", current);
                }
                previous = current;
            }
        }

        enumString += "\n} connector_element_value_type_t;\n";
        fileWriter.write(enumString);
    }

    private String enumStructureString() {
        String string = TYPEDEF_STRUCT
                      + "    size_t count;\n";
        string += "} connector_element_value_enum_t;\n";

        return string;
    }

    private String sanitizeName(String name) {
    	return name.replace('-', '_');
    }

    protected void writeEnumValues(ConfigData configData, BufferedWriter bufferWriter, String prefix, ItemList list) throws Exception {
        for (Item item : list.getItems()) {
            assert (item instanceof Element) || (item instanceof ItemList);

            if (item instanceof Element) {
                Element element = (Element) item;

                if (Element.ElementType.toElementType(element.getType()) == Element.ElementType.ENUM) {
                    bufferWriter.write(TYPEDEF_ENUM);
                    boolean previousempty = false;
                    
                    int i = 0;
                    for (ValueStruct value : element.getValues()) {
                        if (value.getName().equals("")) {
                            previousempty = true;
                        }
                        else {
                        	String partial = getCcapiEnumString(prefix + "_" + element.getName() + "_" + sanitizeName(value.getName()));
                        	
                        	if (previousempty) {
                        		partial += " = " + i;
                        		previousempty = false;
                            }
                            bufferWriter.write(partial + ",\n");
                        }
                        i += 1;
                    }
                    
                    bufferWriter.write(endCcapiEnumString(prefix + "_" + element.getName()));
                }
            } else {
                ItemList items = (ItemList) item;
            	
                writeEnumValues(configData, bufferWriter, prefix + "_" + items.getName(), items); 
            }
        }
    }

    protected void writeItemPrototypes(ConfigData configData, BufferedWriter bufferWriter, String prefix, ItemList list, String retvalErrorType) throws Exception {
	    for (Item item : list.getItems()) {
            assert (item instanceof Element) || (item instanceof ItemList);
	
	        if (item instanceof Element) {
	            Element element = (Element) item;
	
	            String protoType = "";
	            switch (ElementType.toElementType(element.getType())) {
	                case UINT32:
	                case HEX32:
	                case X_HEX32:
	                    protoType += "uint32_t";
	                    break;
	                case INT32:
	                    protoType += "int32_t";
	                    break;
	                case FLOAT:
	                    protoType += "float";
	                    break;
	                case ON_OFF:
	                    protoType += "ccapi_on_off_t";
	                    break;
	                case ENUM:
	                    if (ConfigGenerator.rciParserOption()) {
	                        protoType += "char const *";
	                    } else {
	                        protoType += String.format("%s%s_%s_%s_%s_id_t",
	                        	customPrefix, CCAPI_PREFIX, configType, prefix, element.getName());
	                    }
	                    break;
	                case IPV4:
	                case FQDNV4:
	                case FQDNV6:
	                case DATETIME:
	                case STRING:
	                case MULTILINE_STRING:
	                case PASSWORD:
	                case MAC_ADDR:
	                    protoType += "char const *";
	                    break;
	                case BOOLEAN:
	                    protoType += "ccapi_bool_t";
	                    break;
	                default:
	                    break;
	            }
	            
	            String element_prototype ="";
	            switch (ElementType.toElementType(element.getType())) {
	                case PASSWORD:
	                    element_prototype += String.format("\n%s%srci_%s_%s_%s_get    NULL",
	                    	DEFINE, customPrefix, configType, prefix, element.getName());
	                    break;
	                default:
	                    element_prototype += String.format("%s%srci_%s_%s_%s_get(%s, %s * const value);",
	                    	retvalErrorType, customPrefix, configType, prefix, element.getName(), RCI_INFO_T, protoType);
	                    break;
	            }
	
	            if (getAccess(element.getAccess()).equalsIgnoreCase("read_only")) {
	                element_prototype += String.format("\n%s%srci_%s_%s_%s_set    NULL \n",
                		DEFINE, customPrefix, configType, prefix, element.getName());
	            }
	            else {
	                String value_type_modifier = "";
	                switch (ElementType.toElementType(element.getType())) {
	                    case ENUM:
	                        if (ConfigGenerator.rciParserOption()) {
	                            break;
	                        }
	                        /* Intentional fall-thru */
	                    case UINT32:
	                    case HEX32:
	                    case X_HEX32:
	                    case INT32:
	                    case FLOAT:
	                    case ON_OFF:
	                    case BOOLEAN:
	                        value_type_modifier = "const *";
	                        break;
	                    case IPV4:
	                    case FQDNV4:
	                    case FQDNV6:
	                    case DATETIME:
	                    case STRING:
	                    case MULTILINE_STRING:
	                    case PASSWORD:
	                    case MAC_ADDR:
	                    case LIST:
	                        break;
	                }
	                element_prototype += String.format("%s%srci_%s_%s_%s_set(%s, %s " + value_type_modifier + "const value);\n",
	                	retvalErrorType, customPrefix, configType, prefix, element.getName(), RCI_INFO_T, protoType);
	            }
	            bufferWriter.write(element_prototype);
	        } else {
	            ItemList items = (ItemList) item;
	        	
	            writeItemPrototypes(configData, bufferWriter, prefix + "_" + items.getName(), items, retvalErrorType); 
	        }
	    }
    }
    
    protected void writePrototypes(ConfigData configData, BufferedWriter bufferWriter) throws Exception {
        /* Global errors for session/action_start/end functions */
        bufferWriter.write("\n" + TYPEDEF_ENUM);
        writeCcapiErrorHeader(null,0, getCcapiEnumString("global" + "_" + ERROR), null, bufferWriter);
        writeCcapiErrorHeader("rci",1, getCcapiEnumString("global" + "_" + ERROR), configData.getRciGlobalErrors(), bufferWriter);
        writeCcapiErrorHeader("global",1, getCcapiEnumString("global" + "_" + ERROR), configData.getUserGlobalErrors(), bufferWriter);
        bufferWriter.write(endCcapiEnumString("global" + "_" + ERROR));

        /* session/action_start/end functions' prototypes */
        String globalRetvalErrorType =  "\n" + customPrefix + CCAPI_PREFIX + "_" + "global" + "_error_id_t ";
        String session_prototype = globalRetvalErrorType;
        session_prototype += customPrefix + "rci_session_start_cb(" + RCI_INFO_T + ");";
        session_prototype += globalRetvalErrorType;
        session_prototype += customPrefix + "rci_session_end_cb(" + RCI_INFO_T + ");\n";

        session_prototype += globalRetvalErrorType;
        session_prototype += customPrefix + "rci_action_start_cb(" + RCI_INFO_T + ");";
        session_prototype += globalRetvalErrorType;
        session_prototype += customPrefix + "rci_action_end_cb(" + RCI_INFO_T + ");\n";
        session_prototype += "\n";

        if (ConfigGenerator.rciLegacyEnabled()) {
            session_prototype += globalRetvalErrorType;
            session_prototype += customPrefix + "rci_do_command_cb(" + RCI_INFO_T + ");\n";
            session_prototype += globalRetvalErrorType;
            session_prototype += customPrefix + "rci_set_factory_defaults_cb(" + RCI_INFO_T + ");\n";
            session_prototype += globalRetvalErrorType;
            session_prototype += customPrefix + "rci_reboot_cb(" + RCI_INFO_T + ");\n";
            session_prototype += "\n";
        }
        bufferWriter.write(session_prototype);

        for (GroupType type : GroupType.values()) {
            LinkedList<Group> groups = null;

            configType = type.toString().toLowerCase();
            groups = configData.getConfigGroup(configType);

            if (!groups.isEmpty()) {
                for (Group group : groups) {
                	writeEnumValues(configData, bufferWriter, group.getName(), group);
                }

                /* Write typedef enum for group errors */
                for (Group group : groups) {
                    bufferWriter.write(TYPEDEF_ENUM);

                    writeCcapiErrorHeader(null,0, getCcapiEnumString(group.getName() + "_" + ERROR), null, bufferWriter);
                    writeCcapiErrorHeader("rci",1, getCcapiEnumString(group.getName() + "_" + ERROR), configData.getRciGlobalErrors(), bufferWriter);
                    writeCcapiErrorHeader("global",1, getCcapiEnumString(group.getName() + "_" + ERROR), configData.getUserGlobalErrors(), bufferWriter);

                    if (!group.getErrors().isEmpty()){
                        LinkedHashMap<String, String> errorMap = group.getErrors();
                        int index = 0;

                        for (String key : errorMap.keySet()) {
                            String enumString = getCcapiEnumString(group.getName() + "_" + ERROR + "_" + key);

                            if (index++ == 0) {
                                /*Set start index to the global count */
                                String startIndexString = COUNT_STRING;
                                if(bufferWriter == FileSource.getHeaderWriter()){
                                    startIndexString += "_INDEX";
                                    enumString += " = " + GLOBAL_ERROR + "_" + startIndexString;
                                }
                                else{
                                    enumString += ", " + COMMENTED(" User defined (group errors)");
                                }
                            }
                            else {
                                enumString += ",\n";
                            }
                            bufferWriter.write(enumString);
                        }
                    }
                    bufferWriter.write(endCcapiEnumString(group.getName() + "_" + ERROR));
                }
                
                for (Group group : groups) {
                    String retvalErrorType = "\n" + customPrefix + CCAPI_PREFIX + "_" + configType + "_" + group.getName() + "_error_id_t ";
                    
                    String group_prototype = 
                    		retvalErrorType + String.format("%srci_%s_%s_start(%s);",customPrefix,configType,group.getName(),RCI_INFO_T) +
                    		retvalErrorType + String.format("%srci_%s_%s_end(%s);\n",customPrefix,configType,group.getName(),RCI_INFO_T);
                    bufferWriter.write(group_prototype);

                    writeItemPrototypes(configData, bufferWriter, group.getName(), group, retvalErrorType);
                }
            }
            bufferWriter.write("\n");
        }
    }

    void writeItemFunctionsCB(ConfigData configData, BufferedWriter bufferWriter, String prefix, ItemList list) throws Exception {
        for (Item item : list.getItems()) {
            assert (item instanceof Element) || (item instanceof ItemList);

            if (item instanceof Element) {
                Element element = (Element) item;

                String element_function ="";
                String FType = "";
                String value = "";
                
                switch (ElementType.toElementType(element.getType())){
                    case UINT32:
                        if (element.getMin()!=null)
                           value += "*value = " + element.getMin();
                        else
                            value += "*value = 123";
                        FType += "uint32_t";
                        break;
                    case HEX32:
                    case X_HEX32:
                        value += "*value = 0x20101010";
                        FType += "uint32_t";
                        break;
                    case INT32:
                        if (element.getMin()!=null)
                            value += "*value = " + element.getMin();
                        else
                            value += "*value = 15";
                        FType += "int32_t";
                        break;
                    case FLOAT:
                        value += "*value = 1.2";
                        FType += "float";
                        break;
                    case ON_OFF:
                        value += "*value = CCAPI_ON";
                        FType += "ccapi_on_off_t";
                        break;
                    case ENUM:
                        ValueStruct first_value = element.getValues().get(0);
                        if (ConfigGenerator.rciParserOption()) {
                            value += "*value = \"" + first_value.getName() + "\"";
                            FType += "char const *";
                        } else {
                            value += "*value = " + customPrefix.toUpperCase() + "CCAPI_" + configType.toUpperCase() + "_" + prefix.toUpperCase() + "_" + element.getName().toUpperCase() + "_" + first_value.getName().replace(" ", "_").toUpperCase();
                            FType += String.format("%s%s_%s_%s_%s_id_t", customPrefix, CCAPI_PREFIX, configType, prefix, element.getName());
                        }

                        break;
                    case IPV4:
                    case FQDNV4:
                    case FQDNV6:
                        value += "*value = \"192.168.1.1\"";
                        FType += "char const *";
                        break;
                    case DATETIME:
                        value += "*value = \"2002-05-30T09:30:10-0600\";";
                        FType += "char const *";
                        break;
                    case STRING:
                    case MULTILINE_STRING:
                        value += "*value = \"String\"";
                    case PASSWORD:
                        FType += "char const *";
                        break;
                    case MAC_ADDR:
                        value += "*value = \"00:04:9D:AB:CD:EF\"";
                        FType += "char const *";
                        break;
                    case BOOLEAN:
                        value += "*value = CCAPI_TRUE";
                        FType += "ccapi_bool_t";
                        break;
                default:
                    break;
                }
                
                if (ElementType.toElementType(element.getType()) != ElementType.PASSWORD) {
                    element_function += setFunction(prefix, String.format("%srci_%s_%s_%s_get(%s, %s * const value)", 
                    	customPrefix, configType, prefix, element.getName(), RCI_INFO_T, FType), value);
                }
                
                if (!getAccess(element.getAccess()).equalsIgnoreCase("read_only")) {
                    String value_type_modifier = "";
                    
                    switch (ElementType.toElementType(element.getType())) {
                        case ENUM:
                            if (ConfigGenerator.rciParserOption()) {
                                break;
                            }
                            /* Intentional fall-thru */
                        case UINT32:
                        case HEX32:
                        case X_HEX32:
                        case INT32:
                        case FLOAT:
                        case ON_OFF:
                        case BOOLEAN:
                            value_type_modifier = "const *";
                            break;
                        case IPV4:
                        case FQDNV4:
                        case FQDNV6:
                        case DATETIME:
                        case STRING:
                        case MULTILINE_STRING:
                        case PASSWORD:
                        case MAC_ADDR:
                        case LIST:
                            break;
                    }
                    element_function += setFunction(prefix, String.format("%srci_%s_%s_%s_set(%s, %s " + value_type_modifier + "const value)",
                    	customPrefix, configType, prefix, element.getName(), RCI_INFO_T, FType), "UNUSED_PARAMETER(value)");
                }
                bufferWriter.write(element_function);
            } else {
                ItemList items = (ItemList) item;
            	
                writeItemFunctionsCB(configData, bufferWriter, prefix + "_" + items.getName(), items); 
            }
        }
    }
    
    void writeFunctionsCB(ConfigData configData, BufferedWriter bufferWriter) throws Exception {
        String session_function = 
        	setFunction("_SESSION_GROUP_", customPrefix + "rci_session_start_cb(" + RCI_INFO_T + ")", null) +
        	setFunction("_SESSION_GROUP_", customPrefix + "rci_session_end_cb(" + RCI_INFO_T + ")", null) +
        	setFunction("_SESSION_GROUP_", customPrefix + "rci_action_start_cb(" + RCI_INFO_T + ")", null) +
        	setFunction("_SESSION_GROUP_", customPrefix + "rci_action_end_cb(" + RCI_INFO_T + ")", null);

        if (ConfigGenerator.rciLegacyEnabled()) {
            session_function += 
            	setFunction("_SESSION_GROUP_", "rci_do_command_cb(" + RCI_INFO_T + ")", null) + 
            	setFunction("_SESSION_GROUP_", "rci_set_factory_defaults_cb(" + RCI_INFO_T + ")", null) +
            	setFunction("_SESSION_GROUP_", "rci_reboot_cb(" + RCI_INFO_T + ")", null);
        }
        bufferWriter.write(session_function);

        for (GroupType type : GroupType.values()) {
            configType = type.toString().toLowerCase();
            
            LinkedList<Group> groups = configData.getConfigGroup(configType);
            for (Group group : groups) {
                String group_function = setFunction(group.getName(), String.format("%srci_%s_%s_start(%s)",customPrefix,configType,group.getName(),RCI_INFO_T),null);
                group_function += setFunction(group.getName(), String.format("%srci_%s_%s_end(%s)",customPrefix,configType,group.getName(),RCI_INFO_T),null);
                bufferWriter.write(group_function);
               
                writeItemFunctionsCB(configData, bufferWriter, group.getName(), group);
            }
        }
    }

    private String UNUSED(String parameter) {
        return "UNUSED_PARAMETER(" + parameter + ");\n";
    }

    private String setFunction (String groupName, String parameter, String value){

    	String name = (groupName.equals("_SESSION_GROUP_"))
			? "global"
			: configType + "_" + groupName;
        String retvalErrorType = "\n" + customPrefix + CCAPI_PREFIX + "_" + name + "_error_id_t ";
        String PRINTF = "    printf(\"    Called '%s'\\n\", __FUNCTION__);\n";
        String RETURN_CONTINUE = String.format("    return %sCCAPI_GLOBAL_ERROR_NONE;\n}\n", customPrefix.toUpperCase());
        String function = retvalErrorType + parameter + "\n{\n    " + UNUSED("info") + PRINTF;
        
        if (value != null) {
            function += "    " + value + ";\n";
        }
        function += RETURN_CONTINUE;
        
        return function;
    }

    private void writeElementValueStruct() throws IOException {

        String headerString = "";
        String structString = "";
        String elementValueStruct = "";
        String defineElementString = "";

        int optionCount = 0;

        Boolean isUnsignedIntegerDefined = false;
        Boolean isStringDefined = false;
        Boolean isEnumValueStructDefined = false;

        for (Element.ElementType type : Element.ElementType.values()) {
            if (type.isSet()) {
                switch (type) {
                case UINT32:
                case HEX32:
                case X_HEX32:
                    if (!isUnsignedIntegerDefined) {
                        /* if not defined yet, then define it */
                        structString += TYPEDEF_STRUCT
                            + "   uint32_t min_value;\n"
                            + "   uint32_t max_value;\n"
                            + "} connector_element_value_unsigned_integer_t;\n";
                        elementValueStruct += "    uint32_t unsigned_integer_value;\n";
                        if(ConfigGenerator.rciParserOption())
                            defineElementString += "#define UNSIGNED_INTEGER_VALUE\n";
                        isUnsignedIntegerDefined = true;
                        optionCount++;
                    }
                    break;

                case INT32:
                    structString += TYPEDEF_STRUCT
                                    + "   int32_t min_value;\n"
                                    + "   int32_t max_value;\n"
                                    + "} connector_element_value_signed_integer_t;\n";
                    elementValueStruct += "    int32_t signed_integer_value;\n";
                    if(ConfigGenerator.rciParserOption())
                        defineElementString += "#define SIGNED_INTEGER_VALUE\n";
                    optionCount++;
                    break;

                case ENUM:
                    if (!isEnumValueStructDefined) {
                        structString += enumStructureString();
                        isEnumValueStructDefined = true;
                    }
                    elementValueStruct += "    unsigned int enum_value;\n";
                    if(ConfigGenerator.rciParserOption())
                        defineElementString += "#define ENUM_VALUE\n";
                    optionCount++;
                    break;

                case FLOAT:
                    structString += TYPEDEF_STRUCT
                                    + "    float min_value;\n"
                                    + "    float max_value;\n"
                                    + "} connector_element_value_float_t;\n";
                    elementValueStruct += "    float float_value;\n";
                    if(ConfigGenerator.rciParserOption())
                        defineElementString += "#define FLOAT_VALUE\n";
                    optionCount++;
                    break;

                case ON_OFF:
                    if (!isEnumValueStructDefined) {
                        /* rci parser needs this structure for on/off type */
                        structString += enumStructureString();
                        isEnumValueStructDefined = true;
                    }
                    elementValueStruct += "    connector_on_off_t  on_off_value;\n";
                    if(ConfigGenerator.rciParserOption())
                        defineElementString += "#define ON_OFF_VALUE\n";
                    optionCount++;
                    break;

                case BOOLEAN:
                    if (!isEnumValueStructDefined) {
                        /* rci parser needs this structure for boolean type */
                        structString += enumStructureString();
                        isEnumValueStructDefined = true;
                    }
                    elementValueStruct += "    connector_bool_t  boolean_value;\n";
                    if(ConfigGenerator.rciParserOption())
                        defineElementString += "#define BOOLEAN_VALUE\n";
                    optionCount++;
                    break;

                default:
                    if (!isStringDefined) {
                        /* if not defined yet then define it */
                        structString += TYPEDEF_STRUCT
                                        + "    size_t min_length_in_bytes;\n"
                                        + "    size_t max_length_in_bytes;\n"
                                        + "} connector_element_value_string_t;\n";
                        elementValueStruct += "    char const * string_value;\n";
                        if(ConfigGenerator.rciParserOption())
                            defineElementString += "#define STRING_VALUE\n";
                        isStringDefined = true;
                        optionCount++;
                    }
                    break;
                }
            }
        }

        if(ConfigGenerator.rciLegacyEnabled() && !isStringDefined) {
            /* if not defined yet then define it */
            structString += TYPEDEF_STRUCT
                            + "    size_t min_length_in_bytes;\n"
                            + "    size_t max_length_in_bytes;\n"
                            + "} connector_element_value_string_t;\n";
            elementValueStruct += "    char const * string_value;\n";
            isStringDefined = true;
            optionCount++;
        }

        headerString += structString;

        if (optionCount > 1) {
            headerString += "\n\ntypedef union {\n";
        } else {
            headerString += "\n\ntypedef struct {\n";
        }

        headerString += elementValueStruct + "} connector_element_value_t;\n";

        if(ConfigGenerator.rciParserOption())
            headerString += defineElementString;
        fileWriter.write(headerString);
    }


    private void writeDefineOptionHeader(ConfigData configData) throws IOException {
        /* if fileType == SOURCE, we should not this function */
        String headerString = "";

        if (!ConfigGenerator.excludeErrorDescription()) {
            headerString += DEFINE + RCI_PARSER_USES_ERROR_DESCRIPTIONS;
        }

        String unsignedIntegerString = null;
        String stringsString = null;

        String floatInclude = null;

        for (Element.ElementType type : Element.ElementType.values()) {
            if (type.isSet()) {
                headerString += DEFINE + RCI_PARSER_USES + type.toName().toUpperCase() + "\n";

                switch (type) {
                case UINT32:
                case HEX32:
                case X_HEX32:
                    if (unsignedIntegerString == null) {
                        unsignedIntegerString = DEFINE + RCI_PARSER_USES_UNSIGNED_INTEGER;
                    }
                    break;

                case INT32:
                case ENUM:
                    break;
                case FLOAT:
                    floatInclude = INCLUDE + FLOAT_HEADER;
                    break;

                case ON_OFF:
                case BOOLEAN:
                    break;
                default:
                    if (stringsString == null) {
                        stringsString = DEFINE + RCI_PARSER_USES_STRINGS;
                    }
                    break;
                }
            }
        }

        if (unsignedIntegerString != null) headerString += unsignedIntegerString;
        if (stringsString != null) headerString += stringsString;

        if (floatInclude != null)
            headerString += "\n\n" + floatInclude;

        fileWriter.write(headerString);
    }

    protected void writeDefinesAndStructures(ConfigData configData) throws IOException {

        writeDefineOptionHeader(configData);

        if(ConfigGenerator.rciLegacyEnabled()){
            fileWriter.write(RCI_LEGACY_DEFINE);
        }
        if(ConfigGenerator.rciParserOption()){
            fileWriter.write(RCI_PARSER_DEFINE);
        }

        fileWriter.write(String.format("%sRCI_COMMANDS_ATTRIBUTE_MAX_LEN %d\n", DEFINE, ConfigData.AttributeMaxLen()));
        fileWriter.write(String.format("%sRCI_LIST_MAX_DEPTH %d\n", DEFINE, configData.getMaxDepth()));


        writeOnOffBooleanEnum();
        writeElementTypeEnum();
        writeElementValueStruct();

        String list_start = "";
        String list_end = "";
        	
        if (configData.getMaxDepth() > 0) {
        	list_start = "    connector_request_id_remote_config_list_start,\n";
        	list_end   = "    connector_request_id_remote_config_list_end,\n";
        }
        
        fileWriter.write("\ntypedef enum {\n" +
                         "    connector_request_id_remote_config_session_start,\n" +
                         "    connector_request_id_remote_config_action_start,\n" +
                         "    connector_request_id_remote_config_group_start,\n" +
                         list_start +
                         "    connector_request_id_remote_config_element_process,\n" +
                         list_end +
                         "    connector_request_id_remote_config_group_end,\n" +
                         "    connector_request_id_remote_config_action_end,\n" +
                         "    connector_request_id_remote_config_session_end,\n" +
                         "    connector_request_id_remote_config_session_cancel");
        
        if (ConfigGenerator.rciLegacyEnabled()){
            fileWriter.write(",\n    connector_request_id_remote_config_do_command,\n" +
                             "    connector_request_id_remote_config_reboot,\n" +
                             "    connector_request_id_remote_config_set_factory_def");
        }
        fileWriter.write("\n} connector_request_id_remote_config_t;\n");
        
        fileWriter.write(
        	"\n" +
        	"/* deprecated */\n" +
        	"#define connector_request_id_remote_config_group_process connector_request_id_remote_config_element_process\n"
        	);

        fileWriter.write("\ntypedef enum {\n" +
                         "    connector_remote_action_set,\n" +
                         "    connector_remote_action_query");
        if (ConfigGenerator.rciLegacyEnabled()){
            fileWriter.write(",\n    connector_remote_action_do_command,\n" +
                             "    connector_remote_action_reboot,\n" +
                             "    connector_remote_action_set_factory_def");
        }
        fileWriter.write("\n} connector_remote_action_t;\n");

        fileWriter.write(CONNECTOR_REMOTE_GROUP_TYPE);

        fileWriter.write(CONNECTOR_ELEMENT_ACCESS);

        writeGroupElementStructs(configData);

        String const_name = "";
        if (ConfigGenerator.useNamesOption(UseNames.COLLECTIONS)) {
            const_name = "    char const * name;\n";
            fileWriter.write("\n"+ DEFINE + "RCI_PARSER_USES_COLLECTION_NAMES\n");
        }
        
        fileWriter.write(
        	"\n" + 
        	"typedef struct {\n" +
            "    connector_remote_group_type_t type;\n" +
            "    unsigned int id;\n" +
            "    unsigned int index;\n" +
            const_name +
            "} connector_remote_group_t;\n"
            );

        if (ConfigGenerator.useNamesOption(UseNames.ELEMENTS)) {
            fileWriter.write("\n" + DEFINE + "RCI_PARSER_USES_ELEMENT_NAMES\n");
        } else {
	        fileWriter.write(
        		"\n" + 
        		"typedef struct {\n" +
	            "  unsigned int id;\n" +
	            "  connector_element_value_type_t type;\n" +
	            "  connector_element_value_t * value;\n" +
	            "} connector_remote_element_t;\n"
	            );
        }
        
        fileWriter.write(RCI_QUERY_SETTING_ATTRIBUTE_SOURCE);
        fileWriter.write(RCI_QUERY_SETTING_ATTRIBUTE_COMPARE_TO);

        fileWriter.write("\ntypedef struct {\n" +
                         "  rci_query_setting_attribute_source_t source;\n" +
                         "  rci_query_setting_attribute_compare_to_t compare_to;\n");
        if (ConfigGenerator.rciLegacyEnabled()){
            fileWriter.write("  char const * target;\n");
        }
        fileWriter.write("} connector_remote_attribute_t;\n");

        fileWriter.write(RCI_QUERY_COMMAND_ATTRIBUTE_ID_T);

        if (configData.getMaxDepth() > 0) {
        	String name_field = ConfigGenerator.useNamesOption(UseNames.COLLECTIONS)
    			? "        char const * CONST name;\n"
				: "";

            fileWriter.write(
           		"\n" +
           		"typedef struct {\n" +
        	    "    unsigned int depth;\n" +
        	    "    struct {\n" + 
        	    "        unsigned int id;\n" + 
        	    "          unsigned int index;\n" +
        	    name_field + 
        	    "    } level[RCI_LIST_MAX_DEPTH];\n" + 
        	    "} connector_remote_list_t;\n"
        	    );
        }

        fileWriter.write(CONNECTOR_REMOTE_CONFIG_T);
        fileWriter.write(CONNECTOR_REMOTE_CONFIG_CANCEL_T);
        fileWriter.write(CONNECTOR_REMOTE_GROUP_TABLE_T);

    }
    
    private void generateUseNamesHeader(BufferedWriter writer, String type, int amount) throws IOException {
        writer.write(String.format(
            "#if !(defined RCI_%s_NAME_MAX_SIZE)\n" +
            "#define RCI_%s_NAME_MAX_SIZE %d\n" +
            "#else\n" +
            "#if RCI_%s_NAME_MAX_SIZE < %d\n" +
            "#undef RCI_%s_NAME_MAX_SIZE\n" +
            "#define RCI_%s_NAME_MAX_SIZE %d\n" +
            "#endif\n" +
            "#endif\n", type, type, amount, type, amount, type, type, amount));
    }
    
    public void generateUseNamesFile(ConfigData configData) throws IOException {
        if (ConfigGenerator.useNamesOption(UseNames.NONE)) {
        	return;
        }
        
        String USENAMES_NAME = customPrefix + "rci_usenames_defines.h";
        String usenamesFile = USENAMES_NAME;
        BufferedWriter usenamesHeaderWriter = null;

        try {
            usenamesHeaderWriter = new BufferedWriter(new FileWriter(filePath + usenamesFile));
            writeHeaderComment(usenamesHeaderWriter);

            /* write include header in the header file */
            String usenamesHeaderDefineName = usenamesFile.replace('.', '_').toLowerCase();
            usenamesHeaderWriter.write(String.format("#ifndef %s\n#define %s\n\n", usenamesHeaderDefineName, usenamesHeaderDefineName));

            if (ConfigGenerator.useNamesOption(UseNames.ELEMENTS)) {
            	generateUseNamesHeader(usenamesHeaderWriter, "ELEMENTS", configData.getMaxNameLength(UseNames.ELEMENTS) + 1);
            }
            
            if (ConfigGenerator.useNamesOption(UseNames.COLLECTIONS)) {
            	generateUseNamesHeader(usenamesHeaderWriter, "COLLECTIONS", configData.getMaxNameLength(UseNames.COLLECTIONS) + 1);
            }

            usenamesHeaderWriter.write(String.format("\n#endif\n"));

            String includeFilename = ConfigGenerator.useCcapi()
        		? "custom_connector_config.h"
        		: "connector_config.h";

            ConfigGenerator.log(String.format("Files created:\n\t%s%s",  filePath, usenamesFile));
            ConfigGenerator.log(String.format("NOTE: include \"%s%s\" in your %s so connector_api_remote.h can find the defines\n", filePath, usenamesFile, includeFilename));

        } catch (IOException e) {
            throw new IOException(e.getMessage());
        } finally {
            if (usenamesHeaderWriter != null){
                usenamesHeaderWriter.close();
            }
        }
    }
    
    /* returns field name (or "" if not used) */
    private String writeGroupElementDefine(ConfigData configData, UseNames type) throws IOException {
    	String field = "";
    	
        if (ConfigGenerator.useNamesOption(type)) {
        	String value = type.name();
        	boolean plural = value.endsWith("S");
        	String name = plural ? value.substring(0, value.length() - 1) : value;
        	
            switch (ConfigGenerator.fileTypeOption())
            {
                case SOURCE:
                case GLOBAL_HEADER:
                    break;
                    
                case NONE:
                    fileWriter.write(String.format("\n" + "#define RCI_%s_NAME_MAX_SIZE %d",
                    	name, configData.getMaxNameLength(type) + 1));
            }
            field = String.format("    char name[RCI_%s_NAME_MAX_SIZE];\n", name);
        }
    
    	return field;
    }
    
    private void writeGroupElementStructs(ConfigData configData) throws IOException {
        String element_name_struct_field = writeGroupElementDefine(configData, UseNames.ELEMENTS);
        String collection_name_struct_field = writeGroupElementDefine(configData, UseNames.COLLECTIONS);

        fileWriter.write("\n");
        
        if (ConfigGenerator.rciParserOption()) {
            fileWriter.write(
        		"\ntypedef struct {\n" +
                "    char const * const name;\n" +
                "} connector_element_enum_t;\n"
                );
        }

        String element_enum_data = "";
        if (ConfigGenerator.rciParserOption()) {
            element_enum_data = 
        		"    struct {\n"+
                "        size_t count;\n"+
                "        connector_element_enum_t CONST * CONST data;\n"+
                "    } enums;\n";
        }

        fileWriter.write(
           		"\n" + 
    	        "typedef struct {\n" + 
        		element_name_struct_field +
    	        "    connector_element_access_t access;\n" +
           		element_enum_data +		
    	        "} connector_element_t;\n"
    	        );
            
        fileWriter.write(
           		"\n" + 
    	        "typedef struct {\n" + 
        		collection_name_struct_field +
    	        "    size_t instances;\n" + 
    	        "    struct {\n" + 
    	        "        size_t count;\n" + 
    	        "        struct connector_item CONST * CONST data;\n" + 
    	        "    } item;\n" + 
    	        "} connector_collection_t;\n"
    	        );
            
        fileWriter.write(
    		"\n" + 
    		"typedef struct connector_item {\n" +
            "    connector_element_value_type_t type;\n" +
    		"    union {\n" +
            "        connector_collection_t CONST * CONST collection;\n" +
            "        connector_element_t CONST * CONST element;\n" +
            "    } data;\n" +
    		"} connector_item_t;\n"
            );

        fileWriter.write(
        		"\ntypedef struct {\n" +
                "    connector_collection_t collection;\n" +
                "    struct {\n" +
                "        size_t count;\n" +
                "        char CONST * CONST * description;\n" +
                "    } errors;\n" + 
        		"} connector_group_t;\n" +
                "\n"
        		);
    }

    protected void writeRemoteAllStrings(ConfigData configData, BufferedWriter bufferWriter) throws Exception {
        if (!ConfigGenerator.excludeErrorDescription()) {
            bufferWriter.write(String.format("\nstatic char CONST %s[] = {\n",
                    CONNECTOR_REMOTE_ALL_STRING));
        }

        writeRciErrorsRemoteAllStrings(configData, bufferWriter);

        for (GroupType type : GroupType.values()) {
            LinkedList<Group> theConfig = null;

            configType = type.toString().toLowerCase();

            theConfig = configData.getConfigGroup(configType);

            if (!theConfig.isEmpty()) {
                writeGroupRemoteAllStrings(theConfig, bufferWriter);
            }
        }
        writeErrorsRemoteAllStrings(configData, bufferWriter);

        if (!ConfigGenerator.excludeErrorDescription()) {
            bufferWriter.write("\n};\n\n"); // end of CONNECTOR_REMOTE_ALL_STRING
        }
    }

    protected void writeDefineGroupErrors(ConfigData configData,BufferedWriter bufferWriter) throws Exception {
        for (GroupType type : GroupType.values()) {
            String defineName = null;
            LinkedList<Group> groups = null;

            configType = type.toString().toLowerCase();

            groups = configData.getConfigGroup(configType);

            for (Group group : groups) {
                defineName = getDefineString(group.getName());
                /* define name string index
                 * #define [group name]
                 */

                if ((!ConfigGenerator.excludeErrorDescription()) && (!group.getErrors().isEmpty())) {
                    LinkedHashMap<String, String> errorMap = group.getErrors();
                    for (String key : errorMap.keySet()) {
                        defineName = getDefineString(group.getName() + "_" + ERROR + "_" + key);
                        /* define name string index for each error in the group
                         * #define [group name + ERROR + error name]
                         */
                        bufferWriter.write(getDefineStringIndex(defineName, errorMap.get(key), ConfigGenerator.FileType.SOURCE));
                    }
                }
            }
        }

    }

    private void writeGroupRemoteAllStrings(LinkedList<Group> groups, BufferedWriter bufferWriter) throws Exception {
        for (Group group : groups) {
            if ((!ConfigGenerator.excludeErrorDescription()) && (!group.getErrors().isEmpty())) {
                LinkedHashMap<String, String> errorMap = group.getErrors();
                for (String key : errorMap.keySet()) {
                    bufferWriter.write(getCharString(errorMap.get(key)));
                }
            }
        }
    }

    private void writeDefineErrors(String prefixName, LinkedHashMap<String, String> errorMap, ConfigGenerator.FileType fileType,BufferedWriter bufferWriter) throws IOException {
        for (String key : errorMap.keySet()) {
            String defineName = prefixName.toUpperCase() + "_" + key.toUpperCase();
            /* define name string index for each error */
            bufferWriter.write(getDefineStringIndex(defineName, errorMap.get(key), fileType));
        }
    }

    protected void writeDefineGlobalErrors(ConfigData configData, BufferedWriter bufferWriter) throws IOException {
        if (!ConfigGenerator.excludeErrorDescription()) {
            writeDefineErrors(GLOBAL_ERROR, configData.getUserGlobalErrors(), ConfigGenerator.FileType.SOURCE,bufferWriter);
        }
    }

    protected void writeDefineRciErrors(ConfigData configData,BufferedWriter bufferWriter) throws IOException {
        if (!ConfigGenerator.excludeErrorDescription()) {
            writeDefineErrors(GLOBAL_RCI_ERROR, configData.getRciGlobalErrors(), ConfigGenerator.FileType.SOURCE,bufferWriter);
        }
    }

    private void writeLinkedHashMapStrings(LinkedHashMap<String, String> stringMap,BufferedWriter bufferWriter) throws IOException {
        for (String key : stringMap.keySet()) {
            bufferWriter.write(getCharString(stringMap.get(key)));
        }
    }

    private void writeErrorsRemoteAllStrings(ConfigData configData, BufferedWriter bufferWriter) throws IOException {
        if (!ConfigGenerator.excludeErrorDescription()) {
            writeLinkedHashMapStrings(configData.getUserGlobalErrors(),bufferWriter);
        }
    }

    private void writeRciErrorsRemoteAllStrings(ConfigData configData, BufferedWriter bufferWriter) throws IOException {
        if (!ConfigGenerator.excludeErrorDescription()) {
            writeLinkedHashMapStrings(configData.getRciGlobalErrors(),bufferWriter);
        }
    }

    private void writeEnumArray(BufferedWriter bufferWriter, Element element, String prefix) throws Exception {
        boolean first = true;
        String define_name = getDefineString(prefix + "__" + element.getName() + "_enum");
        
        bufferWriter.write(String.format("static connector_element_enum_t CONST %s%s[] = {\n",
                customPrefix, define_name.toLowerCase()));
        
        for (ValueStruct value : element.getValues()) {
        	if (!first) {
                bufferWriter.write(",\n");
        	} else {
        		first = false;
        	}
        	
            bufferWriter.write(String.format("    {\"%s\"}",value.getName()));
        }
        bufferWriter.write("\n};\n\n");
    }
    
    private void writeEnumArrays(BufferedWriter bufferWriter, ItemList items, String prefix) throws Exception{
        for (Item item: items.getItems()) {
            assert (item instanceof Element) || (item instanceof ItemList);

            if (item instanceof Element) {
                Element element = (Element) item;
                
                if (Element.ElementType.toElementType(element.getType()) == Element.ElementType.ENUM) {
                	writeEnumArray(bufferWriter, element, prefix);
                }
            } else {
            	ItemList subitems = (ItemList) item;
            	
            	writeEnumArrays(bufferWriter, subitems, prefix + "__" + sanitizeName(subitems.getName()));
            }
        }
    }

    private void writeCollectionArray(BufferedWriter bufferWriter, ItemList items, String prefix) throws Exception {
    	// Traverse down the tree to define all the lists first as they need to be defined before the collections that include them are. 
        for (Item item: items.getItems()) {
            assert (item instanceof Element) || (item instanceof ItemList);

            String itemVariable = getDefineString(customPrefix + prefix + "__" + sanitizeName(item.getName())).toLowerCase();
            if (item instanceof Element) {
                Element element = (Element) item;
                String optional = ConfigGenerator.useNamesOption(UseNames.ELEMENTS)
                	? String.format("    \"%s\",\n", element.getName())
                	: "";
                
                bufferWriter.write("static connector_element_t CONST " + itemVariable + "_element = {\n");
                bufferWriter.write(optional + "    " + getElementDefine("access", getAccess(element.getAccess())) + ",\n");
                
                if (ConfigGenerator.rciParserOption()) {
                    String enum_struct;

                    if (Element.ElementType.toElementType(element.getType()) == Element.ElementType.ENUM) {
                        String define_name = getDefineString(prefix + "__" + element.getName() + "_enum");
                        String variableName = customPrefix + define_name.toLowerCase();
                        
                        enum_struct = "{ ARRAY_SIZE(" + variableName + "), " + variableName + "}, ";
                    }
                    else {
                    	enum_struct = "{ 0, NULL}, ";
                    }
                    
                    bufferWriter.write(enum_struct);
                }
                bufferWriter.write(	"};\n\n");
            } else {
            	ItemList subitems = (ItemList) item;
            	String subitemsPrefix = prefix + "__" + sanitizeName(item.getName()).toLowerCase();
            	
            	writeCollectionArray(bufferWriter, subitems, subitemsPrefix);
            	
            	String subitemsVariable = itemVariable + "_items";
                String optional = ConfigGenerator.useNamesOption(UseNames.COLLECTIONS)
                    	? String.format("    \"%s\",\n", subitems.getName())
                    	: "";
            	
                bufferWriter.write("static connector_collection_t CONST " + itemVariable + "_collection = {\n");
                bufferWriter.write(optional + "    " + subitems.getInstances() + ", " + COMMENTED("instances") + "\n");
                bufferWriter.write("    {\n");
                bufferWriter.write("        " + subitems.getItems().size() + ", " + COMMENTED("items") + "\n");
                bufferWriter.write("        " + subitemsVariable + "\n");
                bufferWriter.write("    }\n");
                bufferWriter.write("};\n\n");
            }
        }
        
        // Then write the array for this level
    	String itemsVariable = getDefineString(customPrefix + prefix).toLowerCase();
    	boolean first = true;
        bufferWriter.write("static connector_item_t CONST " + itemsVariable + "_items[] = {\n");
        for (Item item: items.getItems()) {
            assert (item instanceof Element) || (item instanceof ItemList);

            String type; 
            String suffix;
            if (item instanceof Element) {
            	Element element = (Element) item;
            	
            	type = element.getType();
            	suffix = "_element";
            } else {
            	type = "list";
            	suffix = "_collection";
            }
            if (!first) {
            	bufferWriter.write(",\n");
            } else {
            	first = false;
            }
            
            String itemVariable = getDefineString(customPrefix + prefix + "__" + sanitizeName(item.getName())).toLowerCase() + suffix;
            bufferWriter.write("{ " + getElementDefine("type", type) + ", { &" + itemVariable + " } }");
        }
        bufferWriter.write("\n};\n\n");
    }
    
    private void writeCollectionArrays(BufferedWriter bufferWriter, ItemList items, String prefix) throws Exception {
        if (ConfigGenerator.rciParserOption()) {
            writeEnumArrays(bufferWriter, items, prefix);
        }

        writeCollectionArray(bufferWriter, items, prefix);
    }

    private int writeErrorStructures(int errorCount, String prefix, LinkedHashMap<String, String> errorMap, BufferedWriter bufferWriter) throws IOException {
    	String defineName = prefix.toUpperCase();
    	
        for (String key : errorMap.keySet()) {
            bufferWriter.write(getRemoteString(defineName + "_" + key));
            errorCount--;
            if (errorCount > 0) {
                bufferWriter.write(",");
            }
            bufferWriter.write(COMMENTED(key) + "\n");
        }

        return errorCount;
    }

    protected void writeGlobalErrorStructures(ConfigData configData, BufferedWriter bufferWriter) throws IOException {
        if (!ConfigGenerator.excludeErrorDescription()) {
            int errorCount = configData.getRciGlobalErrors().size() + configData.getUserGlobalErrors().size();

            if (errorCount > 0) {
                bufferWriter.write(String.format("static char const * const %ss[] = {\n", GLOBAL_RCI_ERROR));

                /* top-level global errors */
                errorCount = writeErrorStructures(errorCount, GLOBAL_RCI_ERROR, configData.getRciGlobalErrors(),  bufferWriter);
                /* group global errors */
                errorCount = writeErrorStructures(errorCount, GLOBAL_ERROR, configData.getUserGlobalErrors(), bufferWriter);

                bufferWriter.write("};\n\n");
            }
        }
        else {
            bufferWriter.write(String.format("%s%ss NULL\n", DEFINE, GLOBAL_RCI_ERROR));
        }
    }

    private void writeLocalErrorStructures(String error_name, LinkedHashMap<String, String> localErrors, BufferedWriter bufferWriter) throws IOException {
        if (!ConfigGenerator.excludeErrorDescription()) {
        	int errorCount = localErrors.size();
        	 
	        if (errorCount > 0) {
	            String define_name = getDefineString(error_name + "_" + ERROR);
	            
	            bufferWriter.write(CHAR_CONST_STRING + customPrefix + define_name.toLowerCase() + "s[] = {\n");
	            writeErrorStructures(errorCount, define_name, localErrors, bufferWriter);
	            bufferWriter.write("};\n\n");
	        }
	    }
    }
    
    private void writeGroupStructures(LinkedList<Group> groups, BufferedWriter bufferWriter) throws Exception {
        for (Group group: groups) {
            writeCollectionArrays(bufferWriter, group, group.getName());
            
            if (!ConfigGenerator.excludeErrorDescription()) {
            	writeLocalErrorStructures(group.getName(), group.getErrors(), bufferWriter);
            }
        }
    }

    protected void writeAllStructures(ConfigData configData, BufferedWriter bufferWriter) throws Exception {
        for (GroupType type : GroupType.values()) {
            LinkedList<Group> groups = null;

            configType = type.toString().toLowerCase();

            groups = configData.getConfigGroup(configType);

            if (!groups.isEmpty()) {
                writeGroupStructures(groups, bufferWriter);

                bufferWriter.write(String.format("static connector_group_t CONST %sconnector_%s_groups[] = {", customPrefix, configType));

                for (int group_index = 0; group_index < groups.size(); group_index++) {
                    Group group = groups.get(group_index);
                    String items_name = customPrefix + getDefineString(group.getName() + "_items").toLowerCase();
                    String optional = ConfigGenerator.useNamesOption(UseNames.COLLECTIONS)
                        	? String.format("        \"%s\",\n", group.getName())
                        	: "";
                    String group_string = 
                    	"\n" +
                    	"{\n" +
            			"    {\n" +
                    	optional +
                		"        " + group.getInstances() + ", " + COMMENTED("instances") + "\n" +
                        "        { ARRAY_SIZE(" + items_name + "), " + items_name + " }, \n" +
                    	"    },\n";
                    
                    if ((!ConfigGenerator.excludeErrorDescription()) && (!group.getErrors().isEmpty())) {
                        String errors_name = customPrefix + getDefineString(group.getName() + "_errors").toLowerCase();

                        group_string += "    { ARRAY_SIZE(" + errors_name + "), " + errors_name + " }, \n";
                        
                    } else {
                        group_string += "    { 0, NULL }\n";
                    }

                    if (group_index < (groups.size() - 1)) {
                        group_string += "},\n";
                    } else {
                        group_string += "}\n";
                    }
                    bufferWriter.write(group_string);
                }
                bufferWriter.write("\n};\n\n");
            }
        }

        String rciGroupString = String.format("static connector_remote_group_table_t CONST %s[] = {\n",
                                                CONNECTOR_REMOTE_GROUP_TABLE);

        for (GroupType type : GroupType.values()) {
            LinkedList<Group> groups = null;

            configType = type.toString().toLowerCase();

            groups = configData.getConfigGroup(configType);

            if (type.getIndex() != 0) {
                rciGroupString += ",\n";
            }

            rciGroupString += " {";
            if (!groups.isEmpty()) {
                rciGroupString += String.format(" %sconnector_%s_groups,\n   ARRAY_SIZE(%sconnector_%s_groups)\n }",
                                                   customPrefix,configType,customPrefix, configType);

            } else {
                rciGroupString += "NULL,\n 0\n }";
            }

        }
        rciGroupString += "\n};\n\n";

        bufferWriter.write(rciGroupString);
    }

    protected void writeErrorHeader(int errorIndex, String enumDefine, LinkedHashMap<String, String> errorMap, BufferedWriter bufferWriter) throws IOException {

        for (String key : errorMap.keySet()) {
            String error_string = " " + enumDefine + "_" + key;

            if (errorIndex == 1) {
                error_string += " = " + " " + enumDefine + "_" + OFFSET_STRING;
            }
            errorIndex++;

            error_string += ",\n";

            bufferWriter.write(error_string);
        }

    }

    protected void writeErrorHeader(String type, int errorIndex, String enumDefine, LinkedHashMap<String, String> errorMap, BufferedWriter bufferWriter) throws IOException {

        for (String key : errorMap.keySet()) {
            String error_string = enumDefine + "_" + key;

            if (type.equalsIgnoreCase("rci")){
                if (errorIndex == 1) error_string += " = 1, " + COMMENTED("Protocol defined") + "\n";
                else error_string += ",\n";
                errorIndex++;
            } else if (type.equalsIgnoreCase("global")){
                if (errorIndex == 1) error_string += ", " +  COMMENTED("User defined (global errors)") + "\n";
                else error_string += ",\n";
                errorIndex++;
            } else {
                error_string += ",\n";
            }
            bufferWriter.write(error_string);
        }

    }

    protected void writeCcapiErrorHeader(String type, int errorIndex, String enumDefine, LinkedHashMap<String, String> errorMap, BufferedWriter bufferWriter) throws IOException {

        if (errorIndex == 0)
        {
                String defaultErrorsStrings = "";

                defaultErrorsStrings += enumDefine + "_" + "NONE,\n";
                bufferWriter.write(defaultErrorsStrings.toUpperCase());
                return;
        }

        for (String key : errorMap.keySet()) {
            String error_string = enumDefine + "_" + key;

            if (type.equalsIgnoreCase("rci")){
                if (errorIndex == 1)
                        error_string += " = 1, " + COMMENTED("Protocol defined") + "\n";
                else
                        error_string += ",\n";
                errorIndex++;
            } else if (type.equalsIgnoreCase("global")){
                if (errorIndex == 1) error_string += ", " +  COMMENTED("User defined (global errors)") + "\n";
                else error_string += ",\n";
                errorIndex++;
            } else {
                error_string += ",\n";
            }
            bufferWriter.write(error_string.toUpperCase());
        }

    }

    protected void writeRciErrorEnumHeader(ConfigData configData, BufferedWriter bufferWriter ) throws IOException {

    /* write typedef enum for rci errors */
        bufferWriter.write("\n" + TYPEDEF_ENUM + " " + GLOBAL_RCI_ERROR + "_" + OFFSET_STRING + " = 1,\n");
        writeErrorHeader(configData.getRciGlobalErrorsIndex(),GLOBAL_RCI_ERROR, configData.getRciGlobalErrors(), bufferWriter);
        bufferWriter.write(" " + GLOBAL_RCI_ERROR + "_" + COUNT_STRING + "\n} " + customPrefix  + GLOBAL_RCI_ERROR + ID_T_STRING);
    }

    protected void writeGlobalErrorEnumHeader(ConfigData configData, BufferedWriter bufferWriter) throws IOException {

        String index_string = "";
        /* write typedef enum for user global error */
        String enumName = GLOBAL_ERROR + "_" + OFFSET_STRING;

        bufferWriter.write("\n" + TYPEDEF_ENUM + " " + enumName + " = " + GLOBAL_RCI_ERROR + "_" + COUNT_STRING + ",\n");

        writeErrorHeader(1,GLOBAL_ERROR, configData.getUserGlobalErrors(), bufferWriter);

        String endString = String.format(" %s_%s%s", GLOBAL_ERROR, COUNT_STRING, index_string);

        if (configData.getUserGlobalErrors().isEmpty()) {
            endString += " = " + enumName;
        }
        endString += "\n} " + customPrefix + GLOBAL_ERROR + ID_T_STRING;

        bufferWriter.write(endString);

    }

    private void writeEnumHeader(BufferedWriter bufferWriter, Element element, String prefix) throws Exception {
        boolean explicit = false;
        int index = 0;
        
        bufferWriter.write(TYPEDEF_ENUM);
        for (ValueStruct value : element.getValues()) {
            if (value.getName().equals(""))
            	explicit = true;
            else {
            	String line = getEnumString(prefix + "_" + sanitizeName(value.getName()));
            	
            	if (explicit) {
            		line += " = " + index;
            		explicit = false;
            	}
                bufferWriter.write(line + ",\n");
            }
            index++;
        }
        bufferWriter.write(endEnumString(prefix));
    }
    
    private void writeListEnumHeader(BufferedWriter bufferWriter, ItemList list, String prefix) throws Exception {
        String element_enum_string = TYPEDEF_ENUM;

        for (Item item : list.getItems()) {
        	String item_prefix = prefix + "_" + sanitizeName(item.getName());
        	
            assert (item instanceof Element) || (item instanceof ItemList);

            if (item instanceof Element) {
                Element element = (Element) item;

                element_enum_string += getEnumString(item_prefix) + ",\n";

                if (Element.ElementType.toElementType(element.getType()) == Element.ElementType.ENUM) {
                	writeEnumHeader(bufferWriter, element, item_prefix);
                }
            } else {
                ItemList sublist = (ItemList) item;
                
                writeListEnumHeader(bufferWriter, sublist, item_prefix + "_");
            }
        }
        element_enum_string += endEnumString(prefix);

        bufferWriter.write(element_enum_string);
    }
    
    private void writeAllEnumHeaders(ConfigData configData, LinkedList<Group> groups, BufferedWriter bufferWriter) throws Exception {

        for (Group group : groups) {
        	writeListEnumHeader(bufferWriter, group, group.getName());
        	
            if (!group.getErrors().isEmpty()) {
                bufferWriter.write(TYPEDEF_ENUM);

                writeErrorHeader("rci",1, getEnumString(group.getName() + "_" + ERROR), configData.getRciGlobalErrors(), bufferWriter);
                writeErrorHeader("global",1, getEnumString(group.getName() + "_" + ERROR), configData.getUserGlobalErrors(), bufferWriter);

                LinkedHashMap<String, String> errorMap = group.getErrors();
                int index = 0;

                for (String key : errorMap.keySet()) {
                    String enumString = getEnumString(group.getName() + "_" + ERROR + "_" + key);

                    if (index++ == 0) {
                        /*Set start index to the global count */
                        enumString += ", " + COMMENTED("User defined (group errors)") + "\n";
                    }
                    else{
                    enumString += ",\n";
                    }

                    bufferWriter.write(enumString);
                }
                bufferWriter.write(endEnumString(group.getName() + "_" + ERROR));
            }
        }
    }

    protected void writeGroupTypeAndErrorEnum(ConfigData configData,BufferedWriter bufferWriter) throws Exception {

        for (GroupType type : GroupType.values()) {
	        LinkedList<Group> groups = null;

	        configType = type.toString().toLowerCase();
	        groups = configData.getConfigGroup(configType);

	        if (!groups.isEmpty()) {
	            /* build group enum string for group enum */
	            String group_enum_string = TYPEDEF_ENUM;

	            /* Write all enum in H file */
	            writeAllEnumHeaders(configData,groups, bufferWriter);

	            for (Group group : groups) {
	                /* add each group enum */
                    group_enum_string += getEnumString(group.getName()) + ",\n";
	            }

	            /* write group enum buffer to fileWriter */
	            group_enum_string += endEnumString(null);
	            bufferWriter.write(group_enum_string);
	        }
	    }
    }

    String COMMENTED(String comment) {
        return "/* " + comment + " */";
    }

    private String getEnumString(String enum_name) {
        String str = " " + customPrefix + CONNECTOR_PREFIX + "_" + configType;

        if (enum_name != null) {
            str += "_" + enum_name;
        }
        return str;
    }

    private String getCcapiEnumString(String enum_name) {
        String str = " " + customPrefix + CCAPI_PREFIX;
        if (configType != null)
                str += "_" + configType;

        if (enum_name != null) {
            str += "_" + enum_name;
        }
        return str.toUpperCase();
    }

    private String endEnumString(String group_name) {
        /*Add _COUNT */
        String str = " " + customPrefix + CONNECTOR_PREFIX + "_" + configType;
        if(group_name!=null)
           str += "_" + group_name;
        str += "_" + COUNT_STRING +"\n";

        str += "} " + customPrefix + CONNECTOR_PREFIX + "_" + configType;
        if(group_name!=null)
            str += "_" + group_name;
        str +=ID_T_STRING;

        return str;
    }

    private String endCcapiEnumString(String group_name) {
        /*Add _COUNT */

        String str = " " + customPrefix + CCAPI_PREFIX;
        if (configType != null)
                str += "_" + configType;

        if(group_name!=null)
           str += "_" + group_name;
        str += "_" + COUNT_STRING +"\n";

        str = str.toUpperCase();

        str += "} " + customPrefix + CCAPI_PREFIX;
        if (configType != null)
                str += "_" + configType;
        if(group_name!=null)
            str += "_" + group_name;
        str +=ID_T_STRING;

        return str;
    }

    String getDefineString(String define_name) {
        return (configType.toUpperCase() + "_" + define_name.toUpperCase());
    }

    private String getDefineStringIndex(String define_name, String string, ConfigGenerator.FileType fileType) {
        String str = DEFINE + define_name + " " + "(" + CONNECTOR_REMOTE_ALL_STRING + "+" + prevRemoteStringLength + ")\n";
        /* NOTE: this prevRemoteStringLength is important.
         * Must keep track this variable for #define string.
         */
        if (string != null) {
            prevRemoteStringLength += string.length();
        }
        prevRemoteStringLength++; /* add extra byte for the size */

        if (fileType == ConfigGenerator.fileTypeOption() ||
            ConfigGenerator.fileTypeOption() == ConfigGenerator.FileType.NONE) {
            return str;
        } else {
            return "";
        }
    }

    private String getCharString(String string) {

        String quote_char = (isFirstRemoteString) ? "": ",\n";

        isFirstRemoteString = false;

        if (string != null) {

            char[] characters = string.toCharArray();

            quote_char += " " + string.length() + ",";

            int length = characters.length;

            for (int i=0; i < length; i++)
            {
                quote_char += "\'" + characters[i] + "\'";
                if (i < length-1) {
                    //last character
                    quote_char += ",";
                }
            }
        }
        else {
            quote_char += " 0";
        }

        return quote_char;
    }

    private String getRemoteString(String define_name) {
        return (" " + define_name.toUpperCase());

    }

    String getElementDefine(String type_name, String element_name) {
        return (String.format("%s_element_%s_%s", CONNECTOR_PREFIX, type_name, element_name));
    }

    protected String getAccess(String access) {
        if (access == null) {
            return "read_write";
        }
        return access;
    }

}
