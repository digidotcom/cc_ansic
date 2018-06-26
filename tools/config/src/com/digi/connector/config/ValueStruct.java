package com.digi.connector.config;

import java.io.IOException;

public class ValueStruct extends Item {

    public ValueStruct(String name, String description, String helpDescription) throws IOException {
        super(name, description == null ? "" : description, helpDescription);
    }

    public String toString(int id) {
        return String.format("<value value=\"%s\" desc=\"%s\" bin_id=\"%d\"/>", name, getDescription(), id);
    }
}
