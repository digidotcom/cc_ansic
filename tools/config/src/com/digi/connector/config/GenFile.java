package com.digi.connector.config;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.Paths;
import java.nio.file.Path;
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

    abstract public void generateFile(ConfigData configData) throws Exception;

    public GenFile(String dir, String file, Type type) throws IOException {
        this.path = Paths.get(dir, file);
        this.type = type;

        if (!ConfigGenerator.noBackupOption())
            checkPreviousBuild(path.toString());

        fileWriter = new BufferedWriter(new FileWriter(path.toString()));
    }

    public void writePreamble() throws IOException {
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

    protected void checkPreviousBuild(String new_path) throws IOException {

        File new_file = new File(new_path);
        int i = 0;

        while(new_file.isFile()){
            i++;
            new_file = new File(new_path + "_bkp_" + i );
        }
        if(i>0){
            String dest = new_path + "_bkp_" + i;
            InputStream in = new FileInputStream(new_path);
            OutputStream out = new FileOutputStream(dest);
            byte[] buf = new byte[1024];
            int len;
            while ((len = in.read(buf)) > 0) {
                out.write(buf, 0, len);
            }
            in.close();
            out.close();

            ConfigGenerator.log("Existing file " + new_path + " saved as: " + dest);
        }
	}
}
