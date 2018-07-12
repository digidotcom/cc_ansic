package com.digi.connector.config;

import java.io.IOException;
import java.util.LinkedHashMap;

public class Group extends ItemList {
	public enum Type {
	    SETTING,
	    STATE;

	    public String toUpperName() {
            return name();
	    }

	    public String toLowerName() {
	        return toUpperName().toLowerCase();
	    }

	    public static Type toType(String str) throws Exception {
	        try {
                return valueOf(str.toUpperCase());
	        } catch (Exception e) {
	            throw new Exception("Invalid group Type: " + str);
	        }
	    }
	}

    private final LinkedHashMap<String, String> errorMap;

    public Group(String name, String description,
            String helpDescription) throws Exception {
        super(name, description, helpDescription);

        errorMap = new LinkedHashMap<String, String>();
    }

    public String toString(int id) {
        return collectionXmlAttributes(String.format("<descriptor element=`%s` desc=`%s` bin_id=`%d`>", name, toRciDescription(), id));
    }

    public LinkedHashMap<String, String> getErrors() {
        return errorMap;
    }

    public void addError(String name, String description) throws IOException {
        if (errorMap.containsKey(name)) {
            throw new IOException("Duplicate <error>: " + name);
        }
        if (description == null) {
            throw new IOException("Missing or bad error description");
        }
        errorMap.put(name, description);
    }
}
