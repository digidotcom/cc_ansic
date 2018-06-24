package com.digi.connector.config;

import java.io.BufferedWriter;
import java.io.IOException;
import java.util.LinkedHashMap;
import java.util.LinkedList;

public class GenFsmUserHeaderFile extends GenHeaderFile {
    private final static String FILENAME = ConfigGenerator.getCustomPrefix() + "rci_config.h";
    static public String getBasename() { return FILENAME; }

	public GenFsmUserHeaderFile(String path) throws IOException {
		super(path, FILENAME, GenFile.Type.USER);
	}
	
    public void generateFile(ConfigData configData) throws Exception {
        try {
        	writePreamble();
        	
            /* write include header in the header file */
             String headerDefineName = FILENAME.replace('.', '_').toLowerCase();
            fileWriter.write(String.format("#ifndef %s\n#define %s\n\n", headerDefineName, headerDefineName));

        	writeGlobalErrorEnumHeader(configData,fileWriter);  
        	writeGroupTypeAndErrorEnum(configData,fileWriter);

        	fileWriter.write(String.format("\n#endif\n"));
 

            ConfigGenerator.log(String.format("Files created:\n\t%s%s",  filePath, FILENAME));
        } catch (IOException e) {
            throw new IOException(e.getMessage());
        } finally {
        	fileWriter.close();
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
    
    private String getEnumString(String enum_name) {
        String str = " " + customPrefix + CONNECTOR_PREFIX + "_" + configType;

        if (enum_name != null) {
            str += "_" + enum_name;
        }
        return str;
    }

    private String sanitizeName(String name) {
    	return name.replace('-', '_');
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
