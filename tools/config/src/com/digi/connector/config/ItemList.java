package com.digi.connector.config;

import java.util.LinkedList;
import java.util.LinkedHashSet;

import javax.naming.NamingException;

public class ItemList extends Item {
    public enum Capacity { FIXED, VARIABLE };

    private Integer instances = null;
    private LinkedHashSet<String> keys = null;
    private Capacity capacity = null;
    private final LinkedList<Item> items = new LinkedList<>();
    
    public ItemList(String name, String description, String helpDescription) throws Exception {
        super(name, description, helpDescription);
    }

    protected final String collectionXmlAttributes(String header) {
    	String result = header;

        if (instances == null) {
        	int MAX_KEY_LENGTH = 64; // TODO: This should be configurable. -ASK
        	result += String.format("<attr bin_id=`0` name=`name` desc=`key name` type=`string` max=`%d` />", MAX_KEY_LENGTH);
        	if (capacity == ItemList.Capacity.VARIABLE) {
        		result += "<attr bin_id=`1` name=`complete` desc=`all keys given` type=`boolean` default=`false` />";
        		result += "<attr bin_id=`2` name=`remove` desc=`remove key` type=`boolean` default=`false` />";
        	}
        } else {
        	result += String.format("<attr bin_id=`0` name=`index` desc=`item number` type=`int32` max=`%d` />", instances);
        	if (capacity == ItemList.Capacity.VARIABLE) {
        		result += String.format("<attr bin_id=`1` name=`count` desc=`current item count` type=`int32` min=`0` max=`%d` />", instances);
        		result += "<attr bin_id=`2` name=`shrink` desc=`shrink existing array` type=`boolean` default=`true` />";
        	}
        }

    	return result;
    }
    // TODO: Seems like an odd place for this to live, given that the class
    // has no other knowledge of XML. -ASK
    public String toString(int id) {
        return collectionXmlAttributes(String.format("<element name=`%s` desc=`%s` type=`list` bin_id=`%d`>", name, toRciDescription(), id));
    }

    public String getName() {
        return name;
    }

    public String getDescription() {
        return description;
    }

    public String getHelpDescription() {
        return helpDescription;
    }

    public void setCapacity(Capacity capacity) {
    	this.capacity = capacity;
    }
    
    public Capacity getCapacity() {
    	return capacity;
    }
    
    public void setKeys(LinkedHashSet<String> keys) {
    	this.keys = keys;
    }

    public LinkedHashSet<String> getKeys() {
    	return keys;
    }

    public void setInstances(int count) throws Exception {
        if (count <= 0) {
            throw new Exception("Invalid instance count for: " + name);
        }
        instances = count;
    }

    public Integer getInstances() {
        return instances;
    }

    public LinkedList<Item> getItems() {
        return items;
    }

    public void addItem(Item newItem) throws NamingException {
        String newName = newItem.getName();

        for (Item existingItem : items) {
            if (existingItem.getName().equals(newName)) {
                throw new NamingException("Duplicate name: " + newName);
            }
        }
        items.add(newItem);
    }

    public void validate() throws Exception {
    	if (instances == null) { // no count specified
    		if (keys == null) { // and no keys specified
    			instances = 1;
    			assert capacity == null;
    			capacity = Capacity.FIXED;
    		} else {
    			if (capacity == null) {
    				capacity = keys.isEmpty() ? Capacity.VARIABLE : Capacity.FIXED; 
    			}
    		}
    	} else {
    		if (keys != null) {
                throw new Exception("Count and keys both specified");
    		}

    		if (capacity == null) {
        		capacity = Capacity.FIXED; 
        	}
    	}
    	
        if (items.isEmpty()) {
            throw new Exception("No items specified");
        }
    }
}
