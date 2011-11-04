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

#include "flash.h"
#include "platform_hpl.h"

static uint32_t st_flash_addr;
static uint8_t st_flash_mode;



// msp430-spi.h
#define SPI_TX_BUF U0TXBUF
#define SPI_RX_BUF U0RXBUF

#define SPI_WAIT_EOTX() while(!(U0TCTL & TXEPT))
#define SPI_WAIT_EORX() while(!(IFG1 & URXIFG0))


// telos-flash.h
#define ST_FLASH_CS_PORT 4
#define ST_FLASH_CS_PIN  4
#define ST_FLASH_CS ST_FLASH_CS_PORT, ST_FLASH_CS_PIN

#define ST_FLASH_HOLD_PORT 4
#define ST_FLASH_HOLD_PIN  7
#define ST_FLASH_HOLD ST_FLASH_HOLD_PORT, ST_FLASH_HOLD_PIN

#define ST_FLASH_WP_PORT 1
#define ST_FLASH_WP_PIN  2
#define ST_FLASH_WP ST_FLASH_WP_PORT, ST_FLASH_WP_PIN

#define ST_FLASH_MOSI_PORT 3
#define ST_FLASH_MOSI_PIN  1
#define ST_FLASH_MOSI ST_FLASH_MOSI_PORT, ST_FLASH_MOSI_PIN

#define ST_FLASH_MISO_PORT 3
#define ST_FLASH_MISO_PIN  2
#define ST_FLASH_MISO ST_FLASH_MISO_PORT, ST_FLASH_MISO_PIN

#define ST_FLASH_SCLK_PORT 3
#define ST_FLASH_SCLK_PIN  3
#define ST_FLASH_SCLK ST_FLASH_SCLK_PORT, ST_FLASH_SCLK_PIN

#define WREN  0x06
#define WRDI  0x04
#define RDSR  0x05
#define WRSR  0x01
#define READ  0x03
#define FREAD 0x0B
#define PP    0x02
#define SE    0xD8
#define BE    0xC7
#define DP    0xB9
#define RES   0xAB
#define DUMMY 0xAA

// Status Register Masks for the M25P80
#define WIP  0x01
#define WEL  0x02
#define BP0  0x04
#define BP1  0x08
#define BP2  0x10
#define SRWD 0x80



// bitops.h
#define MASK_1(byte, bit) (byte |= (1 << bit))
#define MASK_2(byte, bit1, bit2) (byte |= (1 << bit1) | (1 << bit2))
#define MASK_3(byte, bit1, bit2, bit3)          \
    (byte |= (1 << bit1) | (1 << bit2) | (1 << bit3))
#define MASK_4(byte, bit1, bit2, bit3, bit4)                \
    (byte |= (1 << bit1) | (1 << bit2) | (1 << bit3) | (1 << bit4))
#define MASK_5(byte, bit1, bit2, bit3, bit4, bit5)          \
    (byte |= (1 << bit1) | (1 << bit2) | (1 << bit3) | (1 << bit4) |    \
            (1 << bit5))
#define MASK_6(byte, bit1, bit2, bit3, bit4, bit5, bit6)        \
    (byte |= (1 << bit1) | (1 << bit2) | (1 << bit3) | (1 << bit4) |    \
            (1 << bit5) | (1 << bit6))
#define MASK_7(byte, bit1, bit2, bit3, bit4, bit5, bit6, bit7)      \
    (byte |= (1 << bit1) | (1 << bit2) | (1 << bit3) | (1 << bit4) |    \
            (1 << bit5) | (1 << bit6) | (1 << bit7))

#define UNMASK_1(byte, bit) (byte &= ~(1 << bit))


#define PORT_DIR_(port) P##port##DIR
#define PORT_OUT_(port) P##port##OUT
#define PORT_IN_(port) P##port##IN
//#else
//#define PORT_DIR_(port) DDR##port
//#define PORT_OUT_(port) PORT##port
//#define PORT_IN_(port) PIN##port
//#endif

#define PORT_DIR(port) PORT_DIR_(port)
#define PORT_OUT(port) PORT_OUT_(port)
#define PORT_IN(port) PORT_IN_(port)

#define SET_DIR_1_(port, pin) MASK_1 (PORT_DIR(port), pin)
#define SET_DIR_1(x) SET_DIR_1_ (x)

#define SET_PORT_1_(port, pin) MASK_1 (PORT_OUT(port), pin)
#define SET_PORT_1(x) SET_PORT_1_ (x)

#define UNSET_DIR_1_(port, pin) UNMASK_1 (PORT_DIR(port), pin)
#define UNSET_DIR_1(x) UNSET_DIR_1_ (x)

#define UNSET_PORT_1_(port, pin) UNMASK_1 (PORT_OUT(port), pin)
#define UNSET_PORT_1(x) UNSET_PORT_1_ (x)


//////////////////////////////////
// INTERNAL Flash & SPI Functions
//////////////////////////////////
static inline void st_flash_block_wip();
static inline void spi_tx_byte(uint8_t byte);
static inline uint8_t spi_rx(void);
static inline void st_flash_tx_addr(uint32_t addr);
static inline void st_flash_write_enable();
static inline void st_flash_wake();
static inline void st_flash_sleep();
static void st_flash_page_program(uint32_t addr, const uint8_t* buffer, uint16_t count);
static inline void spi_send(const uint8_t* buf, uint16_t size);
static void mantis_spi_init();




/** @brief transmit a byte (polling mode) */
static inline void spi_tx_byte(uint8_t byte)
{
    SPI_TX_BUF = byte;
    SPI_WAIT_EOTX();
}

/** @brief receive a byte (polling mode) */
static inline uint8_t spi_rx(void)
{
    SPI_TX_BUF = 0;
    SPI_WAIT_EOTX();
    SPI_WAIT_EORX();
    return SPI_RX_BUF;
}

/** @brief send a 24bit address over the SPI */
static inline void st_flash_tx_addr(uint32_t addr)
{
    // gah msp430 is little endian;
    ////spi_send((uint8_t*)&addr, 3);
    spi_tx_byte((addr >> 16) & 0xFF);
    spi_tx_byte((addr >>  8) & 0xFF);
    spi_tx_byte((addr >>  0) & 0xFF);
}


/** @brief enter low power mode */
void st_flash_deep_powerdown()
{
    UNSET_PORT_1(ST_FLASH_CS);

    spi_tx_byte(DP);

    SET_PORT_1(ST_FLASH_CS);
}



/** @brief initialize the st flash, enter deep power down mode */
void st_flash_init()
{
    SET_DIR_1(ST_FLASH_HOLD);
    SET_DIR_1(ST_FLASH_CS);

    SET_PORT_1(ST_FLASH_CS);
    SET_PORT_1(ST_FLASH_HOLD);

    SET_DIR_1(ST_FLASH_WP);
    SET_PORT_1(ST_FLASH_WP);

    SET_DIR_1(ST_FLASH_MOSI);
    SET_DIR_1(ST_FLASH_SCLK);
    UNSET_DIR_1(ST_FLASH_MISO);


    // this macro need only be called once,
    // either by the cc2420 init or here.
    mantis_spi_init(); // WARNING - there are differences between Mantis PLATFORM_SPI_INIT

    // initialize into low power mode
    st_flash_deep_powerdown();


}


void mantis_spi_init()
{
    static uint16_t spi_inited = 0;
    if (spi_inited) return;

    spi_inited = 1;

    U0CTL = SWRST;                                    
    /* 8 bits, SPI, Master, Reset State */             
    U0CTL |= CHAR | SYNC | MM;                        
    /* Delay Tx Half Cycle, SMCLK, 3 Pin Mode */      
    U0TCTL = CKPH | SSEL_3 | STC | TXEPT;             
    /* Baud Rate & Modulation Control Not Used */     
    U0BR0  = 0x02;                                    
    U0BR1  = 0x00;                                    
    U0MCTL = 0x00;                                    
    /* set peripheral mode for MOSI MISO UCLK */      
    MASK_3(P3SEL, 1, 2, 3);                           
    /*UNMASK_1(P4SEL, CC2420_SFD_PIN);*/              
    /* make sure Uart0 rx interrupt is off */         
    IE1 &= ~(URXIE0 | UTXIE0);                        
    /* enable UART0 SPI, send, recv */                
    ME1 |= USPIE0 /*| UTXE0 | URXE0*/;                
    //init_spi_mutex_stuff();                           
    /* Disable Reset State */                         
    U0CTL &= ~SWRST;                                  

//    static unsigned uint16_t spi_inited = 0;
//    if (spi_inited) return;
//
//    spi_inited = 1;
//
//    /* Initalize ports for communication with SPI units. */
//
//    U0CTL  = CHAR + SYNC + MM + SWRST; /* SW  reset,8-bit transfer, SPI master */
//    U0TCTL = CKPH + SSEL1 + STC;  /* Data on Rising Edge, SMCLK, 3-wire. */
//
//    U0BR0  = 0x02;        /* SPICLK set baud. */
//    U0BR1  = 0;  /* Dont need baud rate control register 2 - clear it */
//    U0MCTL = 0;           /* Dont need modulation control. */
//
//    P3SEL |= BV(SCK) | BV(MOSI) | BV(MISO); /* Select Peripheral functionality */
//    P3DIR |= BV(SCK) | BV(MISO);  /* Configure as outputs(SIMO,CLK). */
//
//    ME1   |= USPIE0;     /* Module enable ME1 --> U0ME? xxx/bg */
//    U0CTL &= ~SWRST;      /* Remove RESET */
}


/** @brief block until write in progress is no longer set in the status register */
static inline void st_flash_block_wip()
{
    UNSET_PORT_1(ST_FLASH_CS);

    spi_tx_byte(RDSR);
    while(spi_rx() & WIP)
        ;

    SET_PORT_1(ST_FLASH_CS);
}

/** @brief enable writing to the st flash */
static inline void st_flash_write_enable()
{
    UNSET_PORT_1(ST_FLASH_CS);

    spi_tx_byte(WREN);

    SET_PORT_1(ST_FLASH_CS);
}

/** @brief exit deep power down mode */
static inline void st_flash_wake()
{
    UNSET_PORT_1(ST_FLASH_CS);
    spi_tx_byte(RES);
    SET_PORT_1(ST_FLASH_CS);
}

/** @brief enter deep power down mode */
static inline void st_flash_sleep()
{
    st_flash_block_wip();
    st_flash_deep_powerdown();
}

/** @brief write to a page in flash at addr */
static void st_flash_page_program(uint32_t addr, const uint8_t* buffer, uint16_t count)
{

    st_flash_block_wip();
    st_flash_write_enable();

    UNSET_PORT_1(ST_FLASH_CS);

    spi_tx_byte(PP);
    st_flash_tx_addr(addr);
    spi_send(buffer, count);

    SET_PORT_1(ST_FLASH_CS);
}


static inline void spi_send(const uint8_t* buf, uint16_t size)
{
    // phased out for the collective good.
    //mos_mutex_lock(&spi_mutex);
    uint16_t i;

    for(i = 0; i < size; ++i)
        spi_tx_byte(buf[i]);

/*    spi_buf = buf; */
/*    spi_buf_cnt = size; */
/*    SPI_ENABLE_TX_INT(); */
/*    mos_sem_wait(&spi_sem); */

    //mos_mutex_unlock(&spi_mutex);;
}




/** @brief erase the entire flash */
void st_flash_bulk_erase()
{
    st_flash_block_wip();
    st_flash_write_enable();

    UNSET_PORT_1(ST_FLASH_CS);

    spi_tx_byte(BE);

    SET_PORT_1(ST_FLASH_CS);
}


/** @brief read a block of data from addr in flash */
void flashRead(uint32_t addr, void* buffer, uint16_t count)
{
    st_flash_block_wip();

    UNSET_PORT_1(ST_FLASH_CS);
    spi_tx_byte(READ);
    st_flash_tx_addr(addr);

    /*   spi_recv(buffer, count); */

    uint16_t i = 0;
    for(i = 0; i < count; ++i)
        ((uint8_t *) buffer)[i] = spi_rx();

    SET_PORT_1(ST_FLASH_CS);
}



/** @brief put the flash into deep powerdown mode or power it up */
uint8_t setFlashMode(uint8_t newMode)
{
    switch(newMode) {
    case DEV_MODE_ON:
    case DEV_MODE_IDLE:
        st_flash_wake();
        break;

    case DEV_MODE_OFF:
        st_flash_sleep();
        break;

    default:
        return 1; // not supported
        break;
    }

    st_flash_mode = newMode;
    return 0;
}


void flashSeek(uint32_t addr) {
    st_flash_addr = addr;
}

/** @brief write len bytes (len <= 256) to flash, address must be set before */
void flashWrite(const void *buf, uint16_t len)
{
    if (len > 256)
        len = 256;

    if(st_flash_mode == DEV_MODE_OFF)
        st_flash_wake();

    st_flash_page_program(st_flash_addr, (const uint8_t*)buf, len);

    if(st_flash_mode == DEV_MODE_OFF)
        st_flash_sleep();
}
