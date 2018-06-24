package com.digi.connector.config;

import java.io.BufferedWriter;
import java.io.IOException;

import com.digi.connector.config.ConfigGenerator.UseNames;

public class GenUserNamesHeaderFile extends GenHeaderFile {

    private static String FILE = ConfigGenerator.getCustomPrefix() + "rci_usenames_defines.h";

	public GenUserNamesHeaderFile(String dir) throws IOException {
		super(dir, FILE, GenFile.Type.USER);
	}

    private void generateUseNamesHeader(BufferedWriter writer, String type, int amount) throws IOException {
        writer.write(String.format(
            "#if !(defined RCI_%s_NAME_MAX_SIZE)\n" +
            "#define RCI_%s_NAME_MAX_SIZE %d\n" +
            "#else\n" +
            "#if RCI_%s_NAME_MAX_SIZE < %d\n" +
            "#undef RCI_%s_NAME_MAX_SIZE\n" +
            "#define RCI_%s_NAME_MAX_SIZE %d\n" +
            "#endif\n" +
            "#endif\n", type, type, amount, type, amount, type, type, amount));
    }

    public void generateFile(ConfigData configData) throws Exception {
        try {
            writePreamble();
	
	        /* write include header in the header file */
	        String usenamesHeaderDefineName = FILE.replace('.', '_').toLowerCase();
	        fileWriter.write(String.format("#ifndef %s\n#define %s\n\n", usenamesHeaderDefineName, usenamesHeaderDefineName));
	
	        if (ConfigGenerator.useNamesOption(UseNames.ELEMENTS)) {
	        	generateUseNamesHeader(fileWriter, "ELEMENTS", configData.getMaxNameLength(UseNames.ELEMENTS) + 1);
	        }
	        
	        if (ConfigGenerator.useNamesOption(UseNames.COLLECTIONS)) {
	        	generateUseNamesHeader(fileWriter, "COLLECTIONS", configData.getMaxNameLength(UseNames.COLLECTIONS) + 1);
	        }
	
	        fileWriter.write(String.format("\n#endif\n"));
	
	        ConfigGenerator.log(String.format("Files created:\n\t%s",  path));
	        ConfigGenerator.log(String.format("NOTE: include \"%s%s\" in your custom_connector_config.h so connector_api_remote.h can find the defines\n",  path));
	    } catch (IOException e) {
	        throw new IOException(e.getMessage());
	    } finally {
	        if (fileWriter != null){
	        	fileWriter.close();
	        }
	    }
    }
}
