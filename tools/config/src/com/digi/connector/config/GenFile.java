package com.digi.connector.config;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Paths;
import java.nio.file.Path;
import java.nio.file.Files;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;

public abstract class GenFile {
    private static String COPYRIGHT = String.join("\n",
		"/*",
        " * Copyright (c) 2018 Digi International Inc.", 
        " *",
        " * This Source Code Form is subject to the terms of the Mozilla Public",
        " * License, v. 2.0. If a copy of the MPL was not distributed with this file,",
        " * You can obtain one at http://mozilla.org/MPL/2.0/.",
        " *",
        " * THE SOFTWARE IS PROVIDED \"AS IS\" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH",
        " * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY",
        " * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,",
        " * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM",
        " * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR",
        " * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR",
        " * PERFORMANCE OF THIS SOFTWARE.",
        " *",
        " * Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343",
        " * =======================================================================",
        " */"
        );

    public enum Type { INTERNAL, USER };
    protected Path path;
    private Type type;
    protected BufferedWriter fileWriter;

    abstract protected void writeContent(ConfigData configData) throws Exception;

    public GenFile(String dir, String file, Type type) throws IOException {
        this.path = Paths.get(dir, file);
        this.type = type;

        File existing = path.toFile();
        if (!ConfigGenerator.noBackupOption() && existing.exists())
        	backupExisting(existing);
    }

    private void backupExisting(File existing) throws IOException {
        File backup;

        int i = 0;
        do {
        	backup = new File(existing.getPath() + "_bkp_" + i++);
        } while (!backup.isFile());
        
        Files.copy(existing.toPath(), backup.toPath());
        ConfigGenerator.log("Existing file " + existing + " saved as: " + backup);
	}

    private void writePreamble() throws IOException {
        DateFormat dateFormat = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss");
        Date date = new Date();

        fileWriter.write(
        	"/*\n" +
        	" * This is an auto-generated file - DO NOT EDIT! \n" +
        	" * It was generated using " + ConfigGenerator.class.toString() + ".\n" + 
        	" * This file was generated on " + dateFormat.format(date) + ".\n" +
            " * The command line arguments were: " + ConfigGenerator.getArgumentLogString() + ".\n" +
            " * The version was: " + ConfigGenerator.VERSION + ".\n" +
            " */\n");

        if (type == Type.INTERNAL) {
            fileWriter.write(COPYRIGHT);
        }
    }
    
    public final void generateFile(ConfigData configData) throws Exception {

        fileWriter = new BufferedWriter(new FileWriter(path.toFile()));

        try {
	    	writePreamble();
	    	writeContent(configData);
	    	
	        ConfigGenerator.log(String.format("File created: %s\n", path));
        } finally {
        	fileWriter.close();
        }
    }
    
}
