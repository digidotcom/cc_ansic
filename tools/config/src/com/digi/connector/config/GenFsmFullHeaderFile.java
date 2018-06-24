package com.digi.connector.config;

import java.util.EnumSet;

public class GenFsmFullHeaderFile extends GenFsmHeaderFile {
	public GenFsmFullHeaderFile(String directoryPath) throws Exception {
		super(directoryPath, EnumSet.allOf(Element.Type.class));
	}
}
