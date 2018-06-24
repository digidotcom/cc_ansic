package com.digi.connector.config;

import java.io.IOException;

import com.digi.connector.config.ConfigGenerator.UseNames;

public class GenUserNamesHeaderFile extends GenHeaderFile {

    private static String FILE = ConfigGenerator.getCustomPrefix() + "rci_usenames_defines.h";

	public GenUserNamesHeaderFile(String dir) throws IOException {
		super(dir, FILE, GenFile.Type.USER);
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

    public void writeGuardedContent(ConfigData configData) throws Exception {
        if (ConfigGenerator.useNamesOption(UseNames.ELEMENTS)) {
        	generateUseNamesHeader("ELEMENTS", configData.getMaxNameLength(UseNames.ELEMENTS) + 1);
        }
        
        if (ConfigGenerator.useNamesOption(UseNames.COLLECTIONS)) {
        	generateUseNamesHeader("COLLECTIONS", configData.getMaxNameLength(UseNames.COLLECTIONS) + 1);
        }
    }
}
