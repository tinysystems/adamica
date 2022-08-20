/*
 *
 * Authors 2021 Khakim Akhunov and Kasim Sinan Yildirim
 *
 *
 *
 */
#include <msp430.h>
#include <stdint.h>
#include "extfram.h"

#define SLAVE_CS_OUT    P8OUT
#define SLAVE_CS_DIR    P8DIR
#define SLAVE_CS_PIN    BIT2

#define SPITXBUF UCA3TXBUF
#define SPIRXBUF UCA3RXBUF

//WREN Set write enable latch 0000 0110b
#define FRAM_WREN_CMD 0x06
//WRDI Reset write enable latch 0000 0100b
#define FRAM_WRDI_CMD 0x04
//RDSR Read Status Register 0000 0101b
#define FRAM_RDSR_CMD 0x05
//WRSR Write Status Register 0000 0001b
#define FRAM_WRSR_CMD 0x01
//READ Read memory data 0000 0011b
#define FRAM_READ_CMD 0x03
//FSTRD Fast read memory data 0000 1011b
#define FRAM_FSTRD_CMD 0x0b
//WRITE Write memory data 0000 0010b
#define FRAM_WRITE_CMD 0x02
//SLEEP Enter sleep mode 1011 1001b
#define FRAM_SLEEP_CMD 0xb9
//RDID Read device ID 1001 1111b
#define FRAM_RDID_CMD 0x9f
//SSWR Special Sector Write 42h 0100 0010b
#define FRAM_SSWR_CMD 0x42
//SSRD Special Sector Read 4Bh 0100 1011b
#define FRAM_SSRD_CMD 0x4b
//RUID Read Unique ID 4Ch 0100 1100b
#define FRAM_RUID_CMD 0x4c
//DPD Enter Deep Power-Down BAh 1011 1010b
#define FRAM_DPD_CMD 0xba
//HBN Enter Hibernate mode B9h 1011 1001b
#define FRAM_HBN_CMD 0xb9
//USCI_A3 TX buffer ready?
#define SPI_BUSY UCA3STATW & 0x01

const uint8_t DUMMY = 0x0D;

int32_t max_address = 0x7ffff; //512KB FRAM
int32_t min_address = 0x0;

int generate_address(uint8_t base_address[3], int32_t offset, uint8_t new_address[3]){
    int32_t tmp_address = 0;
    int i;
    for (i = 0; i < 3; i++){
        tmp_address |= (base_address[i] << ((2-i)*8));
    }
    tmp_address += offset;
    if (tmp_address > max_address || tmp_address < min_address) return -1;

    uint8_t tmp = 0;
    for (i = 0; i < 3; i++){
        tmp |= (tmp_address >> ((2-i)*8));
        new_address[i] = tmp;
        tmp = 0;
    }
    return 0;
}

void readEXTFRAM(EXTFRAM_ADDR address, uint8_t* dst, unsigned long len){
    SLAVE_CS_OUT &= ~(SLAVE_CS_PIN);    //Set Chip Select signal
    __delay_cycles(4);
    UCA3TXBUF = FRAM_READ_CMD;
    while (SPI_BUSY);
    UCA3TXBUF = address[0];
    while (SPI_BUSY);
    UCA3TXBUF = address[1];
    while (SPI_BUSY);
    UCA3TXBUF = address[2];
    while (SPI_BUSY);
    uint8_t tmp = UCA3RXBUF; //Clean overrun flag
    while (SPI_BUSY);


    DMACTL1 |= DMA3TSEL__UCA3TXIFG;
    // Write dummy data to TX
    DMA3CTL = DMADT_0 + DMADSTINCR_0 + DMASRCINCR_0 +  DMADSTBYTE__BYTE  + DMASRCBYTE__BYTE ;
    __data20_write_long((uintptr_t) &DMA3SA,(uintptr_t) &DUMMY);
    __data20_write_long((uintptr_t) &DMA3DA,(uintptr_t) &SPITXBUF);
    DMA3SZ = len;
    DMA3CTL |= DMAEN__ENABLE;
    // Read RXBUF -> dst
    DMACTL2 |= DMA4TSEL__UCA3RXIFG;
    DMA4CTL = DMADT_0 + DMADSTINCR_3 + DMASRCINCR_0 +  DMADSTBYTE__BYTE  + DMASRCBYTE__BYTE ;
    //DMA4CTL |= DMAIE;
    __data20_write_long((uintptr_t) &DMA4SA,(uintptr_t) &SPIRXBUF);
    __data20_write_long((uintptr_t) &DMA4DA,(uintptr_t) dst);
    DMA4SZ = len;
    DMA4CTL |= DMAEN__ENABLE;
    //Trigger the DMA to initiate the first transfer
    UCA3IFG &= ~UCTXIFG;
    UCA3IFG |=  UCTXIFG;


    while(DMA3CTL & DMAEN__ENABLE);
    while(DMA4CTL & DMAEN__ENABLE);

    while (SPI_BUSY);
    tmp = UCA3RXBUF; //Clean overrun flag
    SLAVE_CS_OUT |= SLAVE_CS_PIN;
    SLAVE_CS_OUT &= ~(SLAVE_CS_PIN);
}

void writeEXTFRAM(EXTFRAM_ADDR address, uint8_t* src, uint32_t len){
    SLAVE_CS_OUT &= ~(SLAVE_CS_PIN);
    UCA3TXBUF = FRAM_WREN_CMD;
    while(SPI_BUSY);
    SLAVE_CS_OUT |= SLAVE_CS_PIN;
    SLAVE_CS_OUT &= ~(SLAVE_CS_PIN);

    SLAVE_CS_OUT &= ~(SLAVE_CS_PIN);
    SPITXBUF = FRAM_WRITE_CMD;
    while(SPI_BUSY);
    UCA3TXBUF = address[0];
    while(SPI_BUSY);
    UCA3TXBUF = address[1];
    while(SPI_BUSY);
    UCA3TXBUF = address[2];
    while(SPI_BUSY);

    //Triggered when TX is done
    DMACTL1 |= DMA3TSEL__UCA3TXIFG;
    DMA3CTL = DMADT_0 + DMADSTINCR_0 + DMASRCINCR_3 +  DMADSTBYTE__BYTE  + DMASRCBYTE__BYTE ;
    __data20_write_long((uintptr_t) &DMA3SA,(uintptr_t) src);
    __data20_write_long((uintptr_t) &DMA3DA,(uintptr_t) &SPITXBUF);
    DMA3SZ = len;
    DMA3CTL |= DMAEN__ENABLE;
    //Trigger the DMA to initiate the first transfer
    UCA3IFG &= ~UCTXIFG;
    UCA3IFG |=  UCTXIFG;
    while(DMA3CTL & DMAEN__ENABLE);

    while (SPI_BUSY);
    uint8_t tmp = UCA3RXBUF; //Clean overrun flag
    SLAVE_CS_OUT |= SLAVE_CS_PIN;
    SLAVE_CS_OUT &= ~(SLAVE_CS_PIN);
}

void eraseEXTFRAM(EXTFRAM_ADDR address, uint32_t len){
    uint8_t zero[2] = {0x00};
    writeEXTFRAM(address, zero, len);
}




