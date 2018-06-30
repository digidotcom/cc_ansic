package com.digi.connector.config;

import java.util.List;
import java.util.Map;
import java.util.LinkedList;

public class GenSource {
	private static GenSource instance = null;
	private GenSource() {}
	public static final GenSource getInstance() { if (instance == null) instance = new GenSource(); return instance; }
	
	private final String QUOTE = Character.toString('"');
	public final String commented(String string) { return "/* " + string + " */"; }
	public final String quoted(String string) { return QUOTE + string + QUOTE; }
	public final String indented(String line) { return "    " + line; }
	
	public final LinkedList<String> indented(List<String> lines) {
		LinkedList<String> result = new LinkedList<>();

		for (String line: lines) {
			result.add(indented(line));
		}
		return result;
	} 
	
	public final LinkedList<String> commas(List<String> lines) {
		LinkedList<String> result = new LinkedList<>();

		assert !lines.isEmpty();
		int last = lines.size() -1;
		for (String line: lines.subList(0, last)) {
			assert !line.isEmpty();
			result.add(line + ",");
		}
		result.add(lines.get(last));
		return result;
	}

	private final LinkedList<String> _struct(String typedef, String tag, String type, List<String> fields) {
		LinkedList<String> result = new LinkedList<>();
		
		assert tag.trim() == tag;
		if (!tag.isEmpty()) {
			tag = tag + " ";
		}
		result.add(typedef + "struct " + tag + "{");
		for (String field: fields) {
			result.add(indented(field));
		}
		assert type.trim() == type;
		if (!type.isEmpty()) {
			type = " " + type;
		}
		result.add("}" + type + ";");
		return result;
	}
	
	public final LinkedList<String> structTaggedTypedef(String tag, String type, List<String> fields) { return _struct("typedef ", tag, type, fields); }
	public final LinkedList<String> structTagged(String tag, List<String> fields) { return _struct("", tag, "", fields); }
	public final LinkedList<String> structTypedef(String type, List<String> fields) { return structTaggedTypedef("", type, fields); }
	public final LinkedList<String> struct(List<String> fields) { return structTagged("", fields); }
	
	public final String define(String name, String value) {
		assert !name.isEmpty();
		String result = "#define " + name;
		if (value != null) {
			assert !value.isEmpty();
			result += " " + value;
		}
		return result;
	}

	public final String include(String filename) { return "#include " + quoted(filename); }
	
	public final String define(String name) { return define(name, null); }
	public final LinkedList<String> define(List<String> names) {
		LinkedList<String> result = new LinkedList<>();
		
		for (String name: names) {
			result.add(define(name));
		}
		return result;
	}

	public final LinkedList<String> define(Map<String, Object> pairs) {
		LinkedList<String> result = new LinkedList<>();
		
		for (Map.Entry<String, Object> pair : pairs.entrySet()) {
		    String name = pair.getKey();
		    Object value = pair.getValue();
		    
		    result.add(define(name, value.toString()));
		}
		return result;
	}
	
	private final LinkedList<String> _enumeration(String typedef, String tag, String type, Map<String, Integer> pairs) {
		LinkedList<String> result = new LinkedList<>();
		
		assert tag.trim() == tag;
		if (!tag.isEmpty()) {
			tag = tag + " ";
		}
		result.add(typedef + "enum " + tag + "{");
		
		LinkedList<String> lines = new LinkedList<>();
		int expected = 0;
		for (Map.Entry<String, Integer> pair : pairs.entrySet()) {
		    String name = pair.getKey();
		    Integer value = pair.getValue();
		    String line = name;
		    
		    if (value != expected) {
		    	line += " = " + value;
		    }
		    expected = value + 1;
		    
		    lines.add(indented(line));
		}
		result.addAll(commas(lines));
		
		assert type.trim() == type;
		if (!type.isEmpty()) {
			type = " " + type;
		}
		result.add("}" + type + ";");
		return result;
	}
	
	public final LinkedList<String> enumerationTaggedTypedef(String tag, String type, Map<String, Integer> pairs) { return _enumeration("typedef ", tag, type, pairs); }
	public final LinkedList<String> enumerationTagged(String tag, Map<String, Integer> pairs) { return _enumeration("", tag, "", pairs); }
	public final LinkedList<String> enumerationTypedef(String type, Map<String, Integer> pairs) { return enumerationTaggedTypedef("", type, pairs); }
	public final LinkedList<String> enumeration(Map<String, Integer> pairs) { return enumerationTagged("", pairs); }
	
	public final String prototype(String return_type, String name, String parameters) {
		return return_type + " " + name + "(" + parameters + ");";
	}
	public final LinkedList<String>  prototypes(String return_type, List<String> names, String parameters) {
		LinkedList<String> result = new LinkedList<>();
		
		for (String name: names) {
			result.add(prototype(return_type, name, parameters));
		}
		return result;
	}
}
