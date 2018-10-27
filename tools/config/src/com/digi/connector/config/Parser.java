package com.digi.connector.config;

import java.io.IOException;
import java.math.BigDecimal;
import java.util.ArrayDeque;
import java.util.Arrays;
import java.util.LinkedHashSet;
import java.util.List;

import javax.script.ScriptEngine;
import javax.script.ScriptEngineManager;

import java.nio.charset.StandardCharsets;
import java.nio.charset.CharsetEncoder;

import com.digi.connector.config.ConfigGenerator.UseNames;

public class Parser {

    private final static int MAX_DESCRIPTION_LENGTH = 200;
    
    private static TokenScanner tokenScanner;
    private static String token;
    private static int groupLineNumber;
    private static int elementLineNumber;
    private static ArrayDeque<Integer> listLineNumber;

    private final static ConfigData config = ConfigData.getInstance();
    private final static ConfigGenerator options = ConfigGenerator.getInstance();

    public static void processFile(String fileName) throws IOException, NullPointerException {

        try {
            listLineNumber = new ArrayDeque<Integer>();
            tokenScanner = new TokenScanner(fileName);
            token = null;

            /* first use a Scanner to get each word */
            while (tokenScanner.hasToken()) {
                token = tokenScanner.getToken();

                if (token.equalsIgnoreCase("globalerror")) {
                    config.addUserGlobalError(getName(), getErrorDescription());
                } else if (token.equalsIgnoreCase("group")) {
                    /*
                     * syntax for parsing group: group setting or state <name>
                     * [instances] <description> [help description]
                     */

                    /* parse setting or state */
                    String groupType = tokenScanner.getToken();

                    /* parse name */
                    String nameStr = getName();

                    // TODO: Seems like something we should check when we add it to the list. -ASK
                    /* make sure group name doesn't exist in group */
                    Group.Type type = Group.Type.toType(groupType);
                    
                    if (config.countainsGroupName(type, nameStr)) {
                        throw new Exception("Duplicate <group> name: " + nameStr);
                    }

                    groupLineNumber = tokenScanner.getLineNumber();

                    /* parse instances */
                    Integer groupInstances;
                    if (tokenScanner.hasTokenInt()) {
                        groupInstances = tokenScanner.getTokenInt();
                    } else if (tokenScanner.hasToken("\\(.*")){
                        groupInstances = getMathExpression();
                    } else {
                    	groupInstances = null;
                    }

                    Group theGroup = new Group(nameStr, getDescription(), getHelpDescription());
                    config.nameLengthSeen(UseNames.COLLECTIONS, nameStr.length());
                    
                    if (groupInstances != null) {
                    	theGroup.setInstances(groupInstances);
                    }

                    String group_access = (type == Group.Type.SETTING) ? "read_write" : "read_only"; 
                    while (tokenScanner.hasToken()) {
                        token = tokenScanner.getToken();

                        if (token.equalsIgnoreCase("capacity")) {
                        	token = tokenScanner.getToken();
                        	if (token.equalsIgnoreCase("fixed")) {
                        		theGroup.setCapacity(ItemList.Capacity.FIXED);
                        	} else if (token.equalsIgnoreCase("variable")) {
                        		theGroup.setCapacity(ItemList.Capacity.VARIABLE);
                        	} else {
                                throw new Exception("Error in <group>: Invalid capacity type of " + token );
                            }
                        } else if (token.equalsIgnoreCase("keys")) {
                        	token = tokenScanner.getToken();
                        	if (token.equals("{}")) {
                        		theGroup.setKeys(new LinkedHashSet<String>());
                        	} else {
                                throw new Exception("Error in <group>: Key parser is unimplemented");
                        	}
                        } else if (token.equalsIgnoreCase("element")) {
                        	theGroup.addItem(processElement(group_access, config));
                        } else if (token.equalsIgnoreCase("list")) {
                        	theGroup.addItem(processList(group_access, config, 0));
                        } else if (token.equalsIgnoreCase("error")) {
                            theGroup.addError(getName(), getErrorDescription());
                        } else if (token.startsWith("#")) {
                            tokenScanner.skipCommentLine();
                        } else {
                        	tokenScanner.pushbackToken(token);
                            break;
                        }
                    }

                    try {
                        theGroup.validate(config);
                    } catch(Exception e) {
                        throw new Exception("Error in <group>: " + theGroup.getName() + "\n\t" + e.getMessage());
                    }

                    config.addConfigGroup(type, theGroup);
                } else if (token.startsWith("#")) {
                    tokenScanner.skipCommentLine();
                } else {
                    throw new Exception("Unrecognized keyword: " + token);
                }
            }
            
            config.validate();
        } catch (NullPointerException e) {
            options.log("Parser NullPointerException");
            options.log(e.toString());
            throw new NullPointerException();
        } catch (Exception e) {
            throw new IOException(errorFoundLog(fileName, e.getMessage()));
        }
        finally {
            tokenScanner.close();
        }
    }

    private static String errorFoundLog(String fileName, String str) {
        int lineNumber;

        if (str.indexOf("<group>") != -1)
            lineNumber = groupLineNumber;
        else if (str.indexOf("<element>") != -1)
            lineNumber = elementLineNumber;
        else if ((str.indexOf("<list>") != -1) && !listLineNumber.isEmpty())
            lineNumber = listLineNumber.pop();
        else
            lineNumber = tokenScanner.getLineNumber();

        return "Error found in " + fileName + ", line " + lineNumber + ": " + str;
    }

    private static String getName() throws Exception {
        String name = tokenScanner.getToken();

        if (name == null) {
            throw new Exception("Missing name!");
        }
        
        if (name.length() > config.getMaxNameLength()) {
            throw new Exception("The name > the maximum length limited " + config.getMaxNameLength());
        }
        
        /* Only allow alphanumeric, hyphen, and underscore */
        /* See https://www.w3.org/TR/xml/#NT-Name */
        if (!name.matches("[:A-Z_a-z][:A-Z_a-z0-9.-]*")) {
            throw new Exception("Invalid character in name: " + name);
        }
    
        return name;
    }

    private static String getValueName(Element.Type type) throws Exception {
    	String name;
    	
    	switch (type) {
    	case ENUM:
            name = tokenScanner.getToken();
            if (name == null) {
                throw new Exception("Missing name!");
            }
            
            if (name.length() > config.getMaxNameLength()) {
                throw new Exception("The name is larger than the maximum length of " + config.getMaxNameLength());
            }
            
            // Relaxed slightly for values in that we allow digits in the first position (but we really shouldn't -ASK)
            if (!name.matches("[:A-Z_a-z0-9][:A-Z_a-z0-9.-]*")) {
                throw new Exception("Invalid character in value name: " + name);
            }
            break;
    	case REF_ENUM:
    		name = getString();
            if (name == null) {
                throw new Exception("Missing name!");
            }
            break;
        default:
        	throw new Exception("unexpected value");
    	}
    
        return name;
    }

    private static String getRefName() throws Exception {
        String name = getString();

        if (name == null) {
            throw new Exception("Missing name!");
        }
        
        if (!name.startsWith("/")) {
            throw new Exception("ref_enum path must start at root: " + name);
        }
        
        if (name.endsWith("/")) {
            throw new Exception("ref_enum path must not end with '/': " + name);
        }
        
        for (String part: name.substring(1).split("/", -1)) {
	        if (!part.matches("[:A-Z_a-z0-9][:A-Z_a-z0-9.-]*")) {
	            throw new Exception("Invalid character in ref_enum path: '" + part + "'");
	        }
        }
        
        return name;
    }

    private static int getMathExpression() throws Exception {

        String ex = null;
        int result = 0;

        if (tokenScanner.hasToken("\\(.*")) {
            ex = tokenScanner.getToken();
            int count = ex.replace(")", "").length() - ex.replace("(", "").length();

            /*read Tokens till we have the same number of '(' and ')' in the expression */
            while(count > 0){
                if(tokenScanner.hasToken("\\\".*"))
                    throw new Exception("Invalid Math Expression, missing ')'");
                ex += tokenScanner.getToken();
                count = ex.replace(")", "").length() - ex.replace("(", "").length();
            }

            if(count < 0)
                throw new Exception("Invalid Math Expression, missing '('");

            ScriptEngineManager mgr = new ScriptEngineManager();
            ScriptEngine engine = mgr.getEngineByName("js");
            /*eval returns an Object that is a Double */
            try{
                result = new BigDecimal(engine.eval(ex).toString()).intValue();
            }
            catch (Exception e){
                throw new Exception("Bad Expression " + e);
            }

        }
        return result;
    }

    private static CharsetEncoder latin_1 = StandardCharsets.ISO_8859_1.newEncoder();
    
    private static boolean isLatin_1(String string) {
    	return latin_1.canEncode(string);
    }
    
    private static String parseDescription(final String type) throws Exception {
        String description = null;
        
        if (tokenScanner.hasToken("\\\".*")) {
            description = tokenScanner.getTokenInLine("\\\".*?\\\"");
            if (description == null) {
                throw new Exception("Invalid " + type + " description");
            }

            description = description.substring(1, description.lastIndexOf("\""));

            if (!isLatin_1(description)) {
                throw new Exception("Non-Latin-1 character in " + type + " description");
            }
            
            if (description.length() == 0)
                description = null;

        }
        return description;
    	
    }
    
    private static String getDescription() throws Exception {
        String description = parseDescription("label");
        if (description != null) {
            if (description.length() > MAX_DESCRIPTION_LENGTH) {
                throw new Exception("description > maximum length " + MAX_DESCRIPTION_LENGTH);
            }

            description = description.replace(":", "::");
        }
        return description;
    }

    private static String getHelpDescription() throws Exception {
        return parseDescription("help");
    }

    private static String getErrorDescription() throws Exception {
        return parseDescription("error");
    }

    private static String getType() throws Exception {
        String type = tokenScanner.getToken();

        if (type == null) {
            throw new Exception("Missing type");
        }
        return type;
    }

    private static String getAccess() throws Exception {
        String access = tokenScanner.getToken();

        if (access == null) {
            throw new Exception("Missing access");

        }
        Item.AccessType.toAccessType(access);

        return access;
    }

    private static String getMinMax() throws Exception {
        String mvalue = null;

        if (tokenScanner.hasToken("\\(.*"))
            mvalue = Integer.toString(getMathExpression());
        else
            mvalue = tokenScanner.getToken();

        if (mvalue == null) {
            throw new Exception("Missing min or max value");
        }

        return mvalue;
    }

    private static String getString() throws Exception {
    	String string;
    	
        if (tokenScanner.hasToken("\\\".*")) {
            string  = tokenScanner.getTokenInLine("\\\".*?\\\"");
            if (string != null) {
            	string = string.substring(1, string.lastIndexOf("\""));
            }
        } else {
        	string = tokenScanner.getToken();
        }

        return string;
    }

    private static String getRegexCase() throws Exception {
    	final String value = getString();
    	
        if (value == null) {
            throw new Exception("Missing case value");
        }

        final String result = value.toLowerCase();
        final List<String> valid = Arrays.asList("ignore", "match");

        if (!valid.contains(result)) {
            throw new Exception("Invalid case value: must be 'ignore' or 'match'");
        }
        	
        return result;
    }
    
    private static String getDefault() throws Exception {
    	String def = getString();
    	
        if (def == null) {
            throw new Exception("Missing default");
        }

        return def;
    }

    private static final Element processElement(String default_access, ConfigData config) throws Exception {
        /*
         * syntax for parsing element: element <name> <description> [help
         * description] type <type> [min <min>] [max <max>] [access <access>]
         * [units <unit>]
         */
    	String name = getName();
        elementLineNumber = tokenScanner.getLineNumber();

        Element element = new Element(name, getDescription(), getHelpDescription());
        config.nameLengthSeen(UseNames.ELEMENTS, name.length());

        try {
            while (tokenScanner.hasToken()) {
                token = tokenScanner.getToken();

                if (token.equalsIgnoreCase("type")) {
                    Element.Type type = element.setType(getType());
                    config.addTypeSeen(type);
                } else if (token.equalsIgnoreCase("access")) {
                    element.setAccess(getAccess());
                } else if (token.equalsIgnoreCase("min")) {
                    element.setMin(getMinMax());
                } else if (token.equalsIgnoreCase("max")) {
                    element.setMax(getMinMax());
                } else if (token.equalsIgnoreCase("default")) {
                    element.setDefault(getDefault());
                } else if (token.equalsIgnoreCase("units")) {
                    element.setUnit(getString());
                } else if (token.equalsIgnoreCase("pattern")) {
                    element.setRegexPattern(getString());
                } else if (token.equalsIgnoreCase("case")) {
                    element.setRegexCase(getRegexCase());
                } else if (token.equalsIgnoreCase("syntax")) {
                    element.setRegexSyntax(getString());
                } else if (token.equalsIgnoreCase("value")) {
                    element.addValue(config, getValueName(element.getType()), getDescription(), getHelpDescription());
                } else if (token.equalsIgnoreCase("ref")) {
                    element.addRef(config, getRefName(), getDescription(), getHelpDescription());
                } else if (token.startsWith("#")) {
                    tokenScanner.skipCommentLine();
                } else {
                    tokenScanner.pushbackToken(token);
                    break;
                }
            }
            
            if (element.getAccess() == null) {
            	element.setAccess(default_access);
            }
            
        } catch (IOException e) {
            throw new IOException(e.toString());
        }

        try {
            element.validate();
            
            if (element.getType() == Element.Type.REF_ENUM) {
            	config.addRefEnum(element);
            }
        } catch (Exception e) {
            throw new Exception("Error in <element>: " + element.getName() + "\n\t" + e.getMessage());
        }
        
        return element;
    }

    private static final ItemList processList(String default_access, ConfigData config, int depth) throws Exception {
        /*
         * syntax for parsing list: list <name> [instances] <description> [help
         * description] [access <access>]
         */

		String name = getName();
		listLineNumber.push(tokenScanner.getLineNumber());
		
        Integer instances;
        if (tokenScanner.hasTokenInt()) {
        	instances = tokenScanner.getTokenInt();
        } else if (tokenScanner.hasToken("\\(.*")){
        	instances = getMathExpression();
        } else {
        	instances = null;
        }

		depth += 1;
		config.listDepth(depth);

		ItemList list = new ItemList(name, getDescription(), getHelpDescription());
        config.nameLengthSeen(UseNames.COLLECTIONS, name.length());

        if (instances != null) {
        	list.setInstances(instances);
        }

		try {
			String list_access = default_access;
			
		    while (tokenScanner.hasToken()) {
		        token = tokenScanner.getToken();
		
                if (token.equalsIgnoreCase("capacity")) {
                	token = tokenScanner.getToken();
                	if (token.equalsIgnoreCase("fixed")) {
                		list.setCapacity(ItemList.Capacity.FIXED);
                	} else if (token.equalsIgnoreCase("variable")) {
                		list.setCapacity(ItemList.Capacity.VARIABLE);
                	} else {
                        throw new Exception("Error in <group>: Invalid capacity type of " + token );
                    }
                } else if (token.equalsIgnoreCase("keys")) {
                	token = tokenScanner.getToken();
                	if (token.equals("{}")) {
                		list.setKeys(new LinkedHashSet<String>());
                	} else {
                        throw new Exception("Error in <group>: Key parser is unimplemented");
                	}
                } else if (token.equalsIgnoreCase("access")) {
		        	list_access = getAccess();
		            list.setAccess(list_access);
		        } else if (token.equalsIgnoreCase("element")) {
		            list.addItem(processElement(list_access, config));
		        } else if (token.equalsIgnoreCase("list")) {
		            list.addItem(processList(list_access, config, depth));
		        } else if (token.equalsIgnoreCase("end")) {
		            break;
		        } else if (token.startsWith("#")) {
		            tokenScanner.skipCommentLine();
		        } else {
		            tokenScanner.pushbackToken(token);
		            break;
		        }
		    }
		    
		    if (list.getAccess() == null) {
		    	list.setAccess(list_access);
		    }
		    
		} catch (IOException e) {
		    throw new IOException(e.toString());
		}

        try {
            list.validate(config);
        } catch(Exception e) {
            throw new Exception("Error in <list>: " + list.getName() + "\n\t" + e.getMessage());
        }

        listLineNumber.pop();
        config.addTypeSeen(Element.Type.LIST);

        return list;
    }
}
