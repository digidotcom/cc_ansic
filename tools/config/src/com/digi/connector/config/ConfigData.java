package com.digi.connector.config;


import java.io.IOException;
import java.util.ArrayList;
import java.util.EnumMap;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.Map;

import com.digi.connector.config.ConfigGenerator.UseNames;

public class ConfigData {

    /* user setting and state groups */
    private ArrayList<LinkedList<Group>> groupList;

    private RciStrings userGlobalErrors = new RciStrings();

    /* user global error */
    private Map<Object, Integer> rciErrorMap = new HashMap<Object, Integer>();

    private final static String[] rciGlobalErrorStrings = { "bad_command", "Bad command",
        "bad_descriptor", "Bad configuration", "bad_value", "Bad value"};

    RciStrings rciGlobalErrors = new RciStrings(rciGlobalErrorStrings);

    private static int CommandsAttributeMaxLen = 20;
    private static int max_list_depth = 0;

    private static EnumMap<UseNames, Integer> max_name_length = new EnumMap<UseNames, Integer>(UseNames.class);


    public ConfigData() throws Exception {
        groupList = new ArrayList<LinkedList<Group>>();
        groupList.add(GroupType.toGroupType("setting").getIndex(), new LinkedList<Group>());
        groupList.add(GroupType.toGroupType("state").getIndex(), new LinkedList<Group>());

        rciErrorMap.put(rciGlobalErrors, 1);
        rciErrorMap.put(userGlobalErrors, rciGlobalErrors.size() + 1);
        
    	for (UseNames name: UseNames.values()) {
    		max_name_length.put(name, 0);
    	}
    }

    public LinkedList<Group> getSettingGroups() throws Exception {
        return getConfigGroup("setting");
    }

    public LinkedList<Group> getStateGroups() throws Exception {
        return getConfigGroup("state");
    }

    public LinkedList<Group> getConfigGroup(String type) throws Exception {
        GroupType groupType = GroupType.toGroupType(type);

        LinkedList<Group> config = groupList.get(groupType.getIndex());

        return config;
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

	public static int AttributeMaxLen() {

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
	
    public void nameLength(UseNames type, int length) {
    	int current = max_name_length.get(type);
    	max_name_length.put(type, Math.max(current, length));
    }
    
    public int getMaxNameLength(UseNames type) {
    	return max_name_length.get(type);
    }
}
