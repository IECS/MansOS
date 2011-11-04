/**
* Copyright (c) 2008-2010 Leo Selavo and the contributors. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*  * Redistributions of source code must retain the above copyright notice,
*    this list of  conditions and the following disclaimer.
*  * Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "/home/atis/work/mansos/apps/sandbox/Sad2011/data_packet.h"

/*
	SAD Base station.
*/

#define ROUTING_SEND_FREQ 5000	//ms

#define SEND_ROUTING_INFO 1

#define SERIAL_PORT 0

Neighbors_t nbrs[MAX_NEIGHBORS];
uint8_t nbrCount = 0;
uint16_t localAddress;
uint32_t nextRoutingSendTime = 0;
uint16_t myRoutingSeqNum = 1;
uint16_t iAmHere[2];

uint32_t lastRootSyncJiffies;
uint32_t lastRootClockSeconds;

bool checkSeqnum(uint16_t addr, uint8_t seqNum);
void sendRoutingInfo();
// Modified setDestionationParams for binary data sending
int8_t setDestinationParams(packetXBee* paq, const char* address, uint8_t* data, uint16_t data_length, uint8_t type, uint8_t off_type);

void usartReceive(uint8_t byte);

void setup()
{
	// Inits the XBee 802.15.4 library
	xbee802.init(XBEE_802_15_4, FREQ2_4G, NORMAL);
	// Powers XBee
	xbee802.ON();
	xbee802.setChannel(15);
	uint8_t  panid[2]={0x66,0x33};
	nbrCount = 0;
	xbee802.setPAN(panid); // Set PANID
	xbee802.getOwnMac();
	localAddress = (uint16_t)(xbee802.sourceMacLow[2]<<8) | (uint16_t)(xbee802.sourceMacLow[3]);
	XBee.println("");
	XBee.print("Starting... ");
	XBee.print("Local Address: ");
	XBee.println(localAddress,HEX);
	XBee.println("");
}

void loop()
{
	uint8_t i;
	if( XBee.available() )
	{ 
		xbee802.treatData();
		if( !xbee802.error_RX )
		{ 
			// Writing the parameters of the packet received
			while(xbee802.pos>0)
			{
                                packetXBee* paq_recv = xbee802.packet_finished[xbee802.pos-1];
                                switch (paq_recv->packetID) {
				case SAD_I_AM_HERE_ID:
					memcpy(&iAmHere, paq_recv->data, sizeof(uint16_t)*2);
					XBee.print("IAH from ");
					XBee.print((uint16_t)(paq_recv->macOL[2]<<8) | (uint16_t)(paq_recv->macOL[3]),HEX);
					XBee.print(" hops:");
					XBee.print(iAmHere[1],DEC);
					XBee.print(" seq:");
					XBee.println(iAmHere[0],DEC);
                                        break;
                                case SAD_ROUTING_ID:
				    //XBee.println("Dropped routing info!");
                                    break;
                                case SAD_DATA_ID: {
  //					uint16_t addr = (uint16_t)(xbee802.packet_finished[xbee802.pos-1]->macSL[2]<<8) | (uint16_t)(xbee802.packet_finished[xbee802.pos-1]->macSL[3]);
//					if (addr == 0x60DE){
//						uint16_t addr = (uint16_t)(xbee802.packet_finished[xbee802.pos-1]->macOH[2]<<8) | (uint16_t)(xbee802.packet_finished[xbee802.pos-1]->macOH[3]);
//						uint16_t srcAddr = (uint16_t)(xbee802.packet_finished[xbee802.pos-1]->macSL[2]<<8) | (uint16_t)(xbee802.packet_finished[xbee802.pos-1]->macSL[3]);
//						XBee.println("****************************** GOT DATA PACKET");

                                          	DataPacket_t data;
						memcpy(&data, paq_recv->data, sizeof(DataPacket_t));
                                                if (!checkSeqnum(data.sourceAddress, data.dataSeqnum)) {
     						    XBee.println("duplicate DATA PACKET");
                                                    break;
                                                }
                                                
                                                XBee.println("DATA PACKET");

//						uint16_t naO;
						uint32_t macOH, macOL;
						memcpy(&macOH, paq_recv->macOH, sizeof(macOH));
						memcpy(&macOL, paq_recv->macOL, sizeof(macOL));
//						memcpy(&naO, xbee802.packet_finished[xbee802.pos-1]->naO, sizeof(naO));

//						XBee.print("****************************** Source address: ");
//						XBee.println(srcAddr,HEX);

//						XBee.print("****************************** Origin Address: ");
//						XBee.print(macOH, HEX);
//						XBee.print(" ");
//						XBee.println(macOL, HEX);
//						XBee.print("****************************** Network Origin: ");
//						XBee.println(naO, HEX);
						
//						memcpy(&data, xbee802.packet_finished[xbee802.pos-1]->data,sizeof(DataPacket_t));
//						if (checkSeqNum(addr, data.dataSeqnum)) {

#if 0
  						XBee.print("timestamp: ");
						XBee.println(data.timestamp,HEX);
						XBee.print("sourceAddress: ");
						XBee.println(data.sourceAddress,HEX);
						XBee.print("dataSeqnum: ");
						XBee.println(data.dataSeqnum,HEX);
						XBee.print("islLight: ");
						XBee.println(data.islLight,HEX);
						XBee.print("apdsLight0: ");
						XBee.println(data.apdsLight0,HEX);
						XBee.print("apdsLight1: ");
						XBee.println(data.apdsLight1,HEX);
						XBee.print("sq100Light: ");
						XBee.println(data.sq100Light,HEX);
						XBee.print("internalVoltage: ");
						XBee.println(data.internalVoltage,HEX);
						XBee.print("internalTemperature: ");
						XBee.println(data.internalTemperature,HEX);
						XBee.print("sht75Humidity: ");
						XBee.println(data.sht75Humidity,HEX);
						XBee.print("sht75Temperature: ");
						XBee.println(data.sht75Temperature,HEX);
						XBee.print("crc: ");
						XBee.println(data.crc,HEX);
#endif

                  // print the packet in parsable format
                  char buffer[100];
                  sprintf(buffer, "$%x %lx %x %x %x %x %x %x %x %x^",
                    data.sourceAddress,
                    data.timestamp,
                    data.islLight,
                    data.apdsLight0,
                    data.apdsLight1,
                    data.sq100Light,
                    data.internalVoltage,
                    data.internalTemperature,
                    data.sht75Humidity,
                    data.sht75Temperature);
            XBee.print(buffer);
            uint16_t calcCrc = crc16((uint8_t *)buffer, strlen(buffer));
            sprintf(buffer, "%x\n", calcCrc);
            XBee.print(buffer);

                                }
                                    break;
                                default:
                                    XBee.print("got packet with type: ");
                	  	    XBee.println(paq_recv->packetID, HEX);
                                    break;
                                }
				free(xbee802.packet_finished[xbee802.pos-1]);
				xbee802.packet_finished[xbee802.pos-1]=NULL;
				xbee802.pos--;
			}
		}
	}

#if 0 // FIXME? not working!
        while (serialAvailable(SERIAL_PORT) > 0) {
            int rb = serialRead(SERIAL_PORT);
            if (rb == -1) break;
            usartReceive(rb);
        }
#endif

	if (nextRoutingSendTime <= millis()){
		if (SEND_ROUTING_INFO){
			sendRoutingInfo();
		}
	}
	//delay(2000);
}

bool checkSeqnum(uint16_t addr, uint16_t seqNum) {
	for (uint8_t i = 0; i < nbrCount; i++){
		if (nbrs[i].addr == addr){
			if (!seqnumAfter(seqNum, nbrs[i].seqNum) && seqnumAfter(seqNum, nbrs[i].seqNum - POSSIBLE_SEQNUM_OFFSET)) {
				//XBee.print("Dropping - addr:");
				//XBee.print(addr,HEX);
				//XBee.print(" seqNum:");
				//XBee.println(seqNum,HEX);
				return 0;
			} else {
				nbrs[i].seqNum = seqNum;
				//XBee.print("Akcepting - addr:");
				//XBee.print(addr,HEX);
				//XBee.print(" seqNum:");
				//XBee.println(seqNum,HEX);
				return 1;
			}
		}
	}
	// If we got so far it means that there's no such neighbor, let's add him 
	if (nbrCount < MAX_NEIGHBORS) {
		nbrs[nbrCount].addr = addr;
		nbrs[nbrCount++].seqNum = seqNum;
		XBee.print("Adding neighbor ");
		XBee.print(addr,HEX);
		XBee.print(" seqNum:");
		XBee.println(seqNum,HEX);
		return 1;
	} 
	return 0;
}

void sendRoutingInfo()
{
	SadRoutingInfoPacket_t ri;
	ri.packetType = ROUTING_INFORMATION;
	ri.__reserved = 0x0;
	ri.rootAddress = localAddress;
	ri.hopCount = 1;
	ri.seqnum = myRoutingSeqNum++;
        if (lastRootSyncJiffies) {
            ri.rootClock = lastRootClockSeconds + (millis() - lastRootSyncJiffies);
        } else {
            ri.rootClock = millis();
        }

//        XBee.print("send routing info, rootClock: ");
//        XBee.println(ri.rootClock);

	packetXBee* paq_sent = (packetXBee*) calloc(1, sizeof(packetXBee));
	paq_sent->mode=BROADCAST;
	paq_sent->MY_known=0;
	paq_sent->packetID = SAD_ROUTING_ID;
	paq_sent->opt=0;
	xbee802.hops=0;
	xbee802.setOriginParams(paq_sent, "", MAC_TYPE);
	setDestinationParams(paq_sent, "000000000000FFFF", (uint8_t *)&ri, sizeof(SadRoutingInfoPacket_t), MAC_TYPE, DATA_ABSOLUTE);

	xbee802.sendXBee(paq_sent);
	if (xbee802.error_TX)
	{
		XBee.println("TX failed!");
	}
	else {
		// XBee.println("\nRouting packet sent");
	}
	free(paq_sent);
	paq_sent=NULL;
	nextRoutingSendTime = millis() + ROUTING_SEND_FREQ;
}

// Modified setDestionationParams for binary data sending
int8_t setDestinationParams(packetXBee* paq, const char* address, uint8_t* data, uint16_t data_length, uint8_t type, uint8_t off_type)
{
	uint8_t* destination = (uint8_t*) calloc(8,sizeof(uint8_t));
	if( destination==NULL ) return -1;
	uint8_t i=0;
	uint8_t j=0;
	uint16_t data_ind;
	char aux[2];
	
	if( off_type==DATA_ABSOLUTE )
	{
		if( type==MAC_TYPE )
		{
			while(j<8)
			{
				aux[i-j*2]=address[i];
				aux[(i-j*2)+1]=address[i+1];
				destination[j]=Utils.str2hex(aux);
				i+=2;
				j++;
			}
			for(uint8_t a=0;a<4;a++)
			{
				paq->macDH[a]=destination[a];
			}
			for(uint8_t b=0;b<4;b++)
			{
				paq->macDL[b]=destination[b+4];
			}
			paq->address_type=_64B;
		}
		if( type==MY_TYPE )
		{
			while(j<2)
			{
				aux[i-j*2]=address[i];
				aux[(i-j*2)+1]=address[i+1];
				destination[j]=Utils.str2hex(aux);
				i+=2;
				j++;
			}
			paq->naD[0]=destination[0];
			paq->naD[1]=destination[1];
			paq->address_type=_16B;
		}
		data_ind=0;
	}
	for (i=0;i<data_length;i++)
	{
		paq->data[data_ind]=*data++;
		data_ind++;
		if( data_ind>=MAX_DATA ) break;
	}
	paq->data_length=data_ind;
	free(destination);
	return 1;
}

static inline uint16_t crc16Add(uint16_t acc, uint8_t byte) {
    acc ^= byte;
    acc  = (acc >> 8) | (acc << 8);
    acc ^= (acc & 0xff00) << 4;
    acc ^= (acc >> 8) >> 4;
    acc ^= (acc & 0xff00) >> 5;
    return acc;
}

uint16_t crc16(const uint8_t *data, uint16_t len) {
    uint16_t i, crc = 0;
    for (i = 0; i < len; ++i) {
        crc = crc16Add(crc, *data++);
    }
    return crc;
}

void onUsartDataRecvd(uint8_t *data) {
    struct {
        unsigned char delimiter1;
        unsigned char delimiter2;
        unsigned short crc;
        unsigned long time;
    } s;
    
#define DELIMITER '$'

    memcpy(&s, data, sizeof(s));
    if (s.delimiter1 != DELIMITER || s.delimiter2 != 0) {
        XBee.print("onUsartDataRecvd: wrong delimiters!\n");
        return;
    }
    uint16_t crc = crc16((uint8_t *) &s.time, sizeof(s.time));
    if (crc != s.crc) {
        XBee.print("onUsartDataRecvd: wrong crc\n");
        return;
    }

    char buffer[100];
    sprintf(buffer, "rx time %lu via serial\n", s.time);
    XBee.print(buffer);
    
    // change the system time used in routing protocol
    lastRootClockSeconds = s.time;
    lastRootSyncJiffies = millis();
}

void usartReceive(uint8_t byte) {
    static uint8_t recvBuffer[8];
    static int bytesAfterDelimiter = -1;
    
    XBee.print("\nSerial Rx: ");
    XBee.println(byte);

    if (bytesAfterDelimiter == -1) {
        if (byte == DELIMITER) bytesAfterDelimiter = 0;
        else return;
    }
  
    recvBuffer[bytesAfterDelimiter++] = byte;
    if (bytesAfterDelimiter > sizeof(recvBuffer)) {
        bytesAfterDelimiter = -1;
        onUsartDataRecvd(recvBuffer);
    }
}

