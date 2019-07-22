package com.digi.connector.config;

import java.io.IOException;
import java.util.LinkedList;
import java.util.NoSuchElementException;
import java.util.Objects;
import java.util.Set;
import java.util.regex.Pattern;
import java.util.regex.PatternSyntaxException;
import java.util.Arrays;
import java.util.Collections;
import java.util.EnumSet;
import java.util.HashSet;

import com.digi.connector.config.ConfigGenerator.UseNames;

public class Element extends Item {
	public enum Type {
	    STRING(1),
	    MULTILINE_STRING(2),
	    PASSWORD(3),
	    INT32(4),
	    UINT32(5),
	    HEX32(6),
	    X_HEX32(7),
	    FLOAT(8),
	    ENUM(9),
	    ON_OFF(11),
	    BOOLEAN(12),
	    IPV4(13),
	    FQDNV4(14),
	    FQDNV6(15),
	    LIST(17),
	    MAC_ADDR(21),
	    DATETIME(22),
	    REF_ENUM(23),
	    
	    // Virtual types
	    REGEX(-1) // Converts to STRING
	    ;

	    /* special type since enum name cannot start with 0x */
	    private final static String STRING_0XHEX32 = "0X_HEX32";
	    private final int value;

	    private Type(int value) {
	        this.value = value;
	    }

	    public String toUpperName() {
	        if (this == X_HEX32)
	            return STRING_0XHEX32;
	        else
	            return name();
	    }

	    public String toLowerName() {
	        return toUpperName().toLowerCase();
	    }

	    public int toValue() {
	        return value;
	    }

	    public static Type toType(String str) throws Exception {
	        try {
	            if (str.equalsIgnoreCase(STRING_0XHEX32)) {
	                return X_HEX32;
	            } else {
	                return valueOf(str.toUpperCase());
	            }
	        } catch (Exception e) {
	            throw new Exception("Invalid element Type: " + str);
	        }
	    }
	    
        public String toString() {
        	return toLowerName();
        }
	}

	private final static EnumSet<Type> supportsMinMax = EnumSet.of(
			Type.STRING,
			Type.MULTILINE_STRING,
			Type.PASSWORD,
			Type.INT32,
			Type.UINT32,
			Type.HEX32,
			Type.X_HEX32,
			Type.FLOAT,
			Type.FQDNV4,
			Type.FQDNV6,
			
			Type.REGEX
			);
    private final static EnumSet<Type> requiresMax = EnumSet.of(
    		Type.STRING,
    		Type.MULTILINE_STRING,
    		Type.PASSWORD,
			Type.FQDNV4,
			Type.FQDNV6,
			
			Type.REGEX
    		);

    private static final Set<String> validOnOff = Collections.unmodifiableSet(new HashSet<String>(Arrays.asList(new String[] { "on","off" })));
    private static final Set<String> validBoolean = Collections.unmodifiableSet(new HashSet<String>(Arrays.asList(new String[] { "true","false" })));
	private static final Pattern validIPv4 = Pattern.compile("^(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])[.](25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])[.](25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])[.](25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])$");
	private static final Pattern validFQDN = Pattern.compile("(^[a-zA-Z]+://)?([a-z0-9-]*([.]|(::?))[a-z0-9-]*)+(:[0-9]+)?$");
    
    private final static Long INT32_MIN_VALUE = Long.valueOf(-2147483648L);
    private final static Long INT32_MAX_VALUE = Long.valueOf(2147483647L);
    private final static Long UINT32_MIN_VALUE = Long.valueOf(0L);
    private final static Long UINT32_MAX_VALUE = Long.valueOf(4294967295L);

    private final static String VALUE_INVALID = " value invalid";
    private final static String VALUE_TOO_LOW = " value is below protocol minimum";
    private final static String VALUE_TOO_HIGH = " value is above protocol maximum";
    private final static String MIN_GREATER_THAN_MAX = "min value > max value";
    private final static String DEF_OUT_OF_RANGE = "default value is out of range";

    private Type type;
    private String min;
    private String max;
    private String def;
    private String units;
    private String regexPattern;
    private String regexCase;
    private String regexSyntax;
    
    private final LinkedList<Value> values;
    private final LinkedList<Reference> refs;

    public Element(String name, String description, String helpDescription) throws IOException {
        super(name, description, helpDescription);
        this.values = new LinkedList<Value>();
        this.refs = new LinkedList<Reference>();
    }

    public org.dom4j.Element asElement(Integer id) {
    	final Type rci_type = (type == Type.REGEX) ? Type.STRING : type;
    	
    	org.dom4j.Element e = org.dom4j.DocumentHelper.createElement("element")
			.addAttribute("name", name)
			.addAttribute("desc", getRciDescription())
			.addAttribute("type", rci_type.toString())
			.addAttribute("access", Objects.toString(access, null))
			.addAttribute("min", Objects.toString(min, null))
			.addAttribute("max", Objects.toString(max, null))
			.addAttribute("units", Objects.toString(units, null))
			.addAttribute("default", Objects.toString(def, null));
		
    	if (type == Type.REGEX) {
	    	assert regexPattern != null : "validation of regex_pattern failed";
	    	assert regexSyntax != null : "validation of regex_syntax failed";

	    	e.addAttribute("regex_pattern", regexPattern)
    			.addAttribute("regex_case", Objects.toString(regexCase, null))
    			.addAttribute("regex_syntax", regexSyntax);
    	}
    	
 		e.addAttribute("bin_id", id.toString());
    	
        switch (type) {
        case ENUM:
            for (Value value: getValues()) {
                Integer value_id = getValues().indexOf(value);
                
                e.add(value.asElement(value_id));
            }
            break;
        case REF_ENUM:
            for (Value value: getValues()) {
                e.add(value.asElement(null));
            }
            for (Reference ref: getRefs()) {
            	e.add(ref.asElement(null));
            }
        	break;
        default:
        	break;
        }

    	return wrapConditional(e);
    }

    public Type setType(String theType) throws Exception {
        if (type != null)
            throw new Exception("Duplicate <type> keyword: " + theType);

        type = Type.toType(theType);
        return type;
    }

    public void setMin(String theMin) throws Exception {
        if (min != null)
            throw new Exception("Duplicate <min> keyword: " + theMin);

        min = theMin;
    }

    public void setMax(String theMax) throws Exception {
        if (max != null)
            throw new Exception("Duplicate <max> keyword: " + theMax);

        max = theMax;
    }

    public void setDefault(String theDefault) throws Exception {
        if (def != null)
            throw new Exception("Duplicate <default> keyword: " + theDefault);

        def = theDefault;
    }

    private boolean containsValue(final String needle) {
    	for (Value value: values) {
    		if (value.getName().equals(needle)) {
    			return true;
    		}
    	}
    	return false;
    }
    
    public void addValue(Config config, String valueName, String description, String helpDescription) throws Exception {
        if (type == null)
            throw new Exception("Missing type of enum or ref_enum on element: " + name);
        
        if (containsValue(valueName))
            throw new Exception("Duplicate <value>: " + valueName);
      
        switch (type) {
        case ENUM:
            config.nameLengthSeen(UseNames.VALUES, valueName.length());
            break;
        case REF_ENUM:
        	break;
        default:
            throw new Exception("Invalid <value> for type: " + type.toLowerName());
        }
        
        Value value = new Value(valueName, description, helpDescription);
        values.add(value);
    }

    private boolean containsRef(final String needle) {
    	for (Reference ref: refs) {
    		if (ref.getName().equals(needle)) {
    			return true;
    		}
    	}
    	return false;
    }
    
    public void addRef(Config config, String refName, String description, String helpDescription) throws Exception {
        if (type == null)
            throw new Exception("Missing type enum on element: " + name);
        
        if (type != Type.REF_ENUM)
            throw new Exception("Invalid <value> for type: " + type.toLowerName());
        
        if (containsRef(refName))
            throw new Exception("Duplicate <value>: " + refName);
      
        Reference ref = new Reference(refName, description, helpDescription);
        config.nameLengthSeen(UseNames.VALUES, refName.length());

        refs.add(ref);
    }

    private String ExceptMissingOrBad(String newValue, String currentValue, String name) throws IOException {
        if (newValue == null)
            throw new IOException("Missing or bad " + name);
        
        if (currentValue != null)
            throw new IOException("Duplicate " + name + ": " + newValue);
    	
        return newValue;
    }
    
    public void setUnit(String theUnits) throws IOException {
        units = ExceptMissingOrBad(theUnits, units, "units");
    }

    public void setRegexPattern(String thePattern) throws IOException {
    	regexPattern = ExceptMissingOrBad(thePattern, regexPattern, "regex pattern");
    	try {
    		Pattern.compile(regexPattern);
    	} catch (PatternSyntaxException e) {
    		throw new IOException("Invalid regex pattern: " + e.getMessage());
    	}
    }

    public void setRegexCase(String theCase) throws IOException {
    	regexCase = ExceptMissingOrBad(theCase, regexCase, "regex case");
    }

    public void setRegexSyntax(String theSyntax) throws IOException {
    	regexSyntax = ExceptMissingOrBad(theSyntax, regexSyntax, "regex syntax");
    }

    public Type getType() {
        return type;
    }

    public Type getRciType() {
    	return (type == Type.REGEX) ? Type.STRING : type; 
    }

    public String getMin() {
        return min;
    }

    public String getMax() {
        return max;
    }

    public String getDefault() {
        return def;
    }

    private int getValueIndex(String needle) {
		int index = 0;
    	for (Value value: values) {
    		if (value.getName().equals(needle)) {
    			return index;
    		}
    		index += 1;
    	}
    	
    	return -1;
    }
    
    public String getDefaultValue() {
    	switch (type) {
    	case STRING:
    	case MULTILINE_STRING:
    	case PASSWORD:
    	case IPV4:
    	case FQDNV4:
    	case FQDNV6:
    	case MAC_ADDR:
    	case DATETIME:
    	case REF_ENUM:
    	case REGEX:
    		return Code.quoted(def); 
    		
    	case INT32:
    	case UINT32:
    	case HEX32:
    	case X_HEX32:
    	case FLOAT:
    		return def;
    		
    	case ENUM:
    	{
    		int index = getValueIndex(def);
    		if (index >= 0) {
      			return String.valueOf(index);
        	}

        	assert false: "default value not found";
    		return null;
    	}
    		
    	case ON_OFF:
    	case BOOLEAN:
    		return "connector_" + def;
    		
    	case LIST:
    		assert false: "list should not have a default";
    		return null;

    	default:
			assert false: "unexpected default case";
			return null;
		}
    }

    public String getUnit() {
        return units;
    }

    public LinkedList<Value> getValues() {
        return values;
    }
    
    public LinkedList<Reference> getRefs() {
        return refs;
    }
    
    private Float toFloat(String string, Float def, String which) throws Exception {
    	if (string == null)
    		return def;
    	
    	Float value;
        try {
            value = Float.valueOf(string);
        } catch (NumberFormatException e) {
            throw new Exception(which + VALUE_INVALID);
        }

        if (value.isNaN()) {
		throw new Exception(which + VALUE_INVALID);
        }

        return value;
    }

    private Long toLong(String string, Long min, Long max, Long def, String which) throws Exception {
    	if (string == null)
    		return def;

    	boolean is_hex = string.startsWith("0x");
    	String trimmed =  is_hex ? string.substring(2) : string;
    	int radix = is_hex ? 16 : 10;
    		
        try {
            Long value = Long.valueOf(trimmed, radix);
        	if (value < min) {
        		throw new Exception(which + VALUE_TOO_LOW);
        	}

        	if (value > max) {
        		throw new Exception(which + VALUE_TOO_HIGH);
        	}
            return value;
        } catch (NumberFormatException e) {
            throw new Exception(which + VALUE_INVALID);
        }
    }

    private void validateIpV4() throws Exception {
		if (validIPv4.matcher(def) == null) {
			throw new Exception("invalid IPv4 address");
		}
    }
    
    private void validateFQDN(Type type) throws Exception {
		if (validFQDN.matcher(def) == null) {
			throw new Exception("invalid FQDN address");
		}
    }
    
    public void validate() throws Exception {
        if (type == null) {
            throw new Exception("Missing <type>");
        }

        if (!supportsMinMax.contains(type)) {
        	if (min != null) {
                throw new Exception("min is not supported");
        	}
        	
    		if (max != null) {
                throw new Exception("max is not supported");
        	}
        }
        
        if (requiresMax.contains(type) && max == null) {
            throw new Exception("max is required");
        }

        switch (type) {
        case ENUM:
            if (values.isEmpty()) {
                throw new Exception("No values found for enum type");
            }
            
            if ((def != null) && (getValueIndex(def) == -1)) {
        		throw new Exception("default enumeration value not found");
            }
            break;
            
        case REF_ENUM:
            if (values.isEmpty() && refs.isEmpty()) {
                throw new Exception("No values or references found for ref_enum type");
            }
            
            if (def != null) {
            	boolean found = (getValueIndex(def) != -1);
            	
            	if (!found) {
            		for (Reference ref: refs) {
            			found = def.startsWith(ref.getName());
       					if (found) {
            				break;
            			}
            		}
            	}
            	if (!found) {
            		throw new Exception("default enumeration value not found");
            	}
            }
            break;
            
    	case ON_OFF:
    		if ((def != null) && !validOnOff.contains(def)) {
            	throw new Exception("Bad default on_off value");
    		}
    		break;
    		
    	case BOOLEAN:
    		if ((def != null) && !validBoolean.contains(def)) {
            	throw new Exception("Bad default boolean value");
    		}
    		break;
    		
    	case IPV4:
			if (def != null) {
				validateIpV4();
			}
			break;
			
    	case LIST:
            if (def != null) {
            	throw new Exception("Default value is invalid for <list>");
            }
            break;
    		
    	case MAC_ADDR:
            if (def != null) {
            	throw new Exception("Default value is unsupported for mac_addr");
            }
            break;

    	case DATETIME:
            if (def != null) {
            	throw new Exception("Default value is unsupported for datetime");
            }
            break;

        case FLOAT:
	        {
	            Float minValue = toFloat(min, Float.MIN_VALUE, "min");
	            Float maxValue = toFloat(max, Float.MAX_VALUE, "max");
	        	if (minValue > maxValue) {
	                throw new Exception(MIN_GREATER_THAN_MAX);
	        	}
	        	
	        	if (def != null) {
		            Float defValue = toFloat(def, null, "default");
		            assert defValue != null;
		            
		            boolean good = (defValue >= minValue) && (defValue <= maxValue);
		            if (!good) {
		            	throw new Exception(DEF_OUT_OF_RANGE);
		            }
	        	}
	        	break;
	        }

        case INT32:
	        {
	            Long minValue = toLong(min, INT32_MIN_VALUE, INT32_MAX_VALUE, INT32_MIN_VALUE, "min");
	            Long maxValue = toLong(max, INT32_MIN_VALUE, INT32_MAX_VALUE, INT32_MAX_VALUE, "max");
	        	if (minValue > maxValue) {
	                throw new Exception(MIN_GREATER_THAN_MAX);
	        	}

	        	if (def != null) {
		            Long defValue = toLong(def, INT32_MIN_VALUE, INT32_MAX_VALUE, null, "default");
		            assert defValue != null;
		            
		            boolean good = (defValue >= minValue) && (defValue <= maxValue);
		            if (!good) {
		            	throw new Exception(DEF_OUT_OF_RANGE);
		            }
	        	}
	        	break;
	        }
        	
        default:
		    {
		    	// Handle virtual types
		    	if (type == Type.REGEX) {
		    		if (regexPattern == null) {
			    		throw new Exception("Regular expressions require a pattern.");
		    		}
		    		if (regexSyntax == null) {
			    		throw new Exception("Regular expressions require a syntax.");
		    		}
		    	} else {
		    		String rejectedAttribute;
		    		
		    		if (regexPattern != null) {
		    			rejectedAttribute = "pattern";
		    		} else if (regexCase != null) {
		    			rejectedAttribute = "case";
		    		} else if (regexSyntax != null) {
		    			rejectedAttribute = "syntax";
		    		} else {
		    			rejectedAttribute = null;
		    		}
		    		
		    		if (rejectedAttribute != null) {
		    			throw new Exception("'" + rejectedAttribute + "' is only valid for 'regex' type");
		    		}
		    	}

		    	if (supportsMinMax.contains(type)) {
			        Long minValue = toLong(min, UINT32_MIN_VALUE, UINT32_MAX_VALUE, UINT32_MIN_VALUE, "min");
			        Long maxValue = toLong(max, UINT32_MIN_VALUE, UINT32_MAX_VALUE, UINT32_MAX_VALUE, "max");
			    	if (minValue > maxValue) {
			    		throw new Exception(MIN_GREATER_THAN_MAX);
			    	}
			    	
			    	switch (type) {
			    	case STRING:
			    	case MULTILINE_STRING:
			    	case PASSWORD:
			    	case REGEX:
			    		if (def != null) {
			    			Long length = (long) def.length();
			    			
				            boolean good = (length >= minValue) && (length <= maxValue);
				            if (!good) {
				            	throw new Exception(DEF_OUT_OF_RANGE);
				            }
			    		}
			    		break;
			    		
			    	case UINT32:
			    	case HEX32:
			    	case X_HEX32:
			        	if (def != null) {
				            Long defValue = toLong(def, UINT32_MIN_VALUE, UINT32_MAX_VALUE, null, "default");
				            assert defValue != null;
				            
				            boolean good = (defValue >= minValue) && (defValue <= maxValue);
				            if (!good) {
				            	throw new Exception(DEF_OUT_OF_RANGE);
				            }
			        	}
			    		break;
			    		
			    	case FQDNV4:
			    	case FQDNV6:
			    		if (def != null) {
			    			validateFQDN(type);
			    		}
			    		break;

			    	default:
			    		assert false: "unexpected type in switch"; 
			    	}
		    	}
		    	break;
		    }
        }
    }
    
    public Item find(final String name) throws NoSuchElementException {
    	throw new NoSuchElementException();
    }
}
