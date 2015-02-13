package com.digi.connector.config;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;

import com.digi.connector.config.ConfigGenerator.FileType;
import com.digi.connector.config.ConfigGenerator.UseNames;

public class FileSource extends FileGenerator {

    private static FileType fileType = FileType.SOURCE;
    private static String SOURCE_NAME = ConfigGenerator.getPrefix() + "rci_config";
    
    private static BufferedWriter headerWriter = null;
    private String headerFile = "";

	public FileSource(String directoryPath) throws IOException {
		
		super(directoryPath,SOURCE_NAME+ ".c",fileType);
        /* Need to create a header file + source file
         * fileWriter is source file writer. 
         * So we need to create local header file.
         */
        headerFile = SOURCE_NAME + ".h";

        if(!ConfigGenerator.noBackupOption())
            super.checkPreviousBuild(filePath + headerFile);

        headerWriter = new BufferedWriter(new FileWriter(filePath + headerFile));
        writeHeaderComment(headerWriter);

	}
	
    public void writeHeaderComment(BufferedWriter bufferWriter) throws IOException {
        DateFormat dateFormat = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss");
        Date date = new Date();

        String note_string = "/*\n * This is an auto-generated file - DO NOT EDIT! \n";
        
        if (bufferWriter == headerWriter) 
            note_string += String.format(" * This is a H file generated by %s tool.\n", TOOL_NAME);
        else 
            note_string += String.format(" * This is a C file generated by %s tool.\n", TOOL_NAME);


        note_string += String.format(" * This file was generated on: %s \n", dateFormat.format(date))
                     + String.format(" * The command line arguments were: %s\n", ConfigGenerator.getArgumentLogString())
                     + String.format(" * The version of %s tool was: %s */\n\n", TOOL_NAME, ConfigGenerator.VERSION);
        
        bufferWriter.write(note_string);
        
    }
    
    public void generateFile(ConfigData configData) throws Exception {
        try {

            /* write include header in the header file */
             String headerDefineName = headerFile.replace('.', '_').toLowerCase();
            headerWriter.write(String.format("#ifndef %s\n#define %s\n\n", headerDefineName, headerDefineName));

            fileWriter.write(String.format("%s \"%s\"\n\n", INCLUDE, "connector_api.h"));
            fileWriter.write(String.format("#if !(defined CONST)\n#define CONST const\n#endif\n"));
            
            fileWriter.write(String.format("%s \"%s\"\n\n", INCLUDE, headerFile));

        	writeGlobalErrorEnumHeader(configData,headerWriter);  
        	writeGroupTypeAndErrorEnum(configData,headerWriter);

            /* Write Define Errors Macros */
            writeDefineRciErrors(configData, fileWriter);

            writeDefineGroupErrors(configData, fileWriter);

            writeDefineGlobalErrors(configData, fileWriter);

            /* write remote all strings in source file */
            writeRemoteAllStrings(configData, fileWriter);

            /* write connector_rci_errors[] */
            writeGlobalErrorStructures(configData, fileWriter);

            /* write structures in source file */
            writeAllStructures(configData, fileWriter);

            int GlobalErrorCount = configData.getUserGlobalErrors().size() + configData.rciGlobalErrors.size();

            fileWriter.write(String.format("\nconnector_remote_config_data_t const %srci_internal_data = {\n" +
                    "    connector_group_table,\n"+
                    "    connector_rci_errors,\n"+
                    "    %d,\n"+
                    "    %d,\n"+
                    "    %s,\n"+
                    "    \"%s\"\n"+
                    "};\n"+
                    "\n"+
                    "connector_remote_config_data_t const * const %srci_descriptor_data = &%srci_internal_data;"
                    , prefix, GlobalErrorCount, ConfigGenerator.getFirmware(), Descriptors.vendorId(),Descriptors.deviceType(), prefix, prefix));
            
            headerWriter.write(String.format("\n#endif\n"));
 

            ConfigGenerator.log(String.format("Files created:\n\t%s%s",  filePath, generatedFile));
            if (generatedFile.length() > 0) {
                ConfigGenerator.log(String.format("\t%s%s", filePath, headerFile));
            }
            
            if (ConfigGenerator.useNamesOption() != UseNames.NONE) {
                generateUseNamesFile(configData);    
            }

        } catch (IOException e) {
            throw new IOException(e.getMessage());
        }

        finally {
            fileWriter.close();
            if (headerWriter != null) headerWriter.close();
        }

    }
    
    public static BufferedWriter getHeaderWriter(){
    	return headerWriter;
    }
    
}
