package main;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;


public class SRecMergeTool {
	
	// Constants.
	private static final String OPTION_HELP = "-h";
	
	private static final String HEADER = "S3";
	
	private static final String EXTENSION_S19 = ".s19";
	
	private static final String FILE_MERGED = "bootloader_merged.s19";
	
	private static final String END_LINE = "\n";
	
	private static final byte BYTES_LENGTH = 0x0D;
	private static final byte[] LOCATION_ADDRESS = new byte[]{0x00, 0x00, (byte)0xB8,0x00};
	private static final byte[] MAGIC_COOKIE = new byte[]{(byte)0xDE, (byte)0xAD, (byte)0xBE, (byte)0xEF};
	
	public static void main(String[] args) {
		if (args.length > 0 && args[0].equals(OPTION_HELP)) {
			printUsage();
			System.exit(0);
		}
		
		if (args.length != 2) {
			System.out.println("Invalid number of arguments.");
			printUsage();
			System.exit(-1);
		}
		
		String bootloaderFilePath = args[0];
		String applicationFilePath = args[1];
		
		File bootloaderFile = new File(bootloaderFilePath);
		File applicationFile = new File(applicationFilePath);
		
		if (!bootloaderFilePath.toLowerCase().endsWith(EXTENSION_S19)
				|| !bootloaderFile.exists()) {
			System.out.println("Invalid bootloader s19 file.");
			System.exit(-1);
		}
		
		if (!applicationFilePath.toLowerCase().endsWith(EXTENSION_S19)
				|| !applicationFile.exists()) {
			System.out.println("Invalid application s19 file.");
		}
		
		mergeFiles(bootloaderFile, applicationFile);
	}
	
	public static void printUsage() {
		System.out.println(
				"This application allows for merging an MQX bootloader s19 file with\n" +
				"the s19 file of an Etherios Device Connector for MQX application.\n" +
				"\n" +
				"Usage: SRecMergeTool.jar [options] bootloader_s19_file application_s19_file\n" +
				"\n" +
				"Options:\n" +
				"     -h     Shows this message\n" +
				"\n");
	}
	
	private static void mergeFiles(File bootloaderFile, File applicationFile) {
		BufferedReader bootloaderReader = null;
		BufferedReader applicationReader = null;
		BufferedWriter writer = null;
		
		String line = "";
		String prevLine = null;
		
		boolean srecWritten = false;
		
		byte[] entryPoint = getEntryPoint(applicationFile);
		if (entryPoint == null)
			return;
		
		try {
			bootloaderReader = new BufferedReader(new FileReader(bootloaderFile));
			applicationReader = new BufferedReader(new FileReader(applicationFile));
			writer = new BufferedWriter(new FileWriter(new File(bootloaderFile.getParentFile(), FILE_MERGED)));
			
			line = bootloaderReader.readLine();
			while (line != null) {
				if (line.equals("")) {
					writeSrec(writer, entryPoint);
					srecWritten = true;
				}
				if (prevLine != null)
					writer.write(prevLine + END_LINE);
				
				prevLine = line;
				line = bootloaderReader.readLine();
			}
			if (!srecWritten)
				writeSrec(writer, entryPoint);
			
			line = applicationReader.readLine();
			while (line != null) {
				writer.write(line + END_LINE);
				line = applicationReader.readLine();
			}
		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			try {
				if (bootloaderReader != null)
					bootloaderReader.close();
				if (applicationReader != null)
					bootloaderReader.close();
				if (writer != null)
					writer.close();
			} catch (IOException e) { }
		}
	}
	
	/**
	 * Generates the new SREC line and writes it in the given BufferedWriter.
	 * 
	 * @param writer The BufferedWriter to write the generated SREC line.
	 * @throws IOException
	 */
	public static void writeSrec(BufferedWriter writer, byte[] entryPoint) throws IOException {
		String srecLine = HEADER;
		byte[] srecBytes = new byte[LOCATION_ADDRESS.length + 
		                            MAGIC_COOKIE.length + 
		                            entryPoint.length + 
		                            1];
		byte chkSum = 0;
				
		srecBytes[0] = BYTES_LENGTH;
		int ind = 1;
		chkSum += BYTES_LENGTH;
		
		for (int i=0; i<LOCATION_ADDRESS.length; i++) {
			srecBytes[i + ind] = LOCATION_ADDRESS[i];
			chkSum += LOCATION_ADDRESS[i];
		}
		ind += LOCATION_ADDRESS.length;
		
		for (int i=0; i<MAGIC_COOKIE.length; i++) {
			srecBytes[i + ind] = MAGIC_COOKIE[MAGIC_COOKIE.length - 1 - i];
			chkSum += MAGIC_COOKIE[MAGIC_COOKIE.length - 1 - i];
		}
		ind += MAGIC_COOKIE.length;
		
		for (int i=0; i<entryPoint.length; i++) {
			srecBytes[i + ind] = entryPoint[i];
			chkSum += entryPoint[i];
		}
		ind += entryPoint.length;
		
		for (byte sRecByte:srecBytes)
			srecLine += String.format("%02X", sRecByte);
		srecLine += String.format("%02X", (byte)~chkSum);
		
		writer.write(srecLine + END_LINE);
	}
	
	/**
	 * 
	 * @param byteArray
	 * @return
	 */
	private static byte[] swapEndian(byte[] byteArray) {
		byte[] finalByteArray = new byte[byteArray.length];
		for (int i=0; i<byteArray.length; i++)
			finalByteArray[i] = byteArray[byteArray.length -1 - i];
		
		return finalByteArray;
	}
	
	/**
	 * 
	 * @param sRecFilePath
	 * @return
	 */
	private static byte[] getEntryPoint(File srecAppFilePath) {
		byte[] entryPoint = new byte[4];
		BufferedReader reader = null;
		
		String line = "";
		String entryPointLine = null;
		
		try {
			reader = new BufferedReader(new FileReader(srecAppFilePath));
			
			line = reader.readLine();
			while (line != null) {
				if (!line.equals(""))
					entryPointLine = line;
				line = reader.readLine();
			}
		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			try {
				if(reader != null)
					reader.close();
			} catch (IOException e) { }
		}
		
		if (entryPointLine  == null || entryPointLine.length() < 14)
			return null;
		
		for (int i=0; i<4; i++)
			entryPoint[i] = hexStringToByteArray((entryPointLine.substring(4 + i*2, 6 + i*2)))[0];
		
		return swapEndian(entryPoint);
	}
	
	/**
	 * Converts the given hex string into a byte array.
	 * 
	 * @param value Hex string to convert to.
	 * @return Byte array of the given hex string.
	 */
	public static byte[] hexStringToByteArray(String value) {
		int len = value.length();
		byte[] data = new byte[len / 2];
		for (int i = 0; i < len; i += 2) {
			data[i / 2] = (byte) ((Character.digit(value.charAt(i), 16) << 4)
					+ Character.digit(value.charAt(i+1), 16));
		}
		return data;
	}
}
