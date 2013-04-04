package com.digi.connector.config;


public class RciGlobalErrors extends RciStrings {
    /* list global errors s "name" "description" */
    private final static String[] rciGlobalErrorStrings = { "bad_command", "Bad command",
                                                            "bad_descriptor", "Bad configuration"};
    
    protected RciGlobalErrors() {
        super(rciGlobalErrorStrings);
    }

}
