package com.digi.connector.config;

import java.util.Collection;
import java.util.LinkedHashMap;

public class Table {
	private LinkedHashMap<String, Group> groups = new LinkedHashMap<>();
	
    public Table(Group.Type type) {
    }
    
    public int size() {
    	return groups.size();
    }
    
    public Collection<Group> groups() {
    	return groups.values();
    }
    
    public void add(Group group) throws Exception {
    	final String name = group.getName();
    	
        if (groups.containsKey(name)) {
            throw new Exception("Duplicate <group> name: " + name);
        }

    	groups.put(name, group);
    }
}
