package com.digi.connector.config;

import java.io.IOException;
import java.util.LinkedList;
import java.util.EnumSet;

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
	    private final static String STRING_0XHEX32 = "0x_hex32";
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

	    public static Element.Type toType(String str) throws Exception {
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
    		Type.PASSWORD
    		);

    private final static Long INT32_MIN_VALUE = Long.valueOf(-2147483648L);
    private final static Long INT32_MAX_VALUE = Long.valueOf(2147483647L);
    private final static Long UINT32_MIN_VALUE = Long.valueOf(0L);
    private final static Long UINT32_MAX_VALUE = Long.valueOf(4294967295L);

    private final static String BAD_MIN_VALUE = "Bad min value";
    private final static String BAD_MAX_VALUE = "Bad max value";
    
    private final static String MIN_GREATER_THAN_MAX = "min value > max value";

    private Type type;
    private String min;
    private String max;
    private String units;
    private final LinkedList<ValueStruct> values;

    public Element(String name, String description, String helpDescription) throws IOException {
        super(name, description, helpDescription);
        this.values = new LinkedList<ValueStruct>();
    }

    public String toString(int id) {
        String descriptor = String.format("<element name=`%s` desc=`%s` type=`%s`", name, toRciDescription(), type);

        if (access != null)
            descriptor += String.format(" access=`%s`", access);
        if (min != null)
            descriptor += String.format(" min=`%s`", min);
        if (max != null)
            descriptor += String.format(" max=`%s`", max);
        if (units != null)
            descriptor += String.format(" units=`%s`", units);

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

    public void addValue(String valueName, String description, String helpDescription) throws Exception {
        if (type == null)
            throw new Exception("Missing type enum on element: " + name);
        
        if (type != Type.ENUM)
            throw new Exception("Invalid <value> for type: " + type.toLowerName());
        
        if (values.contains(valueName))
            throw new Exception("Duplicate <value>: " + valueName);
      
        ValueStruct value = new ValueStruct(valueName, description, helpDescription);
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

    public String getUnit() {
        return units;
    }

    public LinkedList<ValueStruct> getValues() {
        return values;
    }
    
    private Float toFloat(String string) throws Exception {
    	if (string == null)
    		return null;
    	
        try {
            return Float.valueOf(string);
        } catch (NumberFormatException e) {
            throw new Exception(BAD_MIN_VALUE);
        }
    }
    
    private Long toLong(String string, Long min, Long max, String error) throws Exception {
    	if (string == null)
    		return null;

    	boolean is_hex = string.startsWith("0x");
    	String trimmed =  is_hex ? string.substring(2) : string;
    	int radix = is_hex ? 16 : 10;
    		
        try {
            Long result = Long.valueOf(trimmed, radix);
            if ((result < min) || (result > max))
                throw new Exception(error);
            return result;
        } catch (NumberFormatException e) {
            throw new Exception(error);
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
        
        if (type == Type.ENUM && values.isEmpty()) {
            throw new Exception("No values found for enum type");
        }

        if (requiresMax.contains(type) && max == null) {
            throw new Exception("max is required");
        }

        switch (type) {
        case ENUM:
            if (values.isEmpty()) {
                throw new Exception("Missing <value>!");
            }
            break;

        case FLOAT:
	        {
	        	boolean have_none = (min == null) && (max == null);
	        	if (have_none)
	        		break;
	        	
	            Float minValue = toFloat(min);
	            Float maxValue = toFloat(max);
	        	boolean have_both = (minValue != null) && (maxValue != null);
	        	if (have_both && (minValue > maxValue)) {
	                throw new Exception(MIN_GREATER_THAN_MAX);
	        	}
	        	break;
	        }

        case INT32:
	        {
	        	boolean have_none = (min == null) && (max == null);
	        	if (have_none)
	        		break;
	        	
	            Long minValue = toLong(min, INT32_MIN_VALUE, INT32_MAX_VALUE, BAD_MIN_VALUE);
	            Long maxValue = toLong(max, INT32_MIN_VALUE, INT32_MAX_VALUE, BAD_MAX_VALUE);
	        	boolean have_both = (minValue != null) && (maxValue != null);
	        	if (have_both && (minValue > maxValue)) {
	                throw new Exception(MIN_GREATER_THAN_MAX);
	        	}
	        	break;
	        }
        	
        default:
		    {
		    	if (!supportsMinMax.contains(type))
		    		break;
		    	
		    	boolean have_none = (min == null) && (max == null);
		    	if (have_none)
		    		break;
		    	
		        Long minValue = toLong(min, UINT32_MIN_VALUE, UINT32_MAX_VALUE, BAD_MIN_VALUE);
		        Long maxValue = toLong(max, UINT32_MIN_VALUE, UINT32_MAX_VALUE, BAD_MAX_VALUE);
		    	boolean have_both = (minValue != null) && (maxValue != null);
		    	if (have_both && (minValue > maxValue)) {
		    		throw new Exception(MIN_GREATER_THAN_MAX);
		    	}
		    	break;
		    }
        }
    }
}
