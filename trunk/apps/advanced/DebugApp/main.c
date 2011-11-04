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

//-------------------------------------------
//      DebugApp application.
//-------------------------------------------
//  Setting & Getting Byte via Serial Port


#include "mansos.h"
#include "leds.h"
#include "usart.h"
#include <msp430x16x.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include <udelay.h>

unsigned char buff[20];
unsigned char tmpbuff[20];

unsigned char readBuff[20];
unsigned char readBuffCaret = 0;


//-------------------------------------------
//      Handling UART incoming data
//-------------------------------------------
interrupt ( UART1RX_VECTOR ) IntServiceRoutine(void)
{
    uint8_t *pData;

    if ( (U1RXBUF != '\r') && (readBuffCaret<20) )
    {
        readBuff[readBuffCaret] = U1RXBUF;
        readBuffCaret++;

    }


    if ( U1RXBUF == '\r' )
    {
        switch ( readBuff[0]){
            case 'g':
                        tmpbuff[0] = readBuff[1];
                        tmpbuff[1] = readBuff[2];
                        tmpbuff[2] = readBuff[3];
                        tmpbuff[3] = readBuff[4];
                        tmpbuff[4] = readBuff[5];
                        tmpbuff[5] = '\0';

                        pData =  atoi(tmpbuff);

                        //sprintf( buff, "%p, %s" , pData, itoa(*pData, tmpbuff,2) );
                        sprintf( buff, "%p, %s" , pData, tmpbuff );
                        USARTSendStringLine(1, buff);

                        break;
            case 's':
                        tmpbuff[0] = readBuff[1];
                        tmpbuff[1] = readBuff[2];
                        tmpbuff[2] = readBuff[3];
                        tmpbuff[3] = readBuff[4];
                        tmpbuff[4] = readBuff[5];
                        tmpbuff[5] = '\0';

                        pData =  atoi(tmpbuff);

                        tmpbuff[0] = readBuff[6];
                        tmpbuff[1] = readBuff[7];
                        tmpbuff[2] = readBuff[8];
                        tmpbuff[3] = readBuff[9];
                        tmpbuff[4] = readBuff[10];
                        tmpbuff[5] = '\0';

                        *pData =  atoi(tmpbuff);

                        break;
            default:    // something else - do nothing
        }

        readBuffCaret = 0;
    }

//    return U1RXBUF;
}


//-------------------------------------------
//      Entry point for the application
//-------------------------------------------
void appMain(void)
{
    USARTInit( 1, 115200, 0);
    USARTEnableTX(1);
    USARTEnableRX(1);

    // Enable global interrupts
    eint();
    // Enable rx interrupt
    IE2 |= (URXIE1);

    while(1)
    {
        busyWait(500000);
    }
}
