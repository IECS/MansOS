#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include "atmega_2wire.h"


#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


static volatile uint8_t twiState;
static uint8_t twiSlarw;

//static void (*twi_onSlaveTransmit)();
//static void (*twi_onSlaveReceive)(uint8_t*, int);

static uint8_t* twiMasterBuffer = 0;
static volatile uint8_t twiMasterBufferIndex = 0;
static uint8_t twiMasterBufferLength = 0;

//static uint8_t* twi_txBuffer;
//static volatile uint8_t twi_txBufferIndex;
//static volatile uint8_t twi_txBufferLength;

//static uint8_t* twi_rxBuffer;
//static volatile uint8_t twi_rxBufferIndex;

static volatile uint8_t twiError;

/* 
 * Function twiInit
 * Desc     readys twi pins and sets twi bitrate
 * Input    none
 * Output   none
 */
void twiInit()
{
  // initialize state
  twiState = TWI_READY;

  // #if defined(__AVR_ATmega168__) || defined(__AVR_ATmega8__) || defined(__AVR_ATmega328P__)
  //   // activate internal pull-ups for twi
  //   // as per note from atmega8 manual pg167
    sbi(PORTC, 4);
    sbi(PORTC, 5);
  // #else
  //   // activate internal pull-ups for twi
  //   // as per note from atmega128 manual pg204
  //   sbi(PORTD, 0);
  //   sbi(PORTD, 1);
  // #endif

  // initialize twi prescaler and bit rate
  cbi(TWSR, TWPS0);
  cbi(TWSR, TWPS1);
  TWBR = ((CPU_HZ / TWI_SPEED) - 16) / 2;

  /* twi bit rate formula from atmega128 manual pg 204
  SCL Frequency = CPU Clock Frequency / (16 + (2 * TWBR))
  note: TWBR should be 10 or higher for master mode
  It is 72 for a 16mhz Wiring board with 100kHz TWI */

  // enable acks
  // MOD: do not enable the module implicitly
  TWCR = _BV(TWEA);
  // TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA);

}

/**
 * Enable the I2C bus
 */
void twiOn(void) {
    TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA);
}

/**
 * Disable the I2C bus
 */
void twiOff(void) {
    TWCR = _BV(TWEA);
}

/* 
 * Function twi_readFrom
 * Desc     attempts to become twi bus master and read a
 *          series of bytes from a device on the bus
 * Input    address: 7bit i2c device address
 *          data: pointer to byte array
 *          length: number of bytes to read into array
 * Output   number of bytes read
 */
uint8_t twiRead(uint8_t addr, void *data, uint8_t len)
{
  // ensure data will fit into buffer
  if (TWI_BUFFER_LENGTH < len) {
    return 0;
  }

  // wait until twi is ready, become master receiver
  while (TWI_READY != twiState) {
    continue;
  }
  twiState = TWI_MRX;
  // reset error state (0xFF.. no error occurred)
  twiError = 0xFF;

  // initialize buffer iteration vars
  twiMasterBufferIndex = 0;
  twiMasterBufferLength = len - 1;  // This is not intuitive, read on...
  // On receive, the previously configured ACK/NACK setting is transmitted in
  // response to the received byte before the interrupt is signaled.
  // Therefore we must actually set NACK when the _next_ to last byte is
  // received, causing that NACK to be sent in response to receiving the last
  // expected byte of data.

  // build sla+w, slave device address + w bit
  twiSlarw = TW_READ;
  twiSlarw |= addr << 1;

  // MOD: set twiMasterBuffer to data address
  twiMasterBuffer = data;

  // send start condition
  TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA) | _BV(TWINT) | _BV(TWSTA);

  // wait for read operation to complete
  while (TWI_MRX == twiState) {
    continue;
  }

  if (twiMasterBufferIndex < len)
    len = twiMasterBufferIndex;

  return len;
}

/* 
 * Function twiWriteTo
 * Desc     attempts to become twi bus master and write a
 *          series of bytes to a device on the bus
 * Input    address: 7bit i2c device address
 *          data: pointer to byte array
 *          length: number of bytes in array
 *          wait: boolean indicating to wait for write or not
 * Output   0 .. success
 *          1 .. length to long for buffer
 *          2 .. address send, NACK received
 *          3 .. data send, NACK received
 *          4 .. other twi error (lost bus arbitration, bus error, ..)
 */
uint8_t twiWrite(uint8_t addr, const void *data, uint8_t len,
        uint8_t wait)
{
  // ensure data will fit into buffer
  if (TWI_BUFFER_LENGTH < len) {
    return I2C_OTHER;
  }

  // wait until twi is ready, become master transmitter
  while (TWI_READY != twiState) {
    continue;
  }
  twiState = TWI_MTX;
  // reset error state (0xFF.. no error occured)
  twiError = 0xFF;

  // initialize buffer iteration vars
  twiMasterBufferIndex = 0;
  twiMasterBufferLength = len;
  
  twiMasterBuffer = (void *) data;
  
  // build sla+w, slave device address + w bit
  twiSlarw = TW_WRITE;
  twiSlarw |= addr << 1;
  
  // send start condition
  TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA) | _BV(TWINT) | _BV(TWSTA);

  // wait for write operation to complete
  while (wait && (TWI_MTX == twiState)) {
    continue;
  }
  
  if (twiError == 0xFF)
    return 0;       // success
  else if (twiError == TW_MT_SLA_NACK)
    return I2C_ACK; // error: address send, nack received
  else if (twiError == TW_MT_DATA_NACK)
    return I2C_ACK; // error: data send, nack received
  
  // else
  return I2C_OTHER;  // other twi error
}

/* 
 * Function twi_transmit
 * Desc     fills slave tx buffer with data
 *          must be called in slave tx event callback
 * Input    data: pointer to byte array
 *          length: number of bytes in array
 * Output   1 length too long for buffer
 *          2 not slave transmitter
 *          0 ok
 */
//uint8_t twi_transmit(uint8_t* data, uint8_t length)
//{
//  uint8_t i;
//
//  // ensure data will fit into buffer
//  if (TWI_BUFFER_LENGTH < length) {
//    return 1;
//  }
//
//  // ensure we are currently a slave transmitter
//  if (TWI_STX != twiState) {
//    return 2;
//  }
//
//  // set length and copy data into tx buffer
//  twi_txBufferLength = length;
//  for(i = 0; i < length; ++i) {
//    twi_txBuffer[i] = data[i];
//  }
//
//  return 0;
//}

/* 
 * Function twi_attachSlaveRxEvent
 * Desc     sets function called before a slave read operation
 * Input    function: callback function to use
 * Output   none
 */
//void twi_attachSlaveRxEvent( void (*function)(uint8_t*, int) )
//{
//  twi_onSlaveReceive = function;
//}

/* 
 * Function twi_attachSlaveTxEvent
 * Desc     sets function called before a slave write operation
 * Input    function: callback function to use
 * Output   none
 */
//void twi_attachSlaveTxEvent( void (*function)() )
//{
//  twi_onSlaveTransmit = function;
//}

/* 
 * Function twiReply
 * Desc     sends byte or readys receive line
 * Input    ack: byte indicating to ack or to nack
 * Output   none
 */
void twiReply(uint8_t ack)
{
  // transmit master read ready signal, with or without ack
    if (ack) {
        TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT) | _BV(TWEA);
    } else {
        TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT);
    }
}

/* 
 * Function twiStop
 * Desc     relinquishes bus master status
 * Input    none
 * Output   none
 */
void twiStop(void)
{
  // send stop condition
  TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA) | _BV(TWINT) | _BV(TWSTO);

  // wait for stop condition to be exectued on bus
  // TWINT is not set after a stop condition!
  while (TWCR & _BV(TWSTO)) {
    continue;
  }

  // update twi state
  twiState = TWI_READY;
}

/* 
 * Function twiReleaseBus
 * Desc     releases bus control
 * Input    none
 * Output   none
 */
void twiReleaseBus(void)
{
  // release bus
  TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA) | _BV(TWINT);

  // update twi state
  twiState = TWI_READY;
}

// SIGNAL(TWI_vect)
void dummy()
{
  switch(TW_STATUS) {
    // All Master
    case TW_START:     // sent start condition
    case TW_REP_START: // sent repeated start condition
      // copy device address and r/w bit to output register and ack
      TWDR = twiSlarw;
      twiReply(1);
      break;

    // Master Transmitter
    case TW_MT_SLA_ACK:  // slave receiver acked address
    case TW_MT_DATA_ACK: // slave receiver acked data
      // if there is data to send, send it, otherwise stop 
      if (twiMasterBufferIndex < twiMasterBufferLength) {
        // copy data to output register and ack
        TWDR = twiMasterBuffer[twiMasterBufferIndex++];
        twiReply(1);
      } else {
        twiStop();
      }
      break;
    case TW_MT_SLA_NACK:  // address sent, nack received
      twiError = TW_MT_SLA_NACK;
      twiStop();
      break;
    case TW_MT_DATA_NACK: // data sent, nack received
      twiError = TW_MT_DATA_NACK;
      twiStop();
      break;
    case TW_MT_ARB_LOST: // lost bus arbitration
      twiError = TW_MT_ARB_LOST;
      twiReleaseBus();
      break;

    // Master Receiver
    case TW_MR_DATA_ACK: // data received, ack sent
      // put byte into buffer
      twiMasterBuffer[twiMasterBufferIndex++] = TWDR;
    case TW_MR_SLA_ACK:  // address sent, ack received
      // ack if more bytes are expected, otherwise nack
      if (twiMasterBufferIndex < twiMasterBufferLength) {
        twiReply(1);
      } else {
        twiReply(0);
      }
      break;
    case TW_MR_DATA_NACK: // data received, nack sent
      // put final byte into buffer
      twiMasterBuffer[twiMasterBufferIndex++] = TWDR;
    case TW_MR_SLA_NACK: // address sent, nack received
      twiStop();
      break;
    // TW_MR_ARB_LOST handled by TW_MT_ARB_LOST case

    // All
    case TW_NO_INFO:   // no state information
      break;
    case TW_BUS_ERROR: // bus error, illegal stop/start
      twiError = TW_BUS_ERROR;
      twiStop();
      break;
  }
}

//========================================================

