package ru.dz.jpc.python;

import java.io.File;
import java.io.IOException;

import javax.xml.parsers.ParserConfigurationException;

import org.xml.sax.SAXException;

import ru.dz.plc.compiler.ClassMap;

public class PpcMain {

	/**
	 * @param args
	 * @throws ParserConfigurationException 
	 * @throws IOException 
	 * @throws SAXException 
	 * @throws ConnvertException 
	 */
	public static void main(String[] args) throws ParserConfigurationException, SAXException, IOException, ConnvertException {
		ClassMap cm = new ClassMap();

		PythonFrontendXML pfx = new PythonFrontendXML();
		
		pfx.load(new File("pyfront/testinput.out"));
		
		//pfx.print();
		
		PythonConvertor pc = new PythonConvertor();
		
		pfx.convert();
		
	}

	private static void convert(PythonFrontendXML pfx) {
		// TODO Auto-generated method stub
		
	}

}
