package com.digi.connector.config;

import java.io.IOException;
import java.util.LinkedList;
import java.util.Set;
import java.util.regex.Pattern;
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
	    DATETIME(22);

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
			Type.FQDNV6
			);
    private final static EnumSet<Type> requiresMax = EnumSet.of(
    		Type.STRING,
    		Type.MULTILINE_STRING,
    		Type.PASSWORD,
			Type.FQDNV4,
			Type.FQDNV6
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
    private final LinkedList<Value> values;

    public Element(String name, String description, String helpDescription) throws IOException {
        super(name, description, helpDescription);
        this.values = new LinkedList<Value>();
    }

    public String toString(int id) {
        String descriptor = String.format("<element name=`%s` desc=`%s` type=`%s`", name, Descriptors.encodeEntities(toRciDescription()), type);

        if (access != null)
            descriptor += String.format(" access=`%s`", access);
        if (min != null)
            descriptor += String.format(" min=`%s`", min);
        if (max != null)
            descriptor += String.format(" max=`%s`", max);
        if (units != null)
            descriptor += String.format(" units=`%s`", units);
        if (def != null)
            descriptor += String.format(" default=`%s`", def);

        descriptor += String.format(" bin_id=`%d`", id);

        try {

            if (type == Type.ENUM)
                descriptor += ">";
            else
                descriptor += " />";

        } catch (Exception e) {
            e.printStackTrace();
        }

        descriptor = descriptor.replace('`', '"');

        return descriptor;
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
    
    public void addValue(ConfigData config, String valueName, String description, String helpDescription) throws Exception {
        if (type == null)
            throw new Exception("Missing type enum on element: " + name);
        
        if (type != Type.ENUM)
            throw new Exception("Invalid <value> for type: " + type.toLowerName());
        
        if (containsValue(valueName))
            throw new Exception("Duplicate <value>: " + valueName);
      
        Value value = new Value(valueName, description, helpDescription);
        config.nameLengthSeen(UseNames.VALUES, valueName.length());

        values.add(value);
    }

    public void setUnit(String theUnit) throws IOException {
        if (theUnit == null)
            throw new IOException("Missing or bad units description!");
        
        if (units != null)
            throw new IOException("Duplicate units: " + theUnit);
        
        units = theUnit;
    }

    public Type getType() {
        return type;
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
    		return Code.quoted(def); 
    		
    	case INT32:
    	case UINT32:
    	case HEX32:
    	case X_HEX32:
    	case FLOAT:
    		return def;
    		
    	case ENUM:
    	{
    		int index = 0;
        	for (Value value: values) {
        		if (value.getName().equals(def)) {
        			return String.valueOf(index);
        		}
        		index += 1;
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
    
    private Float toFloat(String string, Float def, String which) throws Exception {
    	if (string == null)
    		return def;
    	
    	Float value;
        try {
            value = Float.valueOf(string);
        } catch (NumberFormatException e) {
            throw new Exception(which + VALUE_INVALID);
        }
        
    	if (value < Float.MIN_VALUE) {
    		throw new Exception(which + VALUE_TOO_LOW);
    	}

    	if (value > Float.MAX_VALUE) {
    		throw new Exception(which + VALUE_TOO_HIGH);
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
            
            if (def != null) {
            	boolean found = false;
            	for (Value value: values) {
            		if (value.getName().equals(def)) {
            			found = true;
            			break;
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
}
