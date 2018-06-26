package com.digi.connector.config;

import java.io.IOException;
import java.util.LinkedHashMap;
import java.util.LinkedList;

import com.digi.connector.config.ConfigGenerator.UseNames;
import com.digi.connector.config.ConfigGenerator.FileType;

public class GenFsmUserSourceFile extends GenSourceFile {
    private final static String FILENAME = "rci_config.c";

	public GenFsmUserSourceFile() throws IOException {
		super(FILENAME, GenFile.Type.USER, GenFile.UsePrefix.CUSTOM);
	}

	public void writeContent() throws Exception {
        write(String.format("%s \"%s\"\n\n", INCLUDE, "connector_api.h"));
        write(String.format("#if !(defined CONST)\n#define CONST const\n#endif\n"));
        
        write(String.format("%s \"%s\"\n\n", INCLUDE, GenFsmUserHeaderFile.FILENAME));

        /* Write Define Errors Macros */
        writeDefineRciErrors();

        writeDefineGroupErrors();

        writeDefineGlobalErrors();

        /* write remote all strings in source file */
        writeRemoteAllStrings();

        /* write connector_rci_errors[] */
        writeGlobalErrorStructures();

        /* write structures in source file */
        writeAllStructures();

        int GlobalErrorCount = config.getUserGlobalErrors().size() + config.rciGlobalErrors.size();

        write(String.format(
        		"\nconnector_remote_config_data_t const %srci_internal_data = {\n" +
                "    connector_group_table,\n"+
                "    connector_rci_errors,\n"+
                "    %d,\n"+
                "    %d,\n"+
                "    %s,\n"+
                "    \"%s\"\n"+
                "};\n"+
                "\n"+
                "connector_remote_config_data_t const * const %srci_descriptor_data = &%srci_internal_data;"
                , customPrefix, GlobalErrorCount, options.getFirmware(), Descriptors.vendorId(),Descriptors.deviceType(), customPrefix, customPrefix));
    }

    private Boolean isFirstRemoteString = true;
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

    private void writeLinkedHashMapStrings(LinkedHashMap<String, String> stringMap) throws IOException {
        for (String key : stringMap.keySet()) {
            write(getCharString(stringMap.get(key)));
        }
    }

    private void writeRciErrorsRemoteAllStrings() throws IOException {
        if (!options.excludeErrorDescription()) {
            writeLinkedHashMapStrings(config.getRciGlobalErrors());
        }
    }

    private void writeGroupRemoteAllStrings(LinkedList<Group> groups) throws Exception {
        for (Group group : groups) {
            if ((!options.excludeErrorDescription()) && (!group.getErrors().isEmpty())) {
                LinkedHashMap<String, String> errorMap = group.getErrors();
                for (String key : errorMap.keySet()) {
                    write(getCharString(errorMap.get(key)));
                }
            }
        }
    }

    private void writeErrorsRemoteAllStrings() throws IOException {
        if (!options.excludeErrorDescription()) {
            writeLinkedHashMapStrings(config.getUserGlobalErrors());
        }
    }

    private void writeRemoteAllStrings() throws Exception {
        if (!options.excludeErrorDescription()) {
            write(String.format("\nstatic char CONST %s[] = {\n",
                    CONNECTOR_REMOTE_ALL_STRING));
        }

        writeRciErrorsRemoteAllStrings();

        for (Group.Type type : Group.Type.values()) {
            LinkedList<Group> theConfig = config.getConfigGroup(type);

            configType = type.toLowerName();
            if (!theConfig.isEmpty()) {
                writeGroupRemoteAllStrings(theConfig);
            }
        }
        writeErrorsRemoteAllStrings();

        if (!options.excludeErrorDescription()) {
            write("\n};\n\n"); // end of CONNECTOR_REMOTE_ALL_STRING
        }
    }

    private void writeDefineGlobalErrors() throws IOException {
        if (!options.excludeErrorDescription()) {
            writeDefineErrors(GLOBAL_ERROR, config.getUserGlobalErrors(), FileType.SOURCE);
        }
    }

    private void writeDefineGroupErrors() throws Exception {
        for (Group.Type type : Group.Type.values()) {
            String defineName = null;
            LinkedList<Group> groups = config.getConfigGroup(type);

            configType = type.toLowerName();

            for (Group group : groups) {
                defineName = getDefineString(group.getName());
                /* define name string index
                 * #define [group name]
                 */

                if ((!options.excludeErrorDescription()) && (!group.getErrors().isEmpty())) {
                    LinkedHashMap<String, String> errorMap = group.getErrors();
                    for (String key : errorMap.keySet()) {
                        defineName = getDefineString(group.getName() + "_" + ERROR + "_" + key);
                        /* define name string index for each error in the group
                         * #define [group name + ERROR + error name]
                         */
                        write(getDefineStringIndex(defineName, errorMap.get(key), FileType.SOURCE));
                    }
                }
            }
        }

    }

    private int prevRemoteStringLength;
    private String getDefineStringIndex(String define_name, String string, FileType fileType) {
        String str = DEFINE + define_name + " " + "(" + CONNECTOR_REMOTE_ALL_STRING + "+" + prevRemoteStringLength + ")\n";
        /* NOTE: this prevRemoteStringLength is important.
         * Must keep track this variable for #define string.
         */
        if (string != null) {
            prevRemoteStringLength += string.length();
        }
        prevRemoteStringLength++; /* add extra byte for the size */

        if (fileType == options.fileTypeOption() ||
            options.fileTypeOption() == FileType.NONE) {
            return str;
        } else {
            return "";
        }
    }

    private void writeDefineErrors(String prefixName, LinkedHashMap<String, String> errorMap, FileType fileType) throws IOException {
        for (String key : errorMap.keySet()) {
            String defineName = prefixName.toUpperCase() + "_" + key.toUpperCase();
            /* define name string index for each error */
            write(getDefineStringIndex(defineName, errorMap.get(key), fileType));
        }
    }

    private void writeDefineRciErrors() throws IOException {
        if (!options.excludeErrorDescription()) {
            writeDefineErrors(GLOBAL_RCI_ERROR, config.getRciGlobalErrors(), FileType.SOURCE);
        }
    }

    private String sanitizeName(String name) {
    	return name.replace('-', '_').replace(".","_fullstop_");
    }

    private String getDefineString(String define_name) {
        return (configType.toUpperCase() + "_" + define_name.toUpperCase());
    }

    private String getAccess(String access) {
        if (access == null) {
            return "read_write";
        }
        return access;
    }

    private String getElementDefine(String type_name, String element_name) {
        return (String.format("%s_element_%s_%s", CONNECTOR_PREFIX, type_name, element_name));
    }

    private String COMMENTED(String comment) {
        return "/* " + comment + " */";
    }

    private void writeCollectionArray(ItemList items, String prefix) throws Exception {
    	// Traverse down the tree to define all the lists first as they need to be defined before the collections that include them are. 
        for (Item item: items.getItems()) {
            assert (item instanceof Element) || (item instanceof ItemList);

            String itemVariable = getDefineString(customPrefix + prefix + "__" + sanitizeName(item.getName())).toLowerCase();
            if (item instanceof Element) {
                Element element = (Element) item;
                String optional = options.useNamesOption(UseNames.ELEMENTS)
                	? String.format("    \"%s\",\n", element.getName())
                	: "";
                
                write("static connector_element_t CONST " + itemVariable + "_element = {\n");
                write(optional + "    " + getElementDefine("access", getAccess(element.getAccess())) + ",\n");
                
                if (options.rciParserOption()) {
                    String enum_struct;

                    if (element.getType() == Element.Type.ENUM) {
                        String define_name = getDefineString(prefix + "__" + element.getName() + "_enum");
                        String variableName = customPrefix + define_name.toLowerCase();
                        
                        enum_struct = "{ ARRAY_SIZE(" + variableName + "), " + variableName + "}, ";
                    }
                    else {
                    	enum_struct = "{ 0, NULL}, ";
                    }
                    
                    write(enum_struct);
                }
                write(	"};\n\n");
            } else {
            	ItemList subitems = (ItemList) item;
            	String subitemsPrefix = prefix + "__" + sanitizeName(item.getName()).toLowerCase();
            	
            	writeCollectionArray(subitems, subitemsPrefix);
            	
            	String subitemsVariable = itemVariable + "_items";
                String optional = options.useNamesOption(UseNames.COLLECTIONS)
                    	? String.format("    \"%s\",\n", subitems.getName())
                    	: "";
            	
                write("static connector_collection_t CONST " + itemVariable + "_collection = {\n");
                write(optional + "    " + subitems.getInstances() + ", " + COMMENTED("instances") + "\n");
                write("    {\n");
                write("        " + subitems.getItems().size() + ", " + COMMENTED("items") + "\n");
                write("        " + subitemsVariable + "\n");
                write("    }\n");
                write("};\n\n");
            }
        }
        
        // Then write the array for this level
    	String itemsVariable = getDefineString(customPrefix + prefix).toLowerCase();
    	boolean first = true;
        write("static connector_item_t CONST " + itemsVariable + "_items[] = {\n");
        for (Item item: items.getItems()) {
            assert (item instanceof Element) || (item instanceof ItemList);

            String type; 
            String suffix;
            if (item instanceof Element) {
            	Element element = (Element) item;
            	
            	type = element.getType().toLowerName();
            	suffix = "_element";
            } else {
            	type = "list";
            	suffix = "_collection";
            }
            if (!first) {
            	write(",\n");
            } else {
            	first = false;
            }
            
            String itemVariable = getDefineString(customPrefix + prefix + "__" + sanitizeName(item.getName())).toLowerCase() + suffix;
            write("{ " + getElementDefine("type", type) + ", { &" + itemVariable + " } }");
        }
        write("\n};\n\n");
    }
    
    private void writeEnumArray(Element element, String prefix) throws Exception {
        boolean first = true;
        String define_name = getDefineString(prefix + "__" + element.getName() + "_enum");
        
        write(String.format("static connector_element_enum_t CONST %s%s[] = {\n",
                customPrefix, define_name.toLowerCase()));
        
        for (Value value : element.getValues()) {
        	if (!first) {
                write(",\n");
        	} else {
        		first = false;
        	}
        	
            write(String.format("    {\"%s\"}",value.getName()));
        }
        write("\n};\n\n");
    }
    
    private void writeEnumArrays(ItemList items, String prefix) throws Exception{
        for (Item item: items.getItems()) {
            assert (item instanceof Element) || (item instanceof ItemList);

            if (item instanceof Element) {
                Element element = (Element) item;
                
                if (element.getType() == Element.Type.ENUM) {
                	writeEnumArray(element, prefix);
                }
            } else {
            	ItemList subitems = (ItemList) item;
            	
            	writeEnumArrays(subitems, prefix + "__" + sanitizeName(subitems.getName()));
            }
        }
    }

    private void writeCollectionArrays(ItemList items, String prefix) throws Exception {
        if (options.rciParserOption()) {
            writeEnumArrays(items, prefix);
        }

        writeCollectionArray(items, prefix);
    }

    private String getRemoteString(String define_name) {
        return (" " + define_name.toUpperCase());

    }

    private int writeErrorStructures(int errorCount, String prefix, LinkedHashMap<String, String> errorMap) throws IOException {
    	String defineName = prefix.toUpperCase();
    	
        for (String key : errorMap.keySet()) {
            write(getRemoteString(defineName + "_" + key));
            errorCount--;
            if (errorCount > 0) {
                write(",");
            }
            write(COMMENTED(key) + "\n");
        }

        return errorCount;
    }

    private void writeLocalErrorStructures(String error_name, LinkedHashMap<String, String> localErrors) throws IOException {
        if (!options.excludeErrorDescription()) {
        	int errorCount = localErrors.size();
        	 
	        if (errorCount > 0) {
	            String define_name = getDefineString(error_name + "_" + ERROR);
	            
	            write(CHAR_CONST_STRING + customPrefix + define_name.toLowerCase() + "s[] = {\n");
	            writeErrorStructures(errorCount, define_name, localErrors);
	            write("};\n\n");
	        }
	    }
    }
    
    private void writeGroupStructures(LinkedList<Group> groups) throws Exception {
        for (Group group: groups) {
            writeCollectionArrays(group, group.getName());
            
            if (!options.excludeErrorDescription()) {
            	writeLocalErrorStructures(group.getName(), group.getErrors());
            }
        }
    }

    private void writeAllStructures() throws Exception {
        for (Group.Type type : Group.Type.values()) {
            LinkedList<Group> groups = config.getConfigGroup(type);

            configType = type.toLowerName();

            if (!groups.isEmpty()) {
                writeGroupStructures(groups);

                write(String.format("static connector_group_t CONST %sconnector_%s_groups[] = {", customPrefix, configType));

                for (int group_index = 0; group_index < groups.size(); group_index++) {
                    Group group = groups.get(group_index);
                    String items_name = customPrefix + getDefineString(group.getName() + "_items").toLowerCase();
                    String optional = options.useNamesOption(UseNames.COLLECTIONS)
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
                    
                    if ((!options.excludeErrorDescription()) && (!group.getErrors().isEmpty())) {
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
                    write(group_string);
                }
                write("\n};\n\n");
            }
        }

    	LinkedList<String> group_lines = new LinkedList<>();
        for (Group.Type type : Group.Type.values()) {
            LinkedList<Group> groups = config.getConfigGroup(type);

            if (!groups.isEmpty()) {
            	group_lines.add("    { NULL, 0 }");
            } else {
                String var_name = customPrefix + "connector_" + type.toLowerName() + "_groups";

                group_lines.add(String.format("    { %s, ARRAY_SIZE(%s) }", var_name, var_name));
            }

        }

        write(
        		String.format("static connector_remote_group_table_t CONST %s[] =\n", CONNECTOR_REMOTE_GROUP_TABLE) +
        		"{\n" +
        		String.join(",\n", group_lines) + "\n" +
                "};\n" +
        		"\n");
    }

    private void writeGlobalErrorStructures() throws IOException {
        if (!options.excludeErrorDescription()) {
            int errorCount = config.getRciGlobalErrors().size() + config.getUserGlobalErrors().size();

            if (errorCount > 0) {
                write(String.format("static char const * const %ss[] = {\n", GLOBAL_RCI_ERROR));

                /* top-level global errors */
                errorCount = writeErrorStructures(errorCount, GLOBAL_RCI_ERROR, config.getRciGlobalErrors());
                /* group global errors */
                errorCount = writeErrorStructures(errorCount, GLOBAL_ERROR, config.getUserGlobalErrors());

                write("};\n\n");
            }
        }
        else {
            write(String.format("%s%ss NULL\n", DEFINE, GLOBAL_RCI_ERROR));
        }
    }
}
