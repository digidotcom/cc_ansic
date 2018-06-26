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
import java.util.List;
import java.util.Arrays;

public abstract class GenFile {
    private static List<String> COPYRIGHT = Arrays.asList(
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

    private Type type;
    private BufferedWriter writer;

    protected Path path;
    protected final GenSource code = GenSource.getInstance();
    protected final ConfigData config = ConfigData.getInstance();
    protected final ConfigGenerator options = ConfigGenerator.getInstance();
    protected final String customPrefix = options.getCustomPrefix();
    
    public enum Type { INTERNAL, USER };
    public enum UsePrefix { CUSTOM, NONE };

    abstract protected void writeContent() throws Exception;

    public GenFile(String file, Type type, UsePrefix use) throws IOException {
    	if (use == UsePrefix.CUSTOM) {
    		file = customPrefix + file;
    	}
        this.path = Paths.get(options.getDir(), file);
        this.type = type;

        File existing = path.toFile();
        if (!options.noBackupOption() && existing.exists())
        	backupExisting(existing);
    }

    private void backupExisting(File existing) throws IOException {
        File backup;

        int i = 0;
        do {
        	backup = new File(existing.getPath() + "_bkp_" + i++);
        } while (!backup.isFile());
        
        Files.copy(existing.toPath(), backup.toPath());
        options.log("Existing file " + existing + " saved as: " + backup);
	}

    // TODO Eventually this goes away. -ASK
    public final void write(String content) throws IOException {
    	writer.write(content);
    }

    public final void writeLine(String line) throws IOException {
    	writer.write(line);
    	writer.newLine();
    }
    
    public final void writeBlock(Iterable<? extends String> lines) throws IOException {
    	for (String line: lines) {
    		writeLine(line);
    	}
    	writer.newLine();
    }

    private void writePreamble() throws IOException {
        DateFormat dateFormat = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss");
        Date date = new Date();

        writeBlock(Arrays.asList(
        	"/*",
        	" * This is an auto-generated file - DO NOT EDIT!",
        	" * It was generated using: " + ConfigGenerator.class.toString(), 
        	" * This file was generated on: " + dateFormat.format(date),
            " * The command line arguments were: " + options.getArgumentLogString(),
            " * The version was: " + ConfigGenerator.VERSION,
            " */"));

        if (type == Type.INTERNAL) {
            writeBlock(COPYRIGHT);
        }
    }
    
    public final void generateFile() throws Exception {
        writer = new BufferedWriter(new FileWriter(path.toFile()));
        try {
	    	writePreamble();
	    	writeContent();
	    	
	        options.log(String.format("File created: %s\n", path));
        } finally {
        	writer.close();
        }
    }
    
}
