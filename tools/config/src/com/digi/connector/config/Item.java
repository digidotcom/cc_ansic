package com.digi.connector.config;

import java.io.IOException;

public abstract class Item {

    protected final String name;
    protected final String description;
    protected final String helpDescription;
    protected AccessType access; // TODO: store as AccessType instead of String

    public enum AccessType {
        READ_ONLY, WRITE_ONLY, READ_WRITE;

        public static AccessType toAccessType(String str) throws Exception {
            try {
                return valueOf(str.toUpperCase());

            } catch (Exception e) {
                throw new Exception("Invalid access Type: " + str);
            }
        }
    }

    public Item(String name, String description, String helpDescription) throws IOException {

        if (description == null) {
            throw new IOException("Missing or bad description");
        }

        this.name = name;
        this.description = description.replaceAll(":", "::");
        this.helpDescription = helpDescription;
    }

    public String toRciDescription() {
        return (helpDescription == null)
            ? description
            : description + ":" + helpDescription;
    }

    public String getName() {
        return name;
    }

    public String getSanitizedName() {
    	return name.replace('-', '_').replace(".","_fullstop_");
    }

    public String getDescription() {
        return description;
    }

    public String getHelpDescription() {
        return helpDescription;
    }

    public AccessType getAccess() {
        return access;
    }

    public void setAccess(String access) throws Exception {
        if (this.access == null)
            this.access = AccessType.toAccessType(access);
        else
            throw new Exception("Duplicate <access> keyword: " + access);
    }

    abstract String toString(int id);
}
