package com.digi.connector.config;

import java.io.IOException;
import java.util.LinkedHashMap;

public class Group extends ItemList {

    private final LinkedHashMap<String, String> errorMap;

    public Group(String name, int count, String description,
            String helpDescription) throws Exception {
        super(name, count, description, helpDescription);

        errorMap = new LinkedHashMap<String, String>();
    }

    public String toString(int id) {
        return String.format(
            "<descriptor element=\"%s\" desc=\"%s\" bin_id=\"%d\">",
             name, toRciDescription(), id);
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
