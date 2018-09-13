package com.digi.connector.config;

import java.io.IOException;

public class Value extends Item {

    public Value(String name, String description, String helpDescription) throws IOException {
        super(name, description == null ? "" : description, helpDescription);
    }

    public String toString(Integer id) {
    	if (id == null) {
    		return String.format("<value value=\"%s\" desc=\"%s\"/>", name, Descriptors.encodeEntities(getDescription()));
    	} else {
    		return String.format("<value value=\"%s\" desc=\"%s\" bin_id=\"%d\"/>", name, Descriptors.encodeEntities(getDescription()), id);
    	}
    }
}
