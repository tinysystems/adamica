#include <adamica.h>

uint8_t numCores = 1;
uint8_t dmt[] = {1,2};
EXTFRAM_ADDR baseAddress = {0x00, 0x00, 0x00}; //Address in the external FRAM is 24 bits (msb ... lsb), forming (in_address[2] in_address[1] in_address[0])
EXTFRAM_ADDR finAddress = {0xFF, 0xFF, 0x00};

void adamica(fn_ptr func) {
    // sample input power
    uint16_t power = sampleADC();
    // decide on the number of cores
    numCores = DMT(power);
    // activate secondary cores
    activate(numCores, func);
    return;
}

void waitCores(){
    if (numCores > 1) {
        // Check the shared memory if the secondary core has completed its task
        uint8_t complete;
        uint8_t dataLength = 1;
        while(P6IN & BIT3); //Wait for the free bus from the slave
        P4OUT |= BIT1; //Set P4.1 indicating busy bus
        readEXTFRAM(finAddress, &complete, dataLength);
        P4OUT &= ~BIT1; //Reset P4.1 indicating free bus

        // If not completed switch to LPM0 mode and wait for the interrupt
        if (complete != 0x0) __bis_SR_register(LPM0_bits | GIE);
    } else
        return;

}

void signalMainCore(){
    while (UCB1STATW & UCBUSY);
    UCB1TXBUF = FIN_CMD;
    while (UCB1STATW & UCBUSY);
    uint8_t tmp = UCB1RXBUF; //Clean overrun flag
    __delay_cycles(50);
    while (UCB1STATW & 0x01);
}

void deepSleep(){
    __bis_SR_register(LPM4_bits | GIE);
}

uint16_t sampleADC(){
    uint16_t power = 0;
    ADC12CTL0 |= ADC12ENC | ADC12SC;    // Sampling and conversion start
    while (!(ADC12IFGR0 & BIT0));
    power = ADC12MEM0;                 // Read conversion result
    return power;
}
uint8_t DMT(uint16_t power){
    uint8_t n;
    if (power < 0x1770) n = dmt[0]; // If less then 6000 uW stay at single-core mode
    else n = dmt[1]; // else switch to dual-core
    return n;
}
void activate(uint8_t n_cores, fn_ptr func){

    // Split into four, since fn_ptr is 16-bit value, but SPI can send only 8 bit data
    uint8_t dataLength = 1;
    uint16_t tmp_ptr = func;
    uint8_t fn_address[2] = {0};
    int i;
    for(i = dataLength-1; i >= 0; i--){
        fn_address[i] = (uint8_t)tmp_ptr;
        tmp_ptr >>= 8;
    }

    // Write function address to the shared FRAM
    while(P6IN & BIT3); //Wait for the free bus from the slave
    P4OUT |= BIT1; //Set P4.1 indicating busy bus
    writeEXTFRAM(baseAddress, fn_address, dataLength);
    P4OUT &= ~BIT1; //Reset P4.1 indicating free bus

    // Activate the secondary core by triggering corresponding bit via SPI
    if (n_cores > 1){
        // If more than one core is connected, Chip Select signal is needed
        while (UCB1STATW & UCBUSY);
        UCB1TXBUF = EXEC_CMD;
        while (UCB1STATW & UCBUSY);
        uint8_t tmp = UCB1RXBUF; //Clean overrun flag
        __delay_cycles(50);
        while (UCB1STATW & 0x01);
    }
    return;
}

