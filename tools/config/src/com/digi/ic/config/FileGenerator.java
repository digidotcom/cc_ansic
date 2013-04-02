package com.digi.ic.config;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.LinkedHashMap;
import java.util.LinkedList;

public class FileGenerator {

    private final static String HEADER_FILENAME = "remote_config.h";
    private final static String SOURCE_NAME = "rci_config";

    private final static String CONNECTOR_PREFIX = "connector";
    private final static String DEFINE = "#define ";
    private final static String INCLUDE = "#include ";
    private final static String ERROR = "error";

    private final static String CONNECTOR_REMOTE_HEADER = "\"connector_api_remote.h\"\n\n";
    private final static String FLOAT_HEADER = "\"float.h\"\n";

    private final static String TYPEDEF_ENUM = "typedef enum {\n";

    private final static String GLOBAL_RCI_ERROR = "connector_rci_error";
    private final static String GLOBAL_ERROR = "connector_global_error";

    private final static String CONNECTOR_REMOTE_ALL_STRING = "connector_remote_all_strings";
    private final static String CONNECTOR_REMOTE_GROUP_TABLE = "connector_group_table";

    private final static String COUNT_STRING = "COUNT";
    private final static String OFFSET_STRING = "OFFSET";
    private final static String STATIC = "static ";
    private final static String CONST = " const ";

    private final static String CHAR_CONST_STRING = STATIC + "char" + CONST
            + "*" + CONST;
    private final static String ID_T_STRING = "_id_t;\n\n";
    private final static String TYPEDEF_STRUCT = "\ntypedef struct {\n";

    /* Do not change these (if you do, you also need to update connector_remote.h */
    private final static String RCI_PARSER_USES = "RCI_PARSER_USES_";
    
    private final static String RCI_PARSER_USES_ERROR_DESCRIPTIONS = RCI_PARSER_USES + "ERROR_DESCRIPTIONS\n";
    private final static String RCI_PARSER_USES_STRINGS = RCI_PARSER_USES + "STRINGS\n";
    private final static String RCI_PARSER_USES_UNSIGNED_INTEGER = RCI_PARSER_USES + "UNSIGNED_INTEGER\n";
    
    private final static String RCI_PARSER_DATA = "CONNECTOR_RCI_PARSER_INTERNAL_DATA";
    
    private final static String CONNECTOR_REMOTE_CONFIG_DATA_STRUCTURE = "\ntypedef struct {\n" +
    " connector_remote_group_table_t const * group_table;\n" +
	" char const * const * error_table;\n" +
	" unsigned int global_error_count;\n" +
	" uint32_t firmware_target_zero_version;\n" +
	"} connector_remote_config_data_t;\n";

    private final static String CONNECTOR_GROUP_ELEMENT_T = "\ntypedef struct {\n" +
    "    connector_element_access_t access;\n" +
    "    connector_element_value_type_t type;\n" +
    "} connector_group_element_t;\n";

    private final static String CONNECTOR_GROUP_T = "\ntypedef struct {\n" +
    "  size_t instances;\n" +
    "  struct {\n" +
    "    size_t count;\n" +
    "    connector_group_element_t const * const data;\n" +
    "  } elements;\n\n" +
    "  struct {\n" +
    "      size_t count;\n" +
    "     char const * const * description;\n" +
    "  } errors;\n" +
    "} connector_group_t;\n";

    private final static String CONNECTOR_REMOTE_GROUP_T = "\ntypedef struct {\n" +
    "  connector_remote_group_type_t type;\n" +
    "  unsigned int id;\n" +
    "  unsigned int index;\n" +
    "} connector_remote_group_t;\n";
    	
    private final static String CONNECTOR_REMOTE_ELEMENT_T = "\ntypedef struct {\n" +
    "  unsigned int id;\n" +
    "  connector_element_value_type_t type;\n" +
    "  connector_element_value_t * value;\n" +
    "} connector_remote_element_t;\n";

    private final static String CONNECTOR_REMOTE_CONFIG_T = "\ntypedef struct {\n" +
    "  void * user_context;\n" +
    "  connector_remote_action_t action;\n" +
    "  connector_remote_group_t group;\n" +
    "  connector_remote_element_t element;\n" +
    "  unsigned int error_id;\n" +
    "\n" +
    "  union {\n" +
    "      char const * error_hint;\n" +
    "      connector_element_value_t * element_value;\n" +
    "  } response;\n" +
    "} connector_remote_config_t;\n";
    	
    private final static String CONNECTOR_REMOTE_CONFIG_CANCEL_T = "\ntypedef struct {\n" +
    "  void * user_context;\n" +
    "} connector_remote_config_cancel_t;\n";
        
    private final static String CONNECTOR_REMOTE_GROUP_TABLE_T = "\ntypedef struct {\n" +
    "  connector_group_t const * groups;\n" +
    "  size_t count;\n" +
    "} connector_remote_group_table_t;\n";
    	
    private String filePath = "";
    private String generatedFile = "";
    private String headerFile = "";
    private final BufferedWriter fileWriter;
    private BufferedWriter headerWriter = null;
    private String configType;
    private int prevRemoteStringLength;
    private Boolean isFirstRemoteString;

    private void writeHeaderComment(BufferedWriter bufferWriter) throws IOException {
        DateFormat dateFormat = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss");
        Date date = new Date();

        String className = ConfigGenerator.class.getName();

        int firstChar = className.lastIndexOf(".") + 1;
        if (firstChar > 0) {
            className = className.substring(firstChar);
        }

        String note_string = "/*\n * This is an auto-generated file - DO NOT EDIT! \n";
        
        switch (ConfigGenerator.fileTypeOption()) {
        case SOURCE:
            if (bufferWriter == headerWriter) 
                note_string += String.format(" * This is a H file generated by %s tool.\n", className);
            else 
                note_string += String.format(" * This is a C file generated by %s tool.\n", className);
            break;
            
        case GLOBAL_HEADER:
            note_string += String.format(" * This is a public header file generated by %s tool.\n", className);
            break;
            
        default:
            note_string += String.format(" * This is generated by %s tool \n", className);
            break;
        }
        
        note_string += String.format(" * This file was generated on: %s \n", dateFormat.format(date))
                     + String.format(" * The command line arguments were: %s\n", ConfigGenerator.getArgumentLogString())
                     + String.format(" * The version of %s tool was: %s */\n\n", className, ConfigGenerator.VERSION);
        
        bufferWriter.write(note_string);
        
    }
    
    public FileGenerator(String directoryPath) throws IOException {
        
        if (directoryPath != null) {
            filePath = directoryPath;
            /* add a / if last char is not / */
            if (!directoryPath.endsWith("/")) filePath += '/';
        }
        
        switch (ConfigGenerator.fileTypeOption()) {
        case SOURCE:
            generatedFile = SOURCE_NAME + ".c";
            headerFile = SOURCE_NAME + ".h";
            break;
            
        default:
            generatedFile = HEADER_FILENAME;
            break;
        }        
        fileWriter = new BufferedWriter(new FileWriter(filePath + generatedFile));
        
        switch (ConfigGenerator.fileTypeOption()) {
        case SOURCE:
            /* Need to create a header file + source file
             * fileWriter is source file writer. 
             * So we need to create local header file.
             */
            headerWriter = new BufferedWriter(new FileWriter(filePath + headerFile));
            writeHeaderComment(headerWriter);
            break;
            
        case GLOBAL_HEADER:
            for (ElementStruct.ElementType type : ElementStruct.ElementType.values()) {
                type.set();
            }
            break;
            
        default:
            break;
        }
        
        writeHeaderComment(fileWriter);
        
        isFirstRemoteString = true;
    }

    public void generateFile(ConfigData configData) throws Exception {
        try {

            String defineName = generatedFile.replace('.', '_').toLowerCase();
            

            switch (ConfigGenerator.fileTypeOption()) {
            case GLOBAL_HEADER:
                fileWriter.write(String.format("#ifndef %s\n#define %s\n\n", defineName, defineName));
                fileWriter.write(String.format("%s \"connector_api.h\"\n\n", INCLUDE));
                
                fileWriter.write("#define CONNECTOR_BINARY_RCI_SERVICE \n");
                fileWriter.write("#define connector_request_id_remote_config_configurations    (connector_request_id_remote_config_t)-1\n\n");

                writeHeaderFile(configData);
                break;
                
             case SOURCE:
                /* write include header in the header file */
                 String headerDefineName = headerFile.replace('.', '_').toLowerCase();
                headerWriter.write(String.format("#ifndef %s\n#define %s\n\n", headerDefineName, headerDefineName));
                headerWriter.write(String.format("%s \"%s\"\n\n", INCLUDE, HEADER_FILENAME));
                
                fileWriter.write(String.format("%s \"%s\"\n\n", INCLUDE, headerFile));
                writeHeaderFile(configData);
                fileWriter.write(String.format("uint32_t const FIRMWARE_TARGET_ZERO_VERSION = 0x%X;\n\n", ConfigGenerator.getFirmware()));
                break;
                
            case NONE:
                fileWriter.write(String.format("#ifndef %s\n#define %s\n\n", defineName, defineName));
                writeHeaderFile(configData);
                /*
                 * Start writing:
                 * 1. all #define for all strings from user's groups 
                 * 2. all #define for all RCI and user's global errors 
                 * 3. all strings in connector_remote_all_strings[]
                 */
                fileWriter.write(String.format("\n\n#if defined %s\n\n", RCI_PARSER_DATA));
                fileWriter.write("#define CONNECTOR_BINARY_RCI_SERVICE \n\n");
                
                fileWriter.write(String.format("#define FIRMWARE_TARGET_ZERO_VERSION  0x%X\n\n",ConfigGenerator.getFirmware()));
                break;
             }
            

            switch (ConfigGenerator.fileTypeOption()) {
            case GLOBAL_HEADER:
                fileWriter.write(CONNECTOR_REMOTE_CONFIG_DATA_STRUCTURE);
                fileWriter.write(String.format("\n#endif /* %s */\n", defineName));
                break;
            case SOURCE:
                headerWriter.write(String.format("\n\nextern uint32_t const FIRMWARE_TARGET_ZERO_VERSION;\n"));
                headerWriter.write("extern unsigned int connector_global_error_COUNT;\n\n");
                headerWriter.write(String.format("extern char const * const %ss[];\n",GLOBAL_RCI_ERROR));
                headerWriter.write(String.format("extern connector_group_table_t const %s[];\n", CONNECTOR_REMOTE_GROUP_TABLE));
                /* no break */
            case NONE:
                writeDefineRciErrors(configData);
                
                /* Write all string length and index defines in C file */
                writeDefineStrings(configData);

                /* Write all string length and index defines in C file */
                writeDefineGlobalErrors(configData);
                
                /* write idigi remote all strings in source file */
                writeRemoteAllStrings(configData);
                
                writeGlobalErrorStructures(configData);

                /* write structures in source file */
                writeAllStructures(configData);
                
                switch (ConfigGenerator.fileTypeOption()) {
                case NONE:
                    fileWriter.write(String.format("\n#endif /* %s */\n\n", RCI_PARSER_DATA));
                    fileWriter.write(String.format("\n#endif /* %s */\n", defineName));
                    break;
                case SOURCE:
                    headerWriter.write(String.format("\n#endif /* %s */\n", defineName));
                    break;
                }
                
                break;
            }
            
            ConfigGenerator.log(String.format("Files created:\n\t%s%s",  filePath, generatedFile));
            if (headerFile.length() > 0) ConfigGenerator.log(String.format("\t%s%s", filePath, headerFile));


        } catch (IOException e) {
            throw new IOException(e.getMessage());
        }

        finally {
            fileWriter.close();
            if (headerWriter != null) headerWriter.close();
        }

    }

    private void writeHeaderFile(ConfigData configData) throws Exception {

        writeDefinesAndStructures(configData);
        
        /* Write all global error enum in H file */
        writeRciErrorEnumHeader(configData);
        
        writeGlobalErrorEnumHeader(configData);

        /* Write all group enum in H file */
        writeGroupEnumHeader(configData);
   
    }

    private void writeOnOffBooleanEnum() throws IOException {

        String enumString = "";

        for (ElementStruct.ElementType type : ElementStruct.ElementType.values()) {
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
        int value = -1;
       
        for (ElementStruct.ElementType type : ElementStruct.ElementType.values()) {
            if (type.isSet()) {
                if (!isFirstEnum) {
                    enumString += ",\n";
                }
                isFirstEnum = false;
                
                enumString += "    connector_element_type_" + type.toName().toLowerCase();
                
                if (type.toValue() != (value +1)) {
                    enumString += String.format(" = %d", type.toValue());
                }
                value = type.toValue();
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
    
    private void writeElementValueStruct() throws IOException {

        String headerString = "";
        String structString = "";
        String elementValueStruct = "";
        
        int optionCount = 0;

        Boolean isUnsignedIntegerDefined = false;
        Boolean isStringDefined = false;
        Boolean isEnumValueStructDefined = false;

        for (ElementStruct.ElementType type : ElementStruct.ElementType.values()) {
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
                    optionCount++;
                    break;
                    
                case ENUM:
                    if (!isEnumValueStructDefined) {
                        structString += enumStructureString();
                        isEnumValueStructDefined = true;
                    }
                    elementValueStruct += "    unsigned int enum_value;\n";
                    optionCount++;
                    break;
                    
                case FLOAT:
                    structString += TYPEDEF_STRUCT
                                    + "    float min_value;\n"
                                    + "    float max_value;\n"
                                    + "} connector_element_value_float_t;\n";
                    elementValueStruct += "    float float_value;\n";
                    optionCount++;
                    break;
                    
                case ON_OFF:
                    if (!isEnumValueStructDefined) {
                        /* rci parser needs this structure for on/off type */
                        structString += enumStructureString();
                        isEnumValueStructDefined = true;
                    }
                    elementValueStruct += "    connector_on_off_t  on_off_value;\n";
                    optionCount++;
                    break;
                    
                case BOOLEAN:
                    if (!isEnumValueStructDefined) {
                        /* rci parser needs this structure for boolean type */
                        structString += enumStructureString();
                        isEnumValueStructDefined = true;
                    }
                    elementValueStruct += "    connector_bool_t  boolean_value;\n";
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
                        isStringDefined = true;
                        optionCount++;
                     }
                    break;
                }
            }
        }
        
        headerString += structString;
        
        if (optionCount > 1) {
            headerString += "\n\ntypedef union {\n";
        } else {
            headerString += "\n\ntypedef struct {\n";
        }
        
        headerString += elementValueStruct + "} connector_element_value_t;\n";
        
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

        for (ElementStruct.ElementType type : ElementStruct.ElementType.values()) {
            if (type.isSet()) {
                
                headerString += DEFINE + RCI_PARSER_USES + type.toName().toUpperCase() + "\n";
                
                switch (type) {
                case UINT32:
                case HEX32:
                case X_HEX32:
                    if (unsignedIntegerString == null) {
                        /* if not defined yet, then define it */
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
                        /* if not defined yet then define it */
                        stringsString = DEFINE + RCI_PARSER_USES_STRINGS;
                     }
                    break;
                }
            } else if (!type.isSet()) {
                switch (type) {
                case ON_OFF:
                    break;
                    
                case BOOLEAN:
                     break;
                default:
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

    private void writeDefinesAndStructures(ConfigData configData) throws IOException {

        switch (ConfigGenerator.fileTypeOption()) {
        case GLOBAL_HEADER:
        case NONE:
            writeDefineOptionHeader(configData);
            writeOnOffBooleanEnum();
            writeElementTypeEnum();
            writeElementValueStruct();
            String headerString = "\n\n" + INCLUDE + CONNECTOR_REMOTE_HEADER;

            fileWriter.write(headerString);
            
            fileWriter.write(CONNECTOR_GROUP_ELEMENT_T);
            fileWriter.write(CONNECTOR_GROUP_T);
            fileWriter.write(CONNECTOR_REMOTE_GROUP_T);
            fileWriter.write(CONNECTOR_REMOTE_ELEMENT_T);
            fileWriter.write(CONNECTOR_REMOTE_CONFIG_T);
            fileWriter.write(CONNECTOR_REMOTE_CONFIG_CANCEL_T);
            fileWriter.write(CONNECTOR_REMOTE_GROUP_TABLE_T);
            break;
        }
        
    }

    private void writeRemoteAllStrings(ConfigData configData) throws Exception {
        if (!ConfigGenerator.excludeErrorDescription()) {
            fileWriter.write(String.format("\nchar const %s[] = {\n",
                    CONNECTOR_REMOTE_ALL_STRING));
        }
        
        writeRciErrorsRemoteAllStrings(configData);

        for (ConfigData.ConfigType type : ConfigData.ConfigType.values()) {
            LinkedList<GroupStruct> theConfig = null;

            configType = type.toString().toLowerCase();

            theConfig = configData.getConfigGroup(configType);

            if (!theConfig.isEmpty()) {
                writeGroupRemoteAllStrings(theConfig);
            }
        }
        writeErrorsRemoteAllStrings(configData);

        if (!ConfigGenerator.excludeErrorDescription()) {
            fileWriter.write("\n};\n\n"); // end of CONNECTOR_REMOTE_ALL_STRING
        }
    }

    private void writeDefineStrings(ConfigData configData) throws Exception {
        for (ConfigData.ConfigType type : ConfigData.ConfigType.values()) {
            String defineName = null;
            LinkedList<GroupStruct> groups = null;

            configType = type.toString().toLowerCase();

            groups = configData.getConfigGroup(configType);

            for (GroupStruct group : groups) {
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
                        fileWriter.write(getDefineStringIndex(defineName, errorMap.get(key), ConfigGenerator.FileType.SOURCE));
                    }
                }
            }
        }

    }

    private void writeGroupRemoteAllStrings(LinkedList<GroupStruct> groups) throws Exception {
        for (GroupStruct group : groups) {
            if ((!ConfigGenerator.excludeErrorDescription()) && (!group.getErrors().isEmpty())) {
                LinkedHashMap<String, String> errorMap = group.getErrors();
                for (String key : errorMap.keySet()) {
                    fileWriter.write(getCharString(errorMap.get(key)));
                }
            }
        }
    }

    private void writeDefineErrors(String prefixName, LinkedHashMap<String, String> errorMap, ConfigGenerator.FileType fileType) throws IOException {
        for (String key : errorMap.keySet()) {
            String defineName = prefixName.toUpperCase() + "_" + key.toUpperCase();
            /* define name string index for each error */
            fileWriter.write(getDefineStringIndex(defineName, errorMap.get(key), fileType));
        }
    }

    private void writeDefineGlobalErrors(ConfigData configData) throws IOException {
        if (!ConfigGenerator.excludeErrorDescription()) {
            writeDefineErrors(GLOBAL_ERROR, configData.getUserGlobalErrors(), ConfigGenerator.FileType.SOURCE);
        }
    }

    private void writeDefineRciErrors(ConfigData configData) throws IOException {
        if (!ConfigGenerator.excludeErrorDescription()) {
            writeDefineErrors(GLOBAL_RCI_ERROR, configData.getRciGlobalErrors(), ConfigGenerator.FileType.SOURCE);
        }
    }
    
    private void writeLinkedHashMapStrings(LinkedHashMap<String, String> stringMap) throws IOException {
        for (String key : stringMap.keySet()) {
            fileWriter.write(getCharString(stringMap.get(key)));
        }
    }

    private void writeErrorsRemoteAllStrings(ConfigData configData) throws IOException {
        if (!ConfigGenerator.excludeErrorDescription()) {
            writeLinkedHashMapStrings(configData.getUserGlobalErrors());
        }
    }

    private void writeRciErrorsRemoteAllStrings(ConfigData configData) throws IOException {
        if (!ConfigGenerator.excludeErrorDescription()) {
            writeLinkedHashMapStrings(configData.getRciGlobalErrors());
        }
    }

    private void writeElementArrays(String group_name, LinkedList<ElementStruct> elements) throws Exception {
        /* write group element structure array */
        fileWriter.write(String.format("static connector_group_element_t const %s_elements[] = {",
                                        getDefineString(group_name).toLowerCase()));

        for (int element_index = 0; element_index < elements.size(); element_index++) {
            ElementStruct element = elements.get(element_index);

            String element_string = String.format("\n { %s", COMMENTED(element.getName()));


            element_string += String.format("   %s,\n",  getElementDefine("access", getAccess(element.getAccess())));
            
            element_string += String.format("   %s", getElementDefine("type", element.getType()));

            element_string += "\n }";

            if (element_index < (elements.size() - 1)) {
                element_string += ",";
            }

            fileWriter.write(element_string);
        }
        fileWriter.write("\n};\n\n");

    }

    private void writeGlobalErrorStructures(ConfigData configData) throws IOException {
        
        if (!ConfigGenerator.excludeErrorDescription()) {
            int errorCount = configData.getRciGlobalErrors().size()
                            + configData.getUserGlobalErrors().size();

            if (errorCount > 0) {
                String staticString = "static ";
                
                if (ConfigGenerator.fileTypeOption() == ConfigGenerator.FileType.SOURCE)  staticString = "";

                fileWriter.write(String.format("%schar const * const %ss[] = {\n", staticString, GLOBAL_RCI_ERROR));
                        
                /* top-level global errors */
                errorCount = writeErrorStructures(errorCount, GLOBAL_RCI_ERROR,
                             configData.getRciGlobalErrors());

                /* group global errors */
                errorCount = writeErrorStructures(errorCount, GLOBAL_ERROR,
                             configData.getUserGlobalErrors());

                fileWriter.write("};\n\n");
            }
        }
    }

    private int writeErrorStructures(int errorCount, String defineName, LinkedHashMap<String, String> errorMap) throws IOException {
        
        for (String key : errorMap.keySet()) {
            fileWriter.write(getRemoteString(defineName.toUpperCase() + "_" + key));
            errorCount--;
            if (errorCount > 0) {
                fileWriter.write(",");
            }
            fileWriter.write(COMMENTED(key));
        }

        return errorCount;
    }

    private void writeErrorStructures(String error_name, LinkedHashMap<String, String> localErrors) throws IOException {
        
        if (!ConfigGenerator.excludeErrorDescription()) {
            String define_name;

            if (!localErrors.isEmpty()) {
                define_name = getDefineString(error_name + "_" + ERROR);
                fileWriter.write(CHAR_CONST_STRING + define_name.toLowerCase() + "s[] = {\n");

                /* local local errors */
                define_name = getDefineString(error_name + "_" + ERROR);
                int error_count = localErrors.size();
                writeErrorStructures(error_count, define_name, localErrors);

                fileWriter.write("};\n\n");
            }
        }
    }

    private void writeGroupStructures(LinkedList<GroupStruct> groups) throws Exception {
        
        for (int group_index = 0; group_index < groups.size(); group_index++) {
            GroupStruct group = groups.get(group_index);

            /* write element structure */
            writeElementArrays(group.getName(), group.getElements());

            writeErrorStructures(group.getName(), group.getErrors());
        }

    }

    private void writeAllStructures(ConfigData configData) throws Exception {
        String define_name;

        for (ConfigData.ConfigType type : ConfigData.ConfigType.values()) {
            LinkedList<GroupStruct> groups = null;

            configType = type.toString().toLowerCase();

            groups = configData.getConfigGroup(configType);

            if (!groups.isEmpty()) {
                writeGroupStructures(groups);

                fileWriter.write(String.format("static connector_group_t const connector_%s_groups[] = {", configType));

                for (int group_index = 0; group_index < groups.size(); group_index++) {
                    GroupStruct group = groups.get(group_index);

                    define_name = getDefineString(group.getName() + "_elements");
                    String group_string = String.format("\n { %s", COMMENTED(group.getName()));

                    group_string += String.format("   %d,\n", group.getInstances())
                                  + String.format("   { asizeof(%s),\n", define_name.toLowerCase())
                                  + String.format("     %s\n   },\n", define_name.toLowerCase());

                    if ((!ConfigGenerator.excludeErrorDescription()) && (!group.getErrors().isEmpty())) {
                        define_name = getDefineString(group.getName() + "_errors");

                        group_string += String.format("   { asizeof(%s),\n", define_name.toLowerCase())
                                        + String.format("     %s\n   }\n }\n", define_name.toLowerCase());

                    } else {
                        group_string += "   { 0,\n     NULL\n   }\n }";
                    }

                    if (group_index < (groups.size() - 1)) {
                        group_string += ",";
                    }

                    fileWriter.write(group_string);
                }
                fileWriter.write("\n};\n\n");
            }
        }

        String idigiGroupString = "static ";
        if (ConfigGenerator.fileTypeOption() == ConfigGenerator.FileType.SOURCE) idigiGroupString = "";
        
        idigiGroupString += String.format("connector_group_table_t const %s[] = {\n",
                                                CONNECTOR_REMOTE_GROUP_TABLE);

        for (ConfigData.ConfigType type : ConfigData.ConfigType.values()) {
            LinkedList<GroupStruct> groups = null;

            configType = type.toString().toLowerCase();

            groups = configData.getConfigGroup(configType);

            if (type.getIndex() != 0) {
                idigiGroupString += ",\n";
            }

            idigiGroupString += " " + "{";
            if (!groups.isEmpty()) {
                idigiGroupString += String.format("connector_%s_groups,\n asizeof(connector_%s_groups)\n }",
                                                   configType, configType);

            } else {
                idigiGroupString += "NULL,\n 0\n }";
            }

        }
        idigiGroupString += "\n};\n\n";

        fileWriter.write(idigiGroupString);
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
    private void writeRciErrorEnumHeader(ConfigData configData) throws IOException {

        switch (ConfigGenerator.fileTypeOption()) {
        case GLOBAL_HEADER:
        case NONE:
        /* write typedef enum for rci errors */
            fileWriter.write("\n" + TYPEDEF_ENUM + " " + GLOBAL_RCI_ERROR + "_" + OFFSET_STRING + " = 1,\n");
            writeErrorHeader(configData.getRciGlobalErrorsIndex(), GLOBAL_RCI_ERROR, configData.getRciGlobalErrors(), fileWriter);
            fileWriter.write(" " + GLOBAL_RCI_ERROR + "_" + COUNT_STRING + "\n} " + GLOBAL_RCI_ERROR + ID_T_STRING);
        }
    }
    
    private void writeGlobalErrorEnumHeader(ConfigData configData) throws IOException {

        BufferedWriter bufferWriter = fileWriter;
        String index_string = "";
        
        switch (ConfigGenerator.fileTypeOption()) {
            case SOURCE:
                bufferWriter = headerWriter;
                index_string = "_INDEX";
                
                fileWriter.write(String.format("unsigned int %s_%s = %s_%s%s;\n\n", GLOBAL_ERROR, COUNT_STRING, GLOBAL_ERROR, COUNT_STRING, index_string));
                /* fall thru */
            case NONE:
                /* write typedef enum for user global error */
                String enumName = GLOBAL_ERROR + "_" + OFFSET_STRING;
        
                bufferWriter.write("\n" + TYPEDEF_ENUM + " " + enumName + " = " + GLOBAL_RCI_ERROR + "_" + COUNT_STRING + ",\n");
        
                writeErrorHeader(1, GLOBAL_ERROR, configData.getUserGlobalErrors(), bufferWriter);
        
                String endString = String.format(" %s_%s%s", GLOBAL_ERROR, COUNT_STRING, index_string);
                
                if (configData.getUserGlobalErrors().isEmpty()) {
                    endString += " = " + enumName;
                }
                endString += "\n} " + GLOBAL_ERROR + ID_T_STRING;
                
                bufferWriter.write(endString);
                break;
        }
    }

    private void writeEnumHeader(LinkedList<GroupStruct> groups, BufferedWriter bufferWriter) throws Exception {

        for (GroupStruct group : groups) {
            /* build element enum string for element enum */
            String element_enum_string = TYPEDEF_ENUM;

            for (ElementStruct element : group.getElements()) {
                /* add element name */
                element_enum_string += getEnumString(group.getName() + "_"
                        + element.getName())
                        + ",\n";

                if (ElementStruct.ElementType.toElementType(element.getType()) == ElementStruct.ElementType.ENUM) {
                    /* write typedef enum for value */
                    bufferWriter.write(TYPEDEF_ENUM);

                    for (ValueStruct value : element.getValues()) {
                        bufferWriter.write(getEnumString(group.getName() + "_" + element.getName() + "_" + value.getName()) + ",\n");
                    }
                    /* done typedef enum for value */
                    bufferWriter.write(endEnumString(group.getName() + "_" + element.getName()));
                }
            }
            /* done typedef enum for element */

            element_enum_string += endEnumString(group.getName());

            bufferWriter.write(element_enum_string);

            if (!group.getErrors().isEmpty()) {
                bufferWriter.write(TYPEDEF_ENUM);

                LinkedHashMap<String, String> errorMap = group.getErrors();
                int index = 0;

                for (String key : errorMap.keySet()) {
                    String enumString = getEnumString(group.getName() + "_" + ERROR + "_" + key);
                    if (index++ == 0) {
                        /* Set start index to the global count 
                         */
                        String startIndexString = COUNT_STRING;
                        switch (ConfigGenerator.fileTypeOption()) {
                        case SOURCE:
                            startIndexString += "_INDEX";
                        }
                        enumString += " = " + GLOBAL_ERROR + "_" + startIndexString;
                    }

                    enumString += ",\n";

                    bufferWriter.write(enumString);
                }
                bufferWriter.write(endEnumString(group.getName() + "_" + ERROR));
            }

        }

    }

    private void writeGroupEnumHeader(ConfigData configData) throws Exception {
        switch (ConfigGenerator.fileTypeOption()) {
        case SOURCE:
        case NONE:
            BufferedWriter bufferWriter;
        
            switch (ConfigGenerator.fileTypeOption()) {
            case SOURCE:
                bufferWriter = headerWriter;
                break;
            default:
                bufferWriter = fileWriter;
                break;
            }
            for (ConfigData.ConfigType type : ConfigData.ConfigType.values()) {
                LinkedList<GroupStruct> groups = null;
    
                configType = type.toString().toLowerCase();
                groups = configData.getConfigGroup(configType);
    
                if (!groups.isEmpty()) {
                    /* build group enum string for group enum */
                    String group_enum_string = TYPEDEF_ENUM;
    
                    /* Write all enum in H file */
                    writeEnumHeader(groups, bufferWriter);
    
                    for (GroupStruct group : groups) {
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

    private String COMMENTED(String comment) {
        return " /*" + comment + "*/\n";
    }

    private String getEnumString(String enum_name) {
        String str = " " + CONNECTOR_PREFIX + "_" + configType;

        if (enum_name != null) {
            str += "_" + enum_name;
        }
        return str;
    }

    private String endEnumString(String group_name) {
        return (getEnumString(group_name) + "_" + COUNT_STRING + "\n}"
                + getEnumString(group_name) + ID_T_STRING);
    }

    private String getDefineString(String define_name) {
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

    private String getElementDefine(String type_name, String element_name) {
        return (String.format("%s_element_%s_%s", CONNECTOR_PREFIX, type_name, element_name));
    }

    private String getAccess(String access) {
        if (access == null) {
            return "read_write";
        }
        return access;
    }

}
