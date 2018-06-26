package com.digi.connector.config;

import java.io.IOException;
import java.util.EnumMap;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.Map;

import com.digi.connector.config.ConfigGenerator.UseNames;
import com.digi.connector.config.Element;

public class ConfigData {
	private static ConfigData instance = null;
    private ConfigData() {
        groupList = new LinkedHashMap<Group.Type, LinkedList<Group>>(2);
        groupList.put(Group.Type.SETTING, new LinkedList<Group>());
        groupList.put(Group.Type.STATE, new LinkedList<Group>());

        rciErrorMap.put(rciGlobalErrors, 1);
        rciErrorMap.put(userGlobalErrors, rciGlobalErrors.size() + 1);
        
    	for (UseNames name: UseNames.values()) {
    		max_name_length.put(name, 0);
    	}
    }
	public static final ConfigData getInstance() { if (instance == null) instance = new ConfigData(); return instance; }

    /* user setting and state groups */
    private LinkedHashMap<Group.Type, LinkedList<Group>> groupList;

    private RciStrings userGlobalErrors = new RciStrings();

    /* user global error */
    private Map<Object, Integer> rciErrorMap = new HashMap<>();

    private final String[] rciGlobalErrorStrings = { "bad_command", "Bad command",
        "bad_descriptor", "Bad configuration", "bad_value", "Bad value"};

    RciStrings rciGlobalErrors = new RciStrings(rciGlobalErrorStrings);

    private int CommandsAttributeMaxLen = 20;
    private int max_list_depth = 0;

    private EnumMap<UseNames, Integer> max_name_length = new EnumMap<>(UseNames.class);
    private EnumSet<Element.Type> typesSeen = EnumSet.noneOf(Element.Type.class);

    public LinkedList<Group> getConfigGroup(Group.Type type) throws Exception {
        return groupList.get(type);
    }

    public LinkedHashMap<String, String> getUserGlobalErrors() {
        return userGlobalErrors.getStrings();
    }

    public void addRCIGroupError(String name, String description)
            throws Exception {

        if ((rciGlobalErrors.size() > 0) && (rciGlobalErrors.getStrings().containsKey(name))) {
            throw new Exception("Duplicate RCI_COMMAND");
        }

        rciGlobalErrors.addStrings(name, description);
    }

    public void addUserGroupError(String name, String description)
            throws Exception {

        if ((userGlobalErrors.size() > 0) && (userGlobalErrors.getStrings().containsKey(name))) {
            throw new Exception("Duplicate <globalerror>: " + name);
        }
        
        if (description == null) {
            throw new IOException("Missing or bad globalerror description");
      }

        userGlobalErrors.addStrings(name, description);
    }

    public LinkedHashMap<String, String> getRciGlobalErrors() {
        return rciGlobalErrors.getStrings();
    }


    public int getRciGlobalErrorsIndex() {
        return rciErrorMap.get(rciGlobalErrors);
    }

    public int getUserGlobalErrorsIndex() {
        return rciErrorMap.get(userGlobalErrors);
    }

    public Map<Object, Integer> getRciErrorMap() {
        return rciErrorMap;
    }

    public int getAllErrorsSize() {
        int size = rciGlobalErrors.size()
                + userGlobalErrors.size();

        return size;
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
}
