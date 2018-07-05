package com.digi.connector.config;

import java.io.IOException;

import com.digi.connector.config.ConfigGenerator.UseNames;

public class GenUserNamesHeaderFile extends GenHeaderFile {

    private static String FILE = "rci_usenames_defines.h";

	public GenUserNamesHeaderFile() throws IOException {
		super(FILE, GenFile.Type.USER, GenFile.UsePrefix.CUSTOM);
	}

    private void generateUseNamesHeader(String type, int amount) throws IOException {
        write(String.format(
            "#if !(defined RCI_%s_NAME_MAX_SIZE)\n" +
            "#define RCI_%s_NAME_MAX_SIZE %d\n" +
            "#else\n" +
            "#if RCI_%s_NAME_MAX_SIZE < %d\n" +
            "#undef RCI_%s_NAME_MAX_SIZE\n" +
            "#define RCI_%s_NAME_MAX_SIZE %d\n" +
            "#endif\n" +
            "#endif\n", type, type, amount, type, amount, type, type, amount));
    }

    public void writeGuardedContent() throws Exception {
        if (options.useNames().contains(UseNames.ELEMENTS)) {
        	generateUseNamesHeader("ELEMENTS", config.getMaxNameLength(UseNames.ELEMENTS) + 1);
        }
        
        if (options.useNames().contains(UseNames.COLLECTIONS)) {
        	generateUseNamesHeader("COLLECTIONS", config.getMaxNameLength(UseNames.COLLECTIONS) + 1);
        }
        
        if (options.useNames().contains(UseNames.VALUES)) {
        	generateUseNamesHeader("VALUES", config.getMaxNameLength(UseNames.VALUES) + 1);
        }
    }
}
