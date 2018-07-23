package com.digi.connector.config;

import java.io.IOException;
import java.util.EnumMap;
import java.util.EnumSet;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.Map;
import java.util.AbstractMap.SimpleImmutableEntry;

import com.digi.connector.config.ConfigGenerator.UseNames;
import com.digi.connector.config.Element;

public class ConfigData {
	private static ConfigData instance = null;
    private ConfigData() {
        groupList = new LinkedHashMap<Group.Type, LinkedList<Group>>(2);
        groupList.put(Group.Type.SETTING, new LinkedList<Group>());
        groupList.put(Group.Type.STATE, new LinkedList<Group>());

        globalFatalProtocolErrorsOffset = 1;
        globalFatalProtocolErrors = new LinkedHashMap<>();
        globalFatalProtocolErrors.put("bad_command", "Bad command");
        globalFatalProtocolErrors.put("bad_descriptor", "Bad configuration");
        globalFatalProtocolErrors.put("bad_value", "Bad value");
            
        globalProtocolErrorsOffset = globalFatalProtocolErrorsOffset + globalFatalProtocolErrors.size();
        globalProtocolErrors = new LinkedHashMap<>();
        globalProtocolErrors.put("bad_value", "Bad value");
        globalProtocolErrors.put("invalid_index", "Invalid index");
		globalProtocolErrors.put("invalid_name", "Invalid name");
		globalProtocolErrors.put("missing_name", "Missing name");
        
        globalUserErrorsOffset = globalProtocolErrorsOffset + globalProtocolErrors.size();
        globalUserErrors = new LinkedHashMap<>();
        
    	for (UseNames name: UseNames.values()) {
    		max_name_length.put(name, 0);
    	}
    }
	public static final ConfigData getInstance() { if (instance == null) instance = new ConfigData(); return instance; }

    /* user setting and state groups */
    private LinkedHashMap<Group.Type, LinkedList<Group>> groupList;

    private int globalFatalProtocolErrorsOffset;
    private final Map<String, String> globalFatalProtocolErrors;
    
    private int globalProtocolErrorsOffset;
    private Map<String, String> globalProtocolErrors;

    private int globalUserErrorsOffset;
    private Map<String, String> globalUserErrors;

    private int CommandsAttributeMaxLen = 20;
    private int max_list_depth = 0;
    private int max_key_length;
    
    private EnumMap<UseNames, Integer> max_name_length = new EnumMap<>(UseNames.class);
    private EnumSet<Element.Type> typesSeen = EnumSet.noneOf(Element.Type.class);

    public LinkedList<Group> getConfigGroup(Group.Type type) throws Exception {
        return groupList.get(type);
    }

    public LinkedList<SimpleImmutableEntry<Group.Type, Group>> getConfigGroupEntries() {
    	LinkedList<SimpleImmutableEntry<Group.Type, Group>> result = new LinkedList<>();
    	
        for (Map.Entry<Group.Type, LinkedList<Group>> pair : groupList.entrySet()) {
        	Group.Type type = pair.getKey();
        	LinkedList<Group> groups = pair.getValue();

            for (Group group : groups) {
                result.add(new SimpleImmutableEntry<Group.Type, Group>(type, group));
            }
        }
        return result;
    }

    public boolean isProtocolGlobalError(String name) {
    	return (globalFatalProtocolErrors.containsKey(name) || globalProtocolErrors.containsKey(name)); 
    }
    
    public boolean isUserGlobalError(String name) {
    	return globalUserErrors.containsKey(name);
    }
    
    public void addUserGlobalError(String name, String description) throws Exception {

        if (description == null) {
            throw new IOException("Missing or bad globalerror description");
        }
         
        if (isProtocolGlobalError(name)) {
            throw new Exception("Existing protocol error <globalerror>: " + name);
        }

        if (isUserGlobalError(name)) {
            throw new Exception("Duplicate <globalerror>: " + name);
        }

        globalUserErrors.put(name, description);
    }

    public int getGlobalFatalProtocolErrorsOffset() {
    	return globalFatalProtocolErrorsOffset;
    }
    
    public Map<String, String> getGlobalFatalProtocolErrors() {
    	return globalFatalProtocolErrors;
    }
    
    public int getGlobalProtocolErrorsOffset() {
    	return globalProtocolErrorsOffset;
    }
    
    public Map<String, String> getGlobalProtocolErrors() {
    	return globalProtocolErrors;
    }
    
    public int getGlobalUserErrorsOffset() {
    	return globalUserErrorsOffset;
    }
    
    public Map<String, String> getGlobalUserErrors() {
    	return globalUserErrors;
    }
    
    public int getGroupErrorsOffset() {
    	return globalUserErrorsOffset + globalUserErrors.size();
    }
    
    public void setAttributeMaxLen(int len) throws Exception {
        if (len > 0)
            CommandsAttributeMaxLen = len;
        else
        	throw new Exception("Invalid CommandsAttributeMaxLen");
    }

	public int AttributeMaxLen() {

		return CommandsAttributeMaxLen;
	}

	public void listDepth(int depth) {
		if (depth > max_list_depth) {
			max_list_depth = depth;
		}
	}
	
	public int getMaxDepth() {
		return max_list_depth;
	}
	
	public void addTypeSeen(Element.Type type) {
		typesSeen.add(type);
	}
	
	public EnumSet<Element.Type> getTypesSeen() {
		return typesSeen;
	}
	
    public void nameLength(UseNames type, int length) {
    	int current = max_name_length.get(type);
    	max_name_length.put(type, Math.max(current, length));
    }
    
    public int getMaxNameLength(UseNames type) {
    	return max_name_length.get(type);
    }

    public int setMaxKeyLength(int length) {
    	return max_key_length = length;
    }

    public int getMaxKeyLength() {
    	return max_key_length;
    }
}
