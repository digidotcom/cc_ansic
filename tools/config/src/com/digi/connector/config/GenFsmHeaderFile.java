package com.digi.connector.config;

import java.io.BufferedWriter;
import java.io.IOException;
import java.util.EnumSet;
import java.util.LinkedHashMap;
import java.util.LinkedList;

import com.digi.connector.config.ConfigGenerator.UseNames;

public class GenFsmHeaderFile extends GenHeaderFile {

    protected final static String CONNECTOR_REMOTE_CONFIG_DATA = "typedef struct connector_remote_config_data {\n" +
    "    struct connector_remote_group_table const * group_table;\n" +
    "    char const * const * error_table;\n" +
    "    unsigned int global_error_count;\n" +
    "    uint32_t firmware_target_zero_version;\n" +
    "    uint32_t vendor_id;\n" +
    "    char const * device_type;\n" +
    "} connector_remote_config_data_t;\n";

    private final static String FILENAME = "connector_api_remote.h";
    static public String getBasename() { return FILENAME; }
    
    // TODO: Everything in this class *must* use types instead of ConfigData/configData
    private EnumSet<Element.Type> types;
    
	public GenFsmHeaderFile(String dir, EnumSet<Element.Type> validTypes) throws IOException {
		super(dir, FILENAME, GenFile.Type.USER);
		types = validTypes;
	}

	public GenFsmHeaderFile(String dir) throws Exception {
		this(dir, ConfigData.getInstance().getTypesSeen());
	}

    public void generateFile(ConfigData configData) throws Exception {
        try {
            writePreamble();
        
            String defineName = FILENAME.replace('.', '_').toUpperCase();
            fileWriter.write(String.format("#ifndef %s\n#define %s\n\n", defineName, defineName));

            writeDefinesAndStructures(configData);
            
            /* Write all group enum in H file */
            writeRciErrorEnumHeader(configData, fileWriter);
            writeGlobalErrorEnumHeader(configData, fileWriter);

            writeGroupTypeAndErrorEnum(configData,fileWriter);

            fileWriter.write(CONNECTOR_REMOTE_CONFIG_DATA);

            fileWriter.write("\nextern connector_remote_config_data_t const * const rci_descriptor_data;\n\n");

            fileWriter.write("\n#if !defined _CONNECTOR_API_H\n");
            fileWriter.write("#error  \"Illegal inclusion of connector_api_remote.h. You should only include connector_api.h in user code.\"\n");
            fileWriter.write("#endif\n\n");
            fileWriter.write("#else\n");
            fileWriter.write("#error  \"Illegal inclusion of connector_api_remote.h. You should only include connector_api.h in user code.\"\n");
            fileWriter.write("#endif\n");
            
            ConfigGenerator.log(String.format("Files created:\n\t%s%s",  filePath, FILENAME));
        } catch (IOException e) {
            throw new IOException(e.getMessage());
        } finally {
            fileWriter.close();
        }
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

        for (Element.Type type : configData.getTypesSeen()) {
            headerString += DEFINE + RCI_PARSER_USES + type.toUpperName() + "\n";

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

        if (unsignedIntegerString != null) headerString += unsignedIntegerString;
        if (stringsString != null) headerString += stringsString;

        if (floatInclude != null)
            headerString += "\n\n" + floatInclude;

        fileWriter.write(headerString);
    }

    private void writeElementTypeEnums(EnumSet<Element.Type> typesSeen) throws IOException {
    	LinkedList<String> enum_lines = new LinkedList<>();
        int previous = -1;
        for (Element.Type type : typesSeen) {
            String enum_line = "    connector_element_type_" + type.toLowerName();

        	int current = type.toValue();
            if (current != (previous + 1)) {
            	enum_line += String.format(" = %d", current);
            }
            previous = current;
            
            enum_lines.add(enum_line);
        }

        assert(enum_lines.size() != 0);
        fileWriter.write(
        	"\n" +
        	"\n" + 
        	"typedef enum {\n" +
        	String.join(",\n", enum_lines) + "\n" +
        	"} connector_element_value_type_t;\n");
    }

    private String enumStructureString() {
        String string = TYPEDEF_STRUCT
                      + "    size_t count;\n";
        string += "} connector_element_value_enum_t;\n";

        return string;
    }

    private void writeElementValueStruct(EnumSet<Element.Type> typesSeen) throws IOException {

        String headerString = "";
        String structString = "";
        String elementValueStruct = "";
        String defineElementString = "";

        int optionCount = 0;

        Boolean isUnsignedIntegerDefined = false;
        Boolean isStringDefined = false;
        Boolean isEnumValueStructDefined = false;

        for (Element.Type type : typesSeen) {
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

    private void writeDefinesAndStructures(ConfigData configData) throws IOException {
    	EnumSet<Element.Type> typesSeen = configData.getTypesSeen();
    	boolean haveLists = typesSeen.contains(Element.Type.LIST);
        String optional_field;

        writeDefineOptionHeader(configData);

        if(ConfigGenerator.rciLegacyEnabled()){
            fileWriter.write(RCI_LEGACY_DEFINE);
        }
        if(ConfigGenerator.rciParserOption()){
            fileWriter.write(RCI_PARSER_DEFINE);
        }

        fileWriter.write(String.format("%sRCI_COMMANDS_ATTRIBUTE_MAX_LEN %d\n", DEFINE, ConfigData.AttributeMaxLen()));
        if (haveLists) {
            fileWriter.write(String.format("%sRCI_LIST_MAX_DEPTH %d\n", DEFINE, configData.getMaxDepth()));
        }


    	if (typesSeen.contains(Element.Type.ON_OFF)) {
            fileWriter.write(
        		"\n" +
        		"typedef enum {\n" +
                "    connector_off,\n" +
                "    connector_on\n" +
                "} connector_on_off_t;\n");
    	}
    	
        writeElementTypeEnums(typesSeen);
        writeElementValueStruct(typesSeen);

        String list_start = "";
        String list_end = "";
        	
        if (haveLists) {
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

        optional_field = ConfigGenerator.useNamesOption(UseNames.COLLECTIONS)
			? "        char const * CONST name;\n"
			: "";
        
        if (ConfigGenerator.useNamesOption(UseNames.COLLECTIONS)) {
            fileWriter.write("\n"+ DEFINE + "RCI_PARSER_USES_COLLECTION_NAMES\n");
        }
        fileWriter.write(
        	"\n" + 
        	"typedef struct {\n" +
            "    connector_remote_group_type_t type;\n" +
            "    unsigned int id;\n" +
            "    unsigned int index;\n" +
            optional_field +
            "} connector_remote_group_t;\n"
            );

        optional_field = ConfigGenerator.useNamesOption(UseNames.ELEMENTS)
			? "        char const * CONST name;\n"
			: "";
        if (ConfigGenerator.useNamesOption(UseNames.ELEMENTS)) {
            fileWriter.write("\n" + DEFINE + "RCI_PARSER_USES_ELEMENT_NAMES\n");
        }
        fileWriter.write(
    		"\n" + 
    		"typedef struct {\n" +
            "    unsigned int id;\n" +
            "    connector_element_value_type_t type;\n" +
            "    connector_element_value_t * value;\n" +
            optional_field + 
            "} connector_remote_element_t;\n"
            );
        
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

        if (haveLists) {
        	optional_field = ConfigGenerator.useNamesOption(UseNames.COLLECTIONS)
    			? "        char const * CONST name;\n"
				: "";
            fileWriter.write(
           		"\n" +
           		"typedef struct {\n" +
        	    "    unsigned int depth;\n" +
        	    "    struct {\n" + 
        	    "        unsigned int id;\n" + 
        	    "          unsigned int index;\n" +
        	    optional_field + 
        	    "    } level[RCI_LIST_MAX_DEPTH];\n" + 
        	    "} connector_remote_list_t;\n"
        	    );
        }

        optional_field = haveLists
    		? "    connector_remote_list_t CONST list;\n"
			: "";
        fileWriter.write(
       		"\n" +
       		"typedef struct {\n" +
    	    "    void * user_context;\n" +
    	    "    connector_remote_action_t CONST action;\n" +
    	    "    connector_remote_attribute_t CONST attribute;\n" +
    	    "    connector_remote_group_t CONST group;\n" +
    	    optional_field +
    	    "    connector_remote_element_t CONST element;\n" +
    	    "    unsigned int error_id;\n" +
    	    "\n" +
    	    "    struct {\n" +
    	    "        connector_bool_t compare_matches;\n" +
    	    "        char const * error_hint;\n" +
    	    "        connector_element_value_t * element_value;\n" +
    	    "    } response;\n" +
    	    "} connector_remote_config_t;\n"
    	    );

        fileWriter.write(CONNECTOR_REMOTE_CONFIG_CANCEL_T);
        fileWriter.write(CONNECTOR_REMOTE_GROUP_TABLE_T);

    }
    
    private String COMMENTED(String comment) {
        return "/* " + comment + " */";
    }

    private void writeErrorHeader(String type, int errorIndex, String enumDefine, LinkedHashMap<String, String> errorMap, BufferedWriter bufferWriter) throws IOException {

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

    private void writeErrorHeader(int errorIndex, String enumDefine, LinkedHashMap<String, String> errorMap, BufferedWriter bufferWriter) throws IOException {

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

    private void writeRciErrorEnumHeader(ConfigData configData, BufferedWriter bufferWriter ) throws IOException {

    /* write typedef enum for rci errors */
        bufferWriter.write("\n" + TYPEDEF_ENUM + " " + GLOBAL_RCI_ERROR + "_" + OFFSET_STRING + " = 1,\n");
        writeErrorHeader(configData.getRciGlobalErrorsIndex(),GLOBAL_RCI_ERROR, configData.getRciGlobalErrors(), bufferWriter);
        bufferWriter.write(" " + GLOBAL_RCI_ERROR + "_" + COUNT_STRING + "\n} " + customPrefix  + GLOBAL_RCI_ERROR + ID_T_STRING);
    }

    private void writeGlobalErrorEnumHeader(ConfigData configData, BufferedWriter bufferWriter) throws IOException {

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

    private String sanitizeName(String name) {
    	return name.replace('-', '_');
    }

    private void writeEnumHeader(BufferedWriter bufferWriter, Element element, String prefix) throws Exception {
        boolean explicit = false;
        int index = 0;
        
        bufferWriter.write(TYPEDEF_ENUM);
        for (Value value : element.getValues()) {
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

                if (element.getType() == Element.Type.ENUM) {
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
    
    private String getEnumString(String enum_name) {
        String str = " " + customPrefix + CONNECTOR_PREFIX + "_" + configType;

        if (enum_name != null) {
            str += "_" + enum_name;
        }
        return str;
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

    private void writeGroupTypeAndErrorEnum(ConfigData configData,BufferedWriter bufferWriter) throws Exception {

        for (Group.Type type : Group.Type.values()) {
	        LinkedList<Group> groups = configData.getConfigGroup(type);

            configType = type.toLowerName();

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
}