#include <stdmansos.h>

//PORT1
#define MuxEn 		0x01
#define MuxSel1 	0x02
#define MuxSel2		0x04
#define DeMuxSel	0x08	//Pie DeMuxSel = 0, Mux izvçlas MSP nevis XPort
#define DeMuxEn		0x10
#define MSPRTS		0x20
#define RST			0x40
#define CTS			0x80
//PORT2
#define	MSPDTR		0x01
#define	XPortRTS	0x02
#define	XPortDTR	0x04
#define	LED1		0x10
#define	LED2		0x20
// //Mainiigie
// unsigned int temp = 0;
// unsigned short int result = 0;

#define MUX_SEL_A  2
#define MUX_SEL_B  1

#define RTS_IN_PORT  2
#define RTS_IN_PIN   1

#define CTS_IN_PORT  2
#define CTS_IN_PIN   2

#define RTS_OUT_PORT 1
#define RTS_OUT_PIN  6

#define CTS_OUT_PORT 1
#define CTS_OUT_PIN  7

#define LED_PORT 2
#define LED1_PIN 4
#define LED2_PIN 5

#define MAGIC_KEY   0xD19A

// module IDs
enum {
    SIGNAL_MODULE = 1,
    POWER_MODULE,
    MOTE_MODULE, // not a module, but the attached mote
};

static void enableModule(uint8_t module)
{
    switch (module) {
    case SIGNAL_MODULE:
        // 0XY
        pinClear(1, MUX_SEL_A);
        pinClear(1, MUX_SEL_B);

        pinClear(LED_PORT, LED1_PIN);
        pinClear(LED_PORT, LED2_PIN);
        break;
    case POWER_MODULE:
        // 1XY
        pinSet(1, MUX_SEL_A);
        pinClear(1, MUX_SEL_B);

        pinSet(LED_PORT, LED1_PIN);
        pinClear(LED_PORT, LED2_PIN);
        break;
    case MOTE_MODULE:
        // 2XY
        pinClear(1, MUX_SEL_A);
        pinSet(1, MUX_SEL_B);

        pinClear(LED_PORT, LED1_PIN);
        pinSet(LED_PORT, LED2_PIN);
        break;
    }
}

XISR(RTS_IN_PORT, myInterrupt)
{
    if (!pinReadIntFlag(RTS_IN_PORT, RTS_IN_PIN)) {
        // some other port 2 interrupt

        if (pinReadIntFlag(CTS_IN_PORT, CTS_IN_PIN)) {
           // copy to output
           pinWrite(CTS_OUT_PORT, CTS_OUT_PIN, pinRead(CTS_IN_PORT, CTS_IN_PIN));
           pinClearIntFlag(CTS_IN_PORT, CTS_IN_PIN);
        }
        return;
    }

    static enum {READ_KEY, READ_ADDR1, READ_ADDR2} state = READ_KEY;

    uint8_t bit = pinRead(CTS_IN_PORT, CTS_IN_PIN); // read data
    pinClearIntFlag(RTS_IN_PORT, RTS_IN_PIN);

    // copy to output
    pinWrite(RTS_OUT_PORT, RTS_OUT_PIN, pinRead(RTS_IN_PORT, RTS_IN_PIN));

    static uint16_t result;
    result = (result << 1) | bit;

    switch (state) {
    case READ_KEY:
        if (result == MAGIC_KEY) {
            state = READ_ADDR1;
            result = 0; // reset
        }
        break;

    case READ_ADDR1:
        state = READ_ADDR2;
        break;

    case READ_ADDR2:
        // at this point, 'result' contains address (ID) of the module. Use it!
        enableModule(result);
        result = 0; // reset
        state = READ_KEY;
        break;
    }
}

void main(void)
{
    P1DIR |= MuxEn + MuxSel1 + MuxSel2 + DeMuxSel + DeMuxEn + MSPRTS; // RST un CTS veel nelietoju.
    P1OUT |= MSPRTS;// + DeMuxSel;// + MuxSel1 + MuxSel2;
    P2DIR |= MSPDTR + LED1 + LED2;
    P2OUT |= MSPDTR;

    pinAsOutput(RTS_OUT_PORT, RTS_OUT_PIN);
    pinAsOutput(CTS_OUT_PORT, CTS_OUT_PIN);

    pinEnableInt(RTS_IN_PORT, RTS_IN_PIN);
    pinIntRising(RTS_IN_PORT, RTS_IN_PIN);

    pinEnableInt(CTS_IN_PORT, CTS_IN_PIN);
    pinIntRising(CTS_IN_PORT, CTS_IN_PIN);
}

// void main(void)
// {
// 	WDTCTL=WDTPW+WDTHOLD;      			 	//STOP WATCHDOG TIMER

// 	P1DIR |= MuxEn + MuxSel1 + MuxSel2 + DeMuxSel + DeMuxEn + MSPRTS; // RST un CTS veel nelietoju.
// 	P1OUT |= MSPRTS;// + DeMuxSel;// + MuxSel1 + MuxSel2;
// 	P2DIR |= MSPCTS + LED1 + LED2;
// 	P2OUT |= MSPCTS;

// 	P1IE |= XPortRTS;
// 	P1IES&=~XPortRTS;

// 	_EINT();
// }

// #pragma vector=PORT2_VECTOR
// __interrupt void P2(void)
// {
// 	{
// 		if ((P2IN & 0x04) == 0x04)
// 			temp = 1;
// 		else
// 			temp = 0;
// 	}

// 		//result = (result >> 1) + (temp << 7); LSB first
// 		result = (result << 1) + temp;		//AIZIET DEBESIIS!!!
// 		//Ierobezho izveeloties unsigned int short ja 16 biti. Unsigned char, ja 8 biti.
// 		//MSB first

// 		switch(result) //parasti lietoju 41891, 41899, 41907, 41915. 16bit tomeer labaak.
// 			{

// 				case 163:
// 					P2OUT = 0x00;
// 					result = 0;
// 					break;

// 				case 171://41899:
// 					P2OUT = 0x10;
// 					result = 0;
// 					break;

// 				case 179:
// 					P2OUT = 0x20;
// 					result = 0;
// 					break;

// 				case 187:
// 					P2OUT = 0x30;
// 					result = 0;
// 					break;

// 			}
// 	P1IFG&=~BIT1;               			//CLEAR THE INTERRUPT FLAG FOR PIN1
// }
