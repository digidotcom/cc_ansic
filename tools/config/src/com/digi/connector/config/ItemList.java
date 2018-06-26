package com.digi.connector.config;

import java.io.IOException;
import java.util.LinkedList;

import javax.naming.NamingException;

public class ItemList extends Item {

    private final int instances;
    private final LinkedList<Item> items;

    public ItemList(String name, int count, String description,
            String helpDescription) throws Exception {
        super(name, description, helpDescription);
        if (count <= 0) {
            throw new IOException("Invalid instance count for: " + name);
        }

        instances = count;
        items = new LinkedList<Item>();
    }

    // TODO: Seems like an odd place for this to live, given that the class
    // has no other knowledge of XML. -ASK
    public String toString(int id) {
        return String.format(
            "<element name=\"%s\" desc=\"%s\" type=\"list\" bin_id=\"%d\">",
             name, toRciDescription(), id);
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

    public int getInstances() {
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

    public void validate() throws Exception{
        if (items.isEmpty()) {
            throw new Exception("No items specified");
        }
    }
}
