//
//  MansOSProxy.java
//  MansOSProxy
//
//  Created by Raimonds Samofals on 08.14.5.
//  Copyright (c) 2008 __MyCompanyName__. All rights reserved.
//

// Additional Jars required :  comm.jar ( comm2.0.3 )

/*
 * @(#)SimpleRead.java	1.12 98/06/25 SMI
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * 
 * Sun grants you ("Licensee") a non-exclusive, royalty free, license
 * to use, modify and redistribute this software in source and binary
 * code form, provided that i) this copyright notice and license appear
 * on all copies of the software; and ii) Licensee does not utilize the
 * software in a manner which is disparaging to Sun.
 * 
 * This software is provided "AS IS," without a warranty of any kind.
 * ALL EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES,
 * INCLUDING ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE OR NON-INFRINGEMENT, ARE HEREBY EXCLUDED. SUN AND
 * ITS LICENSORS SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY
 * LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING THE
 * SOFTWARE OR ITS DERIVATIVES. IN NO EVENT WILL SUN OR ITS LICENSORS
 * BE LIABLE FOR ANY LOST REVENUE, PROFIT OR DATA, OR FOR DIRECT,
 * INDIRECT, SPECIAL, CONSEQUENTIAL, INCIDENTAL OR PUNITIVE DAMAGES,
 * HOWEVER CAUSED AND REGARDLESS OF THE THEORY OF LIABILITY, ARISING
 * OUT OF THE USE OF OR INABILITY TO USE SOFTWARE, EVEN IF SUN HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * 
 * This software is not designed or intended for use in on-line control
 * of aircraft, air traffic, aircraft navigation or aircraft
 * communications; or in the design, construction, operation or
 * maintenance of any nuclear facility. Licensee represents and
 * warrants that it will not use or redistribute the Software for such
 * purposes.
 */
import java.io.*;
import java.util.*;
import gnu.io.*; // import javax.comm.*;

/**
 * Class declaration
 *
 *
 * @author
 * @version 1.8, 08/03/00
 */
public class SimpleRead implements Runnable, SerialPortEventListener {
    static CommPortIdentifier   portId;
    static Enumeration	        portList;
    InputStream		            inputStream;
    OutputStream                outputStream;
    SerialPort		            serialPort;
    Thread		                readThread;
    String                      messageString;
    
    BufferedReader br = new BufferedReader(new InputStreamReader(System.in));    

    /**
     * Method declaration
     *
     *
     * @param args
     *
     * @see
     */
    public static void main(String[] args) {
    boolean		      portFound = false;
    String		      defaultPort = "/dev/tty.usbserial-M4AOEFIR";

 	if (args.length > 0) {
	    defaultPort = args[0];
	} 
   
	portList = CommPortIdentifier.getPortIdentifiers();

	while (portList.hasMoreElements()) {
	    portId = (CommPortIdentifier) portList.nextElement();
	    if (portId.getPortType() == CommPortIdentifier.PORT_SERIAL) {
		if (portId.getName().equals(defaultPort)) {
		    System.out.println("Found port: "+defaultPort);
		    portFound = true;
		    SimpleRead reader = new SimpleRead();
		} 
	    } 
	} 
	if (!portFound) {
	    System.out.println("port " + defaultPort + " not found.");
	} 
 	
    } 

    /**
     * Constructor declaration
     *
     *
     * @see
     */
    public SimpleRead() {
	try {
	    serialPort = (SerialPort) portId.open("SimpleReadApp", 2000);
	} catch (PortInUseException e) {System.out.println("Port already in use by '" +  e.currentOwner + "'" ); }

	try {
	    inputStream = serialPort.getInputStream();
	    outputStream = serialPort.getOutputStream();
	} catch (IOException e) {}

	try {
	    serialPort.addEventListener(this);
	} catch (TooManyListenersException e) {}

	serialPort.notifyOnDataAvailable(true);

	try {
	    serialPort.setSerialPortParams(115200, SerialPort.DATABITS_8, 
					   SerialPort.STOPBITS_1, 
					   SerialPort.PARITY_NONE);
	} catch (UnsupportedCommOperationException e) {}

	readThread = new Thread(this);

	readThread.start();
    }

    /**
     * Method declaration
     *
     *
     * @see
     */
    public void run() {
	try {

	    while(true)
	    {
	    try {

			String msgToSend;
			
			System.out.print("> ");
			messageString = br.readLine(); 

            String [] splittedString = messageString.split(" ");


            
            switch (splittedString.length) {
            case 2:
            
            splittedString[1] = Integer.toString(Integer.parseInt(splittedString[1], 16));
            splittedString[1]  = new PrintfFormat("%5s").sprintf(splittedString[1]);

            msgToSend = splittedString[0] + splittedString[1];
            outputStream.write(msgToSend.getBytes());


                    break;
            case 3:
            splittedString[1] = Integer.toString(Integer.parseInt(splittedString[1], 16));
            splittedString[2] = Integer.toString(Integer.parseInt(splittedString[2], 16));
            splittedString[1]  = new PrintfFormat("%5s").sprintf(splittedString[1]);
            splittedString[2]  = new PrintfFormat("%5s").sprintf(splittedString[2]);

            msgToSend = splittedString[0] + splittedString[1] + splittedString[2] ;
            outputStream.write(msgToSend.getBytes());


                    break;
            default:
                    System.out.println("Wrong argument count...");
            }
            
			outputStream.write('\r');
		    Thread.sleep(1000);
		    } catch (IOException e) {}
		    
        }
	    
	} catch (InterruptedException e) {}
    } 

    /**
     * Method declaration
     *
     *
     * @param event
     *
     * @see
     */
    public void serialEvent(SerialPortEvent event) {
	switch (event.getEventType()) {

	case SerialPortEvent.BI:

	case SerialPortEvent.OE:

	case SerialPortEvent.FE:

	case SerialPortEvent.PE:

	case SerialPortEvent.CD:

	case SerialPortEvent.CTS:

	case SerialPortEvent.DSR:

	case SerialPortEvent.RI:

	case SerialPortEvent.OUTPUT_BUFFER_EMPTY:
	    break;

	case SerialPortEvent.DATA_AVAILABLE:
	    byte[] readBuffer = new byte[30];

	    try {
		while (inputStream.available() > 0) {
		    int numBytes = inputStream.read(readBuffer);
		} 

		System.out.print("[Response]:");
		System.out.print(new String(readBuffer));

	    } catch (IOException e) {}

	    break;
	}
    } 

}



