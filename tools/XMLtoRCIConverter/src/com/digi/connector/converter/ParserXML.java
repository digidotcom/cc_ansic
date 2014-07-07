package com.digi.connector.converter;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import org.w3c.dom.Document;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.Element;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;


public class ParserXML{

	private final static int MAX_DESCRIPTION_LENGTH = 40; /* same value as in ConfigGenerator-Parser.java */

    private static NodeList nodeList;
    private static int len;
    private static String groupType;
    private static String groupName;
    private static String groupDesc;
    private static String attrMax;
    private static String helpDesc;
    private static BufferedWriter rciWriter = null;

    public static void processFile(String fileName,String directoryPath,String fileOut) throws IOException, NullPointerException {

        String filePath="";
        try {
            if (directoryPath != null) {
                filePath = directoryPath;
                /* add a / if last char is not / */
                if (!directoryPath.endsWith("/")) filePath += '/';
            }

            if(fileOut != null){
                filePath += fileOut;
                if (!fileOut.endsWith(".rci")) filePath += ".rci";
            }
            else
                filePath += "config.rci";

        	File XmlFile = new File(fileName);
        	DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
        	DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
        	Document doc = dBuilder.parse(XmlFile);
        	doc.getDocumentElement().normalize();
            rciWriter = new BufferedWriter(new FileWriter(filePath));
        	parseFile(doc);
            XMLtoRCIConverter.log("File generated: " + filePath);

        } catch (Exception e) {
            e.printStackTrace();
        }
        finally {
            rciWriter.close();
        }

    }

    private static void parseFile(Document doc) throws Exception {

        nodeList = doc.getElementsByTagName("*");
        Element e;
        /*len = number of tags of the xml*/
        len = nodeList.getLength();

        for (int j=0; j < len; j++){
    	   
            e = (Element)nodeList.item(j);
            /*search <query descriptor> tag  */
            if(e.getTagName().equalsIgnoreCase("query_descriptor")){
                int number_types = (e.getChildNodes().getLength()-1)/2;

                /*for each group type: state,setting */
                for(int t =0;t< number_types;t++){
                    j=j+1;
                    e = (Element)nodeList.item(j);
                    if(e.getTagName().equalsIgnoreCase("descriptor"))
                        readGroupType(e);
	
                    int number_groups = (e.getChildNodes().getLength()-1)/2;
                    /*for each group */
                    for(int n=0; n<number_groups;n++){
                        j=j+1;
                        e = (Element)nodeList.item(j);
                        if(e.getTagName().equalsIgnoreCase("descriptor")){
                            readGroupName(e);
                            int number_elements = (e.getChildNodes().getLength()-1)/2;
                            if(number_elements == 0)
                            	//throw new Exception("XML Error: Group " + groupName + " with No element specified");
                            	rciWriter.write(String.format("\n# TODO_empty_group %s %s %s\n",groupType,groupName,groupDesc,helpDesc));
                            /*for each element or error */
                            for(int m=0; m<number_elements;m++){
                                j=j+1;
                                e = (Element)nodeList.item(j);
                                /*check if the group has <attr tag */
                                
                                if(e.getTagName().equalsIgnoreCase("attr")){
                                    writeGroupAttr(e);
                                    rciWriter.write(String.format("\n%s %s %s %s %s\n",groupType,groupName,attrMax,groupDesc,helpDesc));
                                }
                                else{
                                    /*If the group doesn't have <attr */
                                    if(m == 0)
                                        rciWriter.write(String.format("\n%s %s %s\n",groupType,groupName,groupDesc,helpDesc));
                                    if(e.getTagName().equalsIgnoreCase("error_descriptor"))
                                        writeGroupError(e);
                                    else if(e.getTagName().equalsIgnoreCase("element")){
                                        writeGroupElement(e);
                                        int number_values = (e.getChildNodes().getLength()-1)/2;
                                        for(int v=0; v<number_values;v++){
                                            j=j+1;
                                            e = (Element)nodeList.item(j);
                                            if(e.getTagName().equalsIgnoreCase("value")){
                                                writeElementValue(e);
                                            }
                                            else{
                                                throw new Exception("XML Error: Missing \"value\" tag at : " + j );  	        			  
                                            }
                                        }//no more values
                                    }
                                    else
                                        throw new Exception("XML Error: Missing \"element\" tag at : " + j );  
                                }
                            }//no more elements/errors
                        }
                        else
                            throw new Exception("XML Error: Missing \"descriptor\" tag at : " + j );
                    }//no more groups
                }//no more types
            }//end query_descriptor
            else if(e.getTagName().equalsIgnoreCase("error_descriptor")){
                writeGlobalError(e);
            }
        }
    }

    private static void writeGlobalError(Element e) throws IOException{

        NamedNodeMap nodeMap;
        String attrname;
        String attrval;
        String globalErrorName = "";
   	 	String globalErrorDesc = "";
        Node node;
    	nodeMap = e.getAttributes();

    	if (nodeMap != null){
	        for (int i=0; i<nodeMap.getLength(); i++){
	            node = nodeMap.item(i);
	            attrname = node.getNodeName();
	            attrval = node.getNodeValue();
	            if (attrname.equalsIgnoreCase("desc")) {
	                globalErrorName = attrval.replace(" ", "_").toLowerCase();
	                globalErrorDesc = "\"" + attrval + "\"" ;
	            }
	        }
	        rciWriter.write(String.format("globalerror %s %s\n",globalErrorName,globalErrorDesc));
    	}
    }

    private static void readGroupType(Element e){

        NamedNodeMap nodeMap;
        String attrname;
        String attrval;
        groupType ="";
        Node node;
        nodeMap = e.getAttributes();

        if (nodeMap != null){
            for (int i=0; i<nodeMap.getLength(); i++){
                node = nodeMap.item(i);
                attrname = node.getNodeName();
                attrval = node.getNodeValue();
                if(attrname.equalsIgnoreCase("element") && attrval.equalsIgnoreCase("query_setting")){
	                groupType = "group setting";
	            }
	            else if(attrval.equalsIgnoreCase("query_state")){
	            	groupType = "group state";
	            }
	         }
    	}
    }

    private static void readGroupName(Element e){

        NamedNodeMap nodeMap;
        String attrname;
        String attrval;
        groupName = "";
   	 	groupDesc= "";
        Node node;
    	nodeMap = e.getAttributes();

        if (nodeMap != null){
            for (int i=0; i<nodeMap.getLength(); i++){
                node = nodeMap.item(i);
                attrname = node.getNodeName();
                attrval = node.getNodeValue();

                if(attrname.equalsIgnoreCase("element"))
                	groupName = attrval;
                else if(attrname.equalsIgnoreCase("desc")){
                    if(attrval.length() > MAX_DESCRIPTION_LENGTH){
                    	int cut = attrval.lastIndexOf(" ", MAX_DESCRIPTION_LENGTH);
                    	helpDesc = "\"" +attrval.substring(cut+1) + "\"";
                    	groupDesc = "\"" + attrval.substring(0,cut) + "\"";
                    }
                    else {
                    	groupDesc = "\"" + attrval + "\"";
                    	helpDesc ="";
                    }
                }

            }
    	}
    }

    private static void writeGroupAttr(Element e){

        NamedNodeMap nodeMap;
        String attrname;
        String attrval;
        attrMax = "";
        Node node;
    	nodeMap = e.getAttributes();

        if (nodeMap != null){
            for (int i=0; i<nodeMap.getLength(); i++){
	            node = nodeMap.item(i);
	            attrname = node.getNodeName();
	            attrval = node.getNodeValue();

	            if(attrname.equalsIgnoreCase("max"))
	            		attrMax = attrval;
	        }
    	}
    }

    private static void writeGroupElement(Element e) throws IOException{

        NamedNodeMap nodeMap;
        String attrname;
        String attrval;
        String eName = "";
   	 	String eDesc = "";
        String eHelpDesc = "";
   	 	String eType = "";
   	 	String eAccess = "";
   	 	String eMax = "";
   	 	String eMin = "";
   	 	String eUnits = "";
        Node node;
    	nodeMap = e.getAttributes();

        if (nodeMap != null){
            for (int i=0; i<nodeMap.getLength(); i++){
                node = nodeMap.item(i);
                attrname = node.getNodeName();
                attrval = node.getNodeValue();

                if(attrname.equalsIgnoreCase("name"))
                    eName = attrval;
                else if(attrname.equalsIgnoreCase("type")){
                	if (attrval.equalsIgnoreCase("long"))
                		attrval = "int32";
                    eType ="type " + attrval;
                }
                else if(attrname.equalsIgnoreCase("access"))
                    eAccess = " access " + attrval;
                else if(attrname.equalsIgnoreCase("max"))
                    eMax = " max " + attrval;
                else if(attrname.equalsIgnoreCase("min"))
                    eMin = " min " + attrval;
                else if(attrname.equalsIgnoreCase("units"))
                    eUnits =" units \"" + attrval + "\"";
                else if(attrname.equalsIgnoreCase("desc")){
                    if(attrval.length() > MAX_DESCRIPTION_LENGTH){
                    	int cut = attrval.lastIndexOf(" ", MAX_DESCRIPTION_LENGTH);
                    	eHelpDesc = " \"" +attrval.substring(cut+1) + "\"";
                    	eDesc = "\"" + attrval.substring(0,cut) + "\"";
                    }
                    else {
                    	eDesc ="\"" + attrval + "\"" ;
                    }
                }
            }
            if(eDesc.equalsIgnoreCase("\"\""))
            	eDesc ="\"TODO_empty_desc\"" ;
            rciWriter.write(String.format("    element %s %s%s %s%s%s%s%s\n",eName,eDesc,eHelpDesc,eType,eAccess,eMin,eMax,eUnits));     
        }
    }
    
    private static void writeElementValue(Element e) throws IOException{

        NamedNodeMap nodeMap;
        String attrname;
        String attrval;
        Node node;
        String value = "";
        String valdesc = "";
        nodeMap = e.getAttributes();

        if (nodeMap != null){
    	    for (int i=0; i<nodeMap.getLength(); i++){
	            node = nodeMap.item(i);
	            attrname = node.getNodeName();
	            attrval = node.getNodeValue();
	            if(attrname.equalsIgnoreCase("value"))
	                value = attrval;
	            else if(attrname.equalsIgnoreCase("desc"))
	                valdesc ="\"" + attrval + "\"" ;
	            else
	                System.out.println(attrname + "tag not identified");

	        }
    	    rciWriter.write(String.format("        value \"%s\" %s\n",value,valdesc));
    	}
    }

    private static void writeGroupError(Element e) throws IOException{

        NamedNodeMap nodeMap;
        String attrname;
        String attrval;
        String errorname = "";
   	 	String errordesc= "";
        Node node;
        nodeMap = e.getAttributes();

        if (nodeMap != null){
            for (int i=0; i<nodeMap.getLength(); i++){
                node = nodeMap.item(i);
                attrname = node.getNodeName();
                attrval = node.getNodeValue();
                if(attrname.equalsIgnoreCase("desc")){
                    errorname = attrval.replace(" ", "_").toLowerCase();
                    errordesc = "\"" + attrval + "\"" ;
                }
            }
            rciWriter.write(String.format("    error %s %s\n",errorname,errordesc));     
        }
    }

}