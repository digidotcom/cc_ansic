package com.digi.connector.config;

import java.io.IOException;

public class Reference extends Item {

    public Reference(String name, String description, String helpDescription) throws IOException {
        super(name, description == null ? "" : description, helpDescription);
    }

    public String toString(Integer id) {
    	assert id == null;
        return String.format("<ref name=\"%s\" desc=\"%s\"/>", name, Descriptors.encodeEntities(getDescription()));
    }
}