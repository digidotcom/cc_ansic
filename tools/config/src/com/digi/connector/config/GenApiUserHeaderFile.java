package com.digi.connector.config;

import java.io.BufferedWriter;
import java.io.IOException;
import java.util.LinkedHashMap;
import java.util.LinkedList;

public class GenApiUserHeaderFile extends GenHeaderFile {

    private static String FILENAME = ConfigGenerator.getCustomPrefix() + "ccapi_rci_functions.h";
    static public String getBasename() { return FILENAME; }
    
	public GenApiUserHeaderFile(String dir) throws IOException {
		super(dir, FILENAME, GenFile.Type.USER);
	}

    public void generateFile(ConfigData configData) throws Exception {
        try {
            writePreamble();

            /* write include header in the header file */
            String headerDefineName = FILENAME.replace('.', '_');
            fileWriter.write(String.format("#ifndef %s\n#define %s\n\n", headerDefineName, headerDefineName));
            fileWriter.write(String.format("%s \"%s\"\n\n", INCLUDE, "connector_api.h"));
            fileWriter.write(String.format("\n%s UNUSED_PARAMETER(a) (void)(a)\n",DEFINE));
            fileWriter.write("\nextern ccapi_rci_data_t const " + customPrefix + "ccapi_rci_data;\n");
            writePrototypes(configData, fileWriter);
            fileWriter.write(String.format("\n#endif\n"));

            /* write structures in source file */
            writeAllCcapiStructures(configData, fileWriter);

            ConfigGenerator.log(String.format("Files created:\n\t%s%s",  filePath, generatedFile));
            if (generatedFile.length() > 0) {
                ConfigGenerator.log(String.format("\t%s%s", filePath, FILENAME));
            }
        } catch (IOException e) {
            throw new IOException(e.getMessage());
        } finally {
            fileWriter.close();
        }
    }

    private String getDefineString(String define_name) {
        return (configType.toUpperCase() + "_" + define_name.toUpperCase());
    }

    private String COMMENTED(String comment) {
        return "/* " + comment + " */";
    }

    private String getAccess(String access) {
        if (access == null) {
            return "read_write";
        }
        return access;
    }

    private void writeItemArrays(String prefix, ItemList list, BufferedWriter bufferWriter) throws Exception {
    	String current = String.format("static ccapi_rci_element_t const %s%s_elements[] = {",
        		customPrefix, getDefineString(prefix).toLowerCase());

        for (Item item: list.getItems()) {
            assert (item instanceof Element) || (item instanceof ItemList);

            if (item instanceof Element) {
                Element element = (Element) item;
                
                current += String.format("\n { %s", COMMENTED(element.getName()));

                if (getAccess(element.getAccess()).equalsIgnoreCase("read_only")) {
                	current += "   NULL,\n";
                }
                else {
                	current += String.format("   %s%srci_%s_%s_set,\n", RCI_FUNCTION_T, customPrefix, getDefineString(prefix).toLowerCase(),element.getName());
                }

                current += String.format("   %s%srci_%s_%s_get\n", RCI_FUNCTION_T, customPrefix, getDefineString(prefix).toLowerCase(),element.getName());
                current += " },";
            } else {
                ItemList items = (ItemList) item;

            	writeItemArrays(prefix + "_" + items.getName(), items, bufferWriter);

            	// TODO: Not sure what to do about lists in the CCAPI version quite yet. Making them NULL for now to trigger an error. -ASK
                current += 
                	String.format("\n { %s", COMMENTED(items.getName())) +
	            	"   NULL,\n" +
	            	"   NULL\n" +
	                " },";
            }
        }
        
        if (current.endsWith(",")) {
        	current = current.substring(0, current.length() - 1);
        }

        bufferWriter.write(current + "\n};\n\n");
    }
    
    private void writeGroupStructures(LinkedList<Group> groups, BufferedWriter bufferWriter) throws Exception {

        for (int group_index = 0; group_index < groups.size(); group_index++) {
            Group group = groups.get(group_index);

            /* write element structure */
            writeItemArrays(group.getName(), group, bufferWriter);
        }

    }

    private void writeAllCcapiStructures(ConfigData configData, BufferedWriter bufferWriter) throws Exception {
        String define_name;

        for (Group.Type type : Group.Type.values()) {
            LinkedList<Group> groups = configData.getConfigGroup(type);

            configType = type.toString().toLowerCase();

            if (!groups.isEmpty()) {
                writeGroupStructures(groups, bufferWriter);

                bufferWriter.write(String.format("static ccapi_rci_group_t const %sccapi_%s_groups[] =\n{", customPrefix, configType));

                for (int group_index = 0; group_index < groups.size(); group_index++) {
                    Group group = groups.get(group_index);

                    define_name = getDefineString(group.getName() + "_elements");
                    String group_string = "";

                    group_string += String.format("\n    { %s", COMMENTED(group.getName()));
                    group_string += String.format("        %s%s,", customPrefix, define_name.toLowerCase());
                    group_string += String.format("\n        ARRAY_SIZE(%s%s),", customPrefix, define_name.toLowerCase());
                    group_string += String.format("\n        {");
                    group_string += String.format("\n            %s%srci_%s_start," , RCI_FUNCTION_T,customPrefix,getDefineString(group.getName()).toLowerCase());
            		group_string += String.format("\n            %s%srci_%s_end" , RCI_FUNCTION_T,customPrefix,getDefineString(group.getName()).toLowerCase());
                    group_string += String.format("\n        }");
                    group_string += String.format("\n    }");
                    if (group_index < (groups.size() - 1)) {
                    	group_string += String.format(",");
                    }
                    bufferWriter.write(group_string);
                }
                bufferWriter.write("\n};\n\n");
            }
        }

        String ccfsm_internal_data = "extern connector_remote_config_data_t const " + customPrefix + "rci_internal_data;\n";
        String ccapi_rci_data = ccfsm_internal_data + "ccapi_rci_data_t const " + customPrefix + "ccapi_rci_data =\n{";

        Group.Type type;
        
        type = Group.Type.SETTING;
        {
        	LinkedList<Group> groups = configData.getConfigGroup(type);

            configType = type.toLowerName();

        	ccapi_rci_data += String.format("\n    {");
            if (groups.isEmpty()) {
                ccapi_rci_data += String.format("\n        NULL,");
            	ccapi_rci_data += String.format("\n        0");
            }
            else {
            	ccapi_rci_data += String.format("\n        %sccapi_%s_groups,", customPrefix, configType);
            	ccapi_rci_data += String.format("\n        ARRAY_SIZE(%sccapi_%s_groups)", customPrefix, configType);
            }
        	ccapi_rci_data += String.format("\n    },");
        }

        type = Group.Type.STATE;
        {
        	LinkedList<Group> groups = configData.getConfigGroup(type);

            configType = type.toLowerName();

        	ccapi_rci_data += String.format("\n    {");
            if (groups.isEmpty()) {
                ccapi_rci_data += String.format("\n        NULL,");
            	ccapi_rci_data += String.format("\n        0");
            }
            else {
            	ccapi_rci_data += String.format("\n        %sccapi_%s_groups,", customPrefix, configType);
            	ccapi_rci_data += String.format("\n        ARRAY_SIZE(%sccapi_%s_groups)", customPrefix, configType);
            }
        	ccapi_rci_data += String.format("\n    },");
        }

        ccapi_rci_data += String.format("\n    {");
        ccapi_rci_data += String.format("\n        %s%srci_session_start_cb,", RCI_FUNCTION_T, customPrefix);
        ccapi_rci_data += String.format("\n        %s%srci_session_end_cb,", RCI_FUNCTION_T, customPrefix);
        ccapi_rci_data += String.format("\n        %s%srci_action_start_cb,", RCI_FUNCTION_T, customPrefix);
        ccapi_rci_data += String.format("\n        %s%srci_action_end_cb", RCI_FUNCTION_T, customPrefix);
        if (ConfigGenerator.rciLegacyEnabled()) {
            ccapi_rci_data += String.format(",\n        %s%srci_do_command_cb,", RCI_FUNCTION_T, customPrefix);
            ccapi_rci_data += String.format("\n        %s%srci_set_factory_defaults_cb,", RCI_FUNCTION_T, customPrefix);
            ccapi_rci_data += String.format("\n        %s%srci_reboot_cb", RCI_FUNCTION_T, customPrefix);
        }
        ccapi_rci_data += String.format("\n    },");

        ccapi_rci_data += String.format("\n    &%srci_internal_data", customPrefix);
        ccapi_rci_data += "\n};\n\n";

        bufferWriter.write(ccapi_rci_data);
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

    private void writeCcapiErrorHeader(String type, int errorIndex, String enumDefine, LinkedHashMap<String, String> errorMap, BufferedWriter bufferWriter) throws IOException {

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

    private void writeEnumValues(ConfigData configData, BufferedWriter bufferWriter, String prefix, ItemList list) throws Exception {
        for (Item item : list.getItems()) {
            assert (item instanceof Element) || (item instanceof ItemList);

            if (item instanceof Element) {
                Element element = (Element) item;

                if (element.getType() == Element.Type.ENUM) {
                    bufferWriter.write(TYPEDEF_ENUM);
                    boolean previousempty = false;
                    
                    int i = 0;
                    for (Value value : element.getValues()) {
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

    private String sanitizeName(String name) {
    	return name.replace('-', '_');
    }

    private void writePrototypes(ConfigData configData, BufferedWriter bufferWriter) throws Exception {
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

        for (Group.Type type : Group.Type.values()) {
            LinkedList<Group> groups = configData.getConfigGroup(type);;

            configType = type.toLowerName();

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

    private void writeItemPrototypes(ConfigData configData, BufferedWriter bufferWriter, String prefix, ItemList list, String retvalErrorType) throws Exception {
	    for (Item item : list.getItems()) {
            assert (item instanceof Element) || (item instanceof ItemList);
	
	        if (item instanceof Element) {
	            Element element = (Element) item;
	
	            String protoType = "";
	            switch (element.getType()) {
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
	            switch (element.getType()) {
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
	                switch (element.getType()) {
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
}
