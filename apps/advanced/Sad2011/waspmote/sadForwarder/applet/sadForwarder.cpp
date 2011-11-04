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
	SAD Data forwarder.
*/

#define ROUTER_MAC_ADDRESS "0013A200406F60AF"

#define I_AM_HERE_SEND_FREQ 10000	//ms
#define SEND_I_AM_HERE 1


#include "WProgram.h"
void setup();
bool isAcceptableDownstreamAddress(uint16_t address);
bool isAcceptableUpstreamAddress(uint16_t address);
void rxDataPacket(packetXBee* paq, uint16_t address);
void rxRoutingPacket(packetXBee* paq, uint16_t address);
void loop();
bool checkSeqNum(uint16_t addr, uint16_t seqNum);
void fwdData(packetXBee* paq_recv);
bool fwdRouting();
bool sendIAmHere();
bool fwdIAmHere(uint16_t *iAmHere);
int8_t setDestinationParams(packetXBee* paq, const char* address, uint8_t* data, uint16_t data_length, uint8_t type, uint8_t off_type);
void randomDoSeed(uint32_t seed);
uint32_t randomRand(void);
void randomDelay(uint16_t maxMillisec);
Neighbors_t nbrs[MAX_NEIGHBORS];
SadRoutingInfoPacket_t myRoutingInfo;
uint8_t nbrCount;
uint16_t localAddress;
uint32_t nextIAmHereSendTime = 0;
uint16_t IAmHereSeqNum = 1;

bool checkSeqNum(uint16_t addr, uint16_t seqNum);

int8_t setDestinationParams(packetXBee* paq, const char* address, uint8_t* data, uint16_t data_length, uint8_t type, uint8_t off_type);
bool fwdRouting();
bool sendIAmHere();
bool fwdIAmHere(uint16_t *iAmHere);

void randomDelay(uint16_t maxMillisec);

void randomDoSeed(uint32_t seed);
uint32_t randomRand(void);

void setup()
{
	// Inits the XBee 802.15.4 library
	xbee802.init(XBEE_802_15_4, FREQ2_4G, NORMAL);
	// Powers XBee
	xbee802.ON();
	xbee802.setChannel(15);
	uint8_t panid[2]={0x66,0x33};
	nbrCount = 0;
	memset(&myRoutingInfo, 0, sizeof(SadRoutingInfoPacket_t));
	xbee802.setPAN(panid); // Set PANID
	xbee802.getOwnMac();
	localAddress = (uint16_t)(xbee802.sourceMacLow[2]<<8) | (uint16_t)(xbee802.sourceMacLow[3]);
        randomDoSeed(localAddress);
	XBee.println("");
	XBee.print("Starting... ");
	XBee.print("Local Address: ");
	XBee.println(localAddress,HEX);
	XBee.println("");
}

bool isAcceptableDownstreamAddress(uint16_t address)
{
    if (address == 0x0) return true;

    switch (localAddress) {
    case 0x60DE:
      return address == 0x60FB || address == 0x6004;
    case 0x6004:
      return address == 0x60FB;
    case 0x60FB:
      return false;
   default:
      XBee.println("Strange local address!");
      return true;
    }
}

bool isAcceptableUpstreamAddress(uint16_t address)
{
    switch (localAddress) {
    case 0x60DE:
      return address == 0x60AF;
    case 0x6004:
      return address == 0x60DE || address == 0x60AF;
    case 0x60FB:
      return address == 0x60DE || address == 0x60AF || address == 0x6004;
   default:
      XBee.println("Strange local address!");
      return true;
    }
}

void rxDataPacket(packetXBee* paq, uint16_t address)
{
    if (isAcceptableDownstreamAddress(address)) {
        fwdData(paq);
        XBee.print("Forwarded packet for ");
        XBee.println(address, HEX);
    } else {
      	XBee.print("Dropped data packet from ");
        XBee.println(address, HEX);
    }
}

void rxRoutingPacket(packetXBee* paq, uint16_t address)
{
    if (isAcceptableUpstreamAddress(address)) {
	SadRoutingInfoPacket_t ri;
        memcpy(&ri, paq->data, sizeof(ri));
        
        XBee.print("got routing info, rootClock: ");
        XBee.println(ri.rootClock);
        
	if (!seqnumAfter(ri.seqnum, myRoutingInfo.seqnum) && seqnumAfter(ri.seqnum, myRoutingInfo.seqnum - POSSIBLE_SEQNUM_OFFSET)) {
		XBee.print("Dropping old routing info.. mine: ");
		XBee.print(myRoutingInfo.seqnum,DEC);
		XBee.print(" , got: ");
		XBee.println(ri.seqnum,DEC);
	} else{
		memcpy(&myRoutingInfo, &ri, sizeof(ri));
	        fwdRouting();
        }
    } else {
      	XBee.print("Dropped routing packet from ");
        XBee.println(address, HEX);
    }
}

void loop()
{ 
	// Waiting the answer
	if( XBee.available() )
	{ 
		xbee802.treatData();
		if( !xbee802.error_RX )
		{
			while(xbee802.pos>0)
			{	
                                packetXBee* paq_recv = xbee802.packet_finished[xbee802.pos-1];
				uint16_t addr = (uint16_t)(paq_recv->macSL[2]<<8) | (uint16_t)(paq_recv->macSL[3]);
          			uint16_t orgAddr = (uint16_t)(paq_recv->macOL[2]<<8) | (uint16_t)(paq_recv->macOL[3]);

                                switch (paq_recv->packetID) {
                                case SAD_I_AM_HERE_ID:
                                {
					uint16_t newIAmHere[2];
					memcpy(&newIAmHere, xbee802.packet_finished[xbee802.pos-1]->data, sizeof(uint16_t)*2);
					if (checkSeqNum(addr, newIAmHere[0]) && orgAddr != localAddress) {
						newIAmHere[1]++;
						fwdIAmHere((uint16_t *)&newIAmHere);
					} else{
						XBee.print("Dropping iAmHere from ");
						XBee.println(orgAddr,HEX);
					}
                                	break;
                                }
                                case SAD_DATA_ID:
                                	rxDataPacket(paq_recv, addr);
                                	break;
                                case SAD_ROUTING_ID:
                                	rxRoutingPacket(paq_recv, addr);
                                	break;
				}
				//fwdData(xbee802.packet_finished[xbee802.pos-1]);
				free(xbee802.packet_finished[xbee802.pos-1]);
				xbee802.packet_finished[xbee802.pos-1]=NULL;
				xbee802.pos--;
			}
		}
	}
	if (nextIAmHereSendTime <= millis()) {
		if (SEND_I_AM_HERE){
			sendIAmHere();
		}
	}
	//delay(2000);
}

bool checkSeqNum(uint16_t addr, uint16_t seqNum)
{
	for (uint8_t i = 0; i < nbrCount; i++){
		if (nbrs[i].addr == addr){
  			if (!seqnumAfter(seqNum, nbrs[i].seqNum) && seqnumAfter(seqNum, nbrs[i].seqNum - POSSIBLE_SEQNUM_OFFSET)) {
				XBee.print("Dropping - addr:");
				XBee.print(addr,HEX);
				XBee.print(" seqNum:");
				XBee.println(seqNum,DEC);
				return 0;
			} else {
				XBee.print("Akcepting - addr:");
				XBee.print(addr,HEX);
				XBee.print(" seqNum:");
				XBee.println(seqNum,DEC);
				XBee.print(" old seqNum:");
				XBee.println(nbrs[i].seqNum,DEC);
				nbrs[i].seqNum = seqNum;
				return 1;
			}
		}
	}
	// If we got so far it means that there's no such neighbor, let's add him 
	if (nbrCount < MAX_NEIGHBORS) {
		nbrs[nbrCount].addr = addr;
		nbrs[nbrCount++].seqNum = seqNum;
		XBee.print("Adding - addr:");
		XBee.print(addr,HEX);
		XBee.print(" seqNum:");
		XBee.println(seqNum,DEC);
		return 1;
	} 
	return 0;
}

void fwdData(packetXBee* paq_recv)
{
	DataPacket_t data;
        memcpy(&data, paq_recv->data, sizeof(DataPacket_t));

	XBee.print("MAC Address Source: ");          
	for(int c=0;c<4;c++)
	{
		XBee.print(paq_recv->macSL[c],HEX);
	}
	for(int c=0;c<4;c++)
	{
		XBee.print(paq_recv->macSH[c],HEX);
	}
	XBee.println("");

        if (!checkSeqNum(data.sourceAddress, data.dataSeqnum)){
            XBee.println("already seen this data packet!");
            return;
        }
        
        randomDelay(100);

	packetXBee* paq_sent;
	paq_sent=(packetXBee*) calloc(1,sizeof(packetXBee)); 
	paq_sent->mode=BROADCAST;
	paq_sent->MY_known=0;
	paq_sent->packetID = SAD_DATA_ID;
	paq_sent->opt=0; 
	xbee802.hops=0;

        // XXX: set local MAC address the as origin address
        xbee802.setOriginParams(paq_sent, "", MAC_TYPE); 

	setDestinationParams(paq_sent, ROUTER_MAC_ADDRESS, (uint8_t *)paq_recv->data, paq_recv->data_length, MAC_TYPE, DATA_ABSOLUTE);
	//if (haveOrigin)
	XBee.print("********************** Forwarding packet for ");
	XBee.println((uint16_t)(paq_recv->macSL[2]<<8) | (uint16_t)(paq_recv->macSL[3]),HEX);
	xbee802.sendXBee(paq_sent);
	if( !xbee802.error_TX )
	{
		XBee.println("ok");
	}
	free(paq_sent);
	paq_sent=NULL;
}

bool fwdRouting() {
	SadRoutingInfoPacket_t ri;
	memcpy(&ri, &myRoutingInfo, sizeof(ri));
	ri.hopCount++;

        randomDelay(100);
	
	packetXBee* paq_sent = (packetXBee*) calloc(1, sizeof(packetXBee));
	paq_sent->mode=BROADCAST;
	paq_sent->MY_known=0;
	paq_sent->packetID = SAD_ROUTING_ID;
	paq_sent->opt=0;
	xbee802.hops=0;
	xbee802.setOriginParams(paq_sent, "", MAC_TYPE);
	setDestinationParams(paq_sent, "000000000000FFFF", (uint8_t *)&ri, sizeof(ri), MAC_TYPE, DATA_ABSOLUTE);

	xbee802.sendXBee(paq_sent);
	if (xbee802.error_TX)
	{
		XBee.println("TX failed!");
	}
	else {
 		XBee.print("\nsize=");
  		XBee.print(sizeof(SadRoutingInfoPacket_t));
		XBee.print(" Routing packet forwarded, seqNum: ");
		XBee.println(ri.seqnum, DEC);
	}
	free(paq_sent);
	paq_sent=NULL;
	return true;
}

bool sendIAmHere(){
	uint16_t newIAmHere[2]={IAmHereSeqNum++,1};
	
	packetXBee* paq_sent = (packetXBee*) calloc(1, sizeof(packetXBee));
	paq_sent->mode=BROADCAST;
	paq_sent->MY_known=0;
	paq_sent->packetID = SAD_I_AM_HERE_ID;
	paq_sent->opt=0;
	xbee802.hops=0;
	xbee802.setOriginParams(paq_sent, "", MAC_TYPE);
	setDestinationParams(paq_sent, "000000000000FFFF", (uint8_t *)&newIAmHere, sizeof(uint16_t)*2, MAC_TYPE, DATA_ABSOLUTE);

	xbee802.sendXBee(paq_sent);
	if (xbee802.error_TX)
	{
		XBee.println("TX failed!");
	}
	else {
		XBee.print("I Am Here sent, seqNum: ");
		XBee.println(newIAmHere[0], DEC);
	}
	free(paq_sent);
	paq_sent=NULL;
	nextIAmHereSendTime = millis() + I_AM_HERE_SEND_FREQ;
	return true;
}

bool fwdIAmHere(uint16_t *iAmHere){
	packetXBee* paq_sent = (packetXBee*) calloc(1, sizeof(packetXBee));
	paq_sent->mode=BROADCAST;
	paq_sent->MY_known=0;
	paq_sent->packetID = SAD_I_AM_HERE_ID;
	paq_sent->opt=0;
	xbee802.hops=0;
	xbee802.setOriginParams(paq_sent, "", MAC_TYPE);
	for(uint8_t a=0;a<4;a++)
	{
		paq_sent->macOL[a]=xbee802.packet_finished[xbee802.pos-1]->macOL[a];
		paq_sent->macOH[a]=xbee802.packet_finished[xbee802.pos-1]->macOH[a+4];
	}
	setDestinationParams(paq_sent, "000000000000FFFF", (uint8_t *)iAmHere, sizeof(uint16_t)*2, MAC_TYPE, DATA_ABSOLUTE);

	xbee802.sendXBee(paq_sent);
	if (xbee802.error_TX)
	{
		XBee.println("TX failed!");
	}
	else {
		XBee.print("I Am Here forwarded, seqNum: ");
		XBee.print(iAmHere[0], DEC);
		XBee.print(" hops: ");
		XBee.println(iAmHere[1], DEC);
	}
	free(paq_sent);
	paq_sent=NULL;
	return true;
}

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

static uint32_t rngSeed;

void randomDoSeed(uint32_t seed)
{
    rngSeed = seed;
}

uint32_t randomRand(void)
{
    rngSeed = rngSeed * 1664525ul + 1013904223ul;
    return rngSeed;
}

void randomDelay(uint16_t maxMillisec)
{
  delay((randomRand() >> 16) % maxMillisec);
}



int main(void)
{
	init();

	setup();
    
	for (;;)
		loop();
        
	return 0;
}

