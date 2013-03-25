package com.digi.ic.config;


public class RciGlobalErrors extends RciStrings {
    /* list global errors s "name" "description" */
    private final static String[] rciGlobalErrorStrings = { "bad_command", "Bad command",
                                                            "invalid_version", "Invalid version",
                                                            "bad_descriptor", "Bad configuration"};
    
    protected RciGlobalErrors() {
        super(rciGlobalErrorStrings);

        if (ConfigGenerator.getBinaryOption()) {
            rciStrings.remove("invalid_version");
            rciStrings.remove("Invalid version");
        } else {
            rciStrings.remove("bad_descriptor");
            rciStrings.remove("Bad configuration");
            
        }
    }

}
