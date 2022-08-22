#include <msp430.h>
#include <stdlib.h>
#include <stdint.h>
#include <nvs.h>
#include <ctpl.h>
#include <ctpl_low_level.h>

#include <adamica.h>
#include <fram.h>
#include <input.h>

#define SLAVE_CS_OUT    P8OUT
#define SLAVE_CS_DIR    P8DIR
#define SLAVE_CS_PIN    BIT2

#define ADC_MONITOR_THRESHOLD   2.0
#define ADC_MONITOR_FREQUENCY   1000

#define MCLK_FREQUENCY          1000000
#define SMCLK_FREQUENCY         1000000

typedef struct{
    uint16_t c[OUTPUT_HEIGHT][OUTPUT_HEIGHT];
    uint16_t i;
    uint16_t j;
    uint16_t k;
    uint16_t start;
    uint16_t end;
    uint16_t n;
    uint16_t m;
    uint16_t tmp;
    fn_ptr fn;
} data_t;

data_t data;
uint8_t nvsStorage1[NVS_DATA_STORAGE_SIZE(sizeof(data_t))] = {0};
nvs_data_handle nvsHandle;
uint16_t status;


void multiply(uint8_t[][INPUT_HEIGHT], uint8_t[][INPUT_HEIGHT], uint16_t[][OUTPUT_HEIGHT]);
void initClockTo16MHz(void);
void initGPIO(void);
void initSPI(void);
void initSPIFRAM(void);
void initAdcMonitor(void);

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5; // Disable the GPIO power-on default high-impedance mode

    initClockTo16MHz();
    initGPIO();
    initSPIFRAM();
    initSPI();
    initAdcMonitor();

    nvsHandle = nvs_data_init(nvsStorage1, sizeof(data_t));
    status = nvs_data_restore(nvsHandle, &data);
    ctpl_init();

    #ifdef MAINCORE
    multiply(a, b, data.c);
    #else
    deepSleep();
    #endif
    
    return 0;
}

void multiply(uint8_t a[][INPUT_HEIGHT], uint8_t b[][INPUT_HEIGHT], uint16_t c[][OUTPUT_HEIGHT]){

    // Initialise all the elements of output matrix to 0
    int nOut,mOut;
    for(nOut = 0; nOut < OUTPUT_HEIGHT; nOut++){
        for(mOut = 0; mOut < OUTPUT_WIDTH; mOut++){
            data.c[nOut][mOut] = 0;
        }
    }

    data.n = INPUT_HEIGHT;
    data.m = INPUT_WIDTH;

    data.fn = &multiply;

    _begin_parallel(data.fn)

    data.start = coreid * data.n/numCores;
    data.end = data.start + data.n/numCores;

    for (data.i = data.start; data.i < data.end; data.i++){
        for (data.j = 0; data.j < data.m; data.j++){
            for (data.k = 0; data.k < data.m; data.k++){
                data.tmp = a[data.i][data.k] * b[data.k][data.j];
                data.c[data.i][data.j] += data.tmp;
            }
        }
    }
    _end_parallel

    return;
}

void initClockTo16MHz(void){
    // Configure one FRAM waitstate as required by the device datasheet for MCLK
    // operation beyond 8MHz _before_ configuring the clock system.
    FRCTL0 = FRCTLPW | NWAITS_1;

    // Clock System Setup
    CSCTL0_H = CSKEY_H;                     // Unlock CS registers
    CSCTL1 = DCOFSEL_0;                     // Set DCO to 1MHz

    // Set SMCLK = MCLK = DCO, ACLK = VLOCLK
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;

    // Per Device Errata set divider to 4 before changing frequency to
    // prevent out of spec operation from overshoot transient
    CSCTL3 = DIVA__4 | DIVS__4 | DIVM__4;   // Set all corresponding clk sources to divide by 4 for errata
    CSCTL1 = DCOFSEL_4 | DCORSEL;           // Set DCO to 16MHz

    // Delay by ~10us to let DCO settle. 60 cycles = 20 cycles buffer + (10us / (1/4MHz))
    __delay_cycles(60);
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;   // Set all dividers to 1 for 16MHz operation
    CSCTL0_H = 0;                           // Lock CS registers
}

void initGPIO(void){
    // Configure GPIO
    P6OUT = 0x0; //Clear OUTPUT of P6
    P6DIR = 0xFF;
    //SPI master for the external FRAM
    P6SEL1 &= ~(BIT0 | BIT1 | BIT2 | BIT3); // USCI_A3 SCLK, MOSI, and MISO pin
    P6SEL0 |= (BIT0 | BIT1 | BIT2 | BIT3);
    //SPI master for the secondary core
    P5SEL1 &= ~(BIT0 | BIT1 | BIT2); // USCI_B1 SCLK, MOSI, and MISO pin
    P5SEL0 |= (BIT0 | BIT1 | BIT2);

    SLAVE_CS_DIR |= SLAVE_CS_PIN;
    SLAVE_CS_OUT |= SLAVE_CS_PIN;

    //P6DIR |= (BIT0 | BIT2); //Clock and MOSI pins direction out
    //P5DIR |= (BIT0 | BIT2); //Clock and MOSI pins direction out

    //Bus control pin (STE pin for SPI A3)
    P4OUT &= ~BIT1; //Clear OUTPUT of P4.1
    P4DIR |= BIT1; //Define OUT direction of P4.1

    //Start computation pin
    P4OUT &= ~BIT2; //Clear OUTPUT of P4.2
    P4DIR |= BIT2; //Define OUT direction of P4.2

    // Configure GPIO for the LED
    P1OUT &= ~BIT0;  // Clear P1.0 output latch for a defined power-on state
    P1DIR |= BIT0;   // Set P1.0 to output direction

    P1SEL1 |= BIT5;  // Configure P1.5 for ADC
    P1SEL0 |= BIT5;

//    // Configure ADC12
//    ADC12CTL0 = ADC12SHT0_2 | ADC12ON;
//    ADC12CTL1 = ADC12SHP;                   // ADCCLK = MODOSC; sampling timer
//    ADC12CTL2 |= ADC12RES_2;                // 12-bit conversion results
//    //ADC12IER0 |= ADC12IE0;                  // Enable ADC conv complete interrupt
//    ADC12MCTL0 |= ADC12INCH_5 | ADC12VRSEL_0; // A1 ADC input select; Vref=1.2V


    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;
}
void initSPIFRAM(void)
{
    //Clock Polarity: The inactive state is high
    //MSB First, 8-bit, Master, 3-pin mode, Synchronous
    UCA3CTLW0 = UCSWRST;                       // **Put state machine in reset**
    UCA3CTLW0 |= UCCKPL | UCSYNC | UCMSB
                | UCMST | UCSSEL__SMCLK | UCMODE_1 | UCSTEM_0;      // 3-pin, 8-bit SPI Master
    UCA3BRW = 0x0;
    //UCB1MCTLW = 0;
    UCA3CTLW0 &= ~UCSWRST;                     // **Initialize USCI state machine**
    //UCB1IE |= UCRXIE;                          // Enable USCI0 RX interrupt
}

void initSPI(void)
{
    //Clock Polarity: The inactive state is high
    //MSB First, 8-bit, Master, 3-pin mode, Synchronous
    UCB1CTLW0 = UCSWRST;                       // **Put state machine in reset**
    #ifdef MAINCORE
        UCB1CTLW0 |= UCCKPL | UCSYNC | UCMSB
                    | UCMST | UCSSEL__SMCLK | UCMODE_0;      // 3-pin, 8-bit SPI Master
    #else
        UCB1CTLW0 |= UCCKPL | UCSYNC | UCMSB
                    | UCMST_0 | UCSSEL__SMCLK | UCMODE_0;      // 3-pin, 8-bit SPI Slave
    #endif
    UCB1BRW = 0x0;
    //UCB1MCTLW = 0;
    UCB1CTLW0 &= ~UCSWRST;                     // **Initialize USCI state machine**
    //UCB1IE |= UCRXIE;                          // Enable USCI0 RX interrupt
}

void initAdcMonitor(void)
{
    /* Initialize timer for ADC trigger. */
    TA0CCR0 = (SMCLK_FREQUENCY/ADC_MONITOR_FREQUENCY);
    TA0CCR1 = TA0CCR0/2;
    TA0CCTL1 = OUTMOD_3;
    TA0CTL = TASSEL__SMCLK | MC__UP;

    /* Configure internal 2.0V reference. */
    while(REFCTL0 & REFGENBUSY);
    REFCTL0 |= REFVSEL_1 | REFON;
    while(!(REFCTL0 & REFGENRDY));

    /*
     * Initialize ADC12_B window comparator using the battery monitor.
     * The monitor will first enable the high side to the monitor voltage plus
     * 0.1v to make sure the voltage is sufficiently above the threshold. When
     * the high side is triggered the interrupt service routine will switch to
     * the low side and wait for the voltage to drop below the threshold. When
     * the voltage drops below the threshold voltage the device will invoke the
     * compute through power loss shutdown function to save the application
     * state and enter complete device shutdown.
     */
    ADC12CTL0 = ADC12SHT0_2 | ADC12ON;
    ADC12CTL1 = ADC12SHS_1 | ADC12SSEL_0 | ADC12CONSEQ_2 | ADC12SHP;
    ADC12CTL3 = ADC12BATMAP;
    ADC12MCTL0 = ADC12INCH_31 | ADC12VRSEL_1 | ADC12WINC;
    ADC12HI = (uint16_t)(4096*((ADC_MONITOR_THRESHOLD+0.1)/2)/(2.0));
    ADC12LO = (uint16_t)(4096*(ADC_MONITOR_THRESHOLD/2)/(2.0));
    ADC12IFGR2 &= ~(ADC12HIIFG | ADC12LOIFG);
    ADC12IER2 = ADC12HIIE;
    ADC12CTL0 |= ADC12ENC;
}

/*******INTERRUPT SERVICE ROUTINES********/

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = DMA_VECTOR
__interrupt void DMA_A4_ISR (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(DMA_VECTOR))) DMA_A4_ISR (void)
#else
#error Compiler not supported!
#endif
{
    DMA4CTL &= ~DMAIFG;
    while (UCA3STATW & 0x01);
    SLAVE_CS_OUT |= SLAVE_CS_PIN;
    __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_B1_VECTOR
__interrupt void USCI_B1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_B1_VECTOR))) USCI_B1_ISR (void)
#else
#error Compiler not supported!
#endif
{
    int tmp = UCB1RXBUF;
    if (tmp == 0xE0){ // Signal coming to the secondary core to Execute
        // Read function address from external FRAM
        uint8_t fn_address[2] = {0}; // We need to read four bytes, since the function pointer is 16-bit value
        uint8_t dataLength = 2;
        while(P6IN & BIT3); //Wait for the free bus from the slave
        P4OUT |= BIT1; //Set P4.1 indicating busy bus
        readEXTFRAM(baseAddress, fn_address, dataLength);
        P4OUT &= ~BIT1; //Reset P4.1 indicating free bus

        uint32_t tmp_ptr = 0x0;
        int i;
        for(i = 0; i < dataLength; i++){
            tmp_ptr |= fn_address[i];
            tmp_ptr <<= 8;
        }

        fn_ptr function = (fn_ptr)tmp_ptr;
        function(a,b,data.c);

    } else if (tmp == 0xF0) // Signal coming to the main core about completion of the subtask
        __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0 for the main core
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_B1_VECTOR))) USCI_B1_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(ADC12IV, ADC12IV_ADC12LOIFG)) {
        case ADC12IV_NONE:        break;        // Vector  0: No interrupt
        case ADC12IV_ADC12OVIFG:  break;        // Vector  2: ADC12MEMx Overflow
        case ADC12IV_ADC12TOVIFG: break;        // Vector  4: Conversion time overflow
        case ADC12IV_ADC12HIIFG:                // Vector  6: Window comparator high side
            /* Disable the high side and enable the low side interrupt. */
            ADC12IER2 &= ~ADC12HIIE;
            ADC12IER2 |= ADC12LOIE;
            ADC12IFGR2 &= ~ADC12LOIFG;
            break;
        case ADC12IV_ADC12LOIFG:                // Vector  8: Window comparator low side
            /* Enter device shutdown with 64ms timeout. */
            status = nvs_data_commit(nvsHandle, &data);
            ctpl_saveCpuStackEnterLpm(CTPL_MODE_NONE,0);

            /* Disable the low side and enable the high side interrupt. */
            ADC12IER2 &= ~ADC12LOIE;
            ADC12IER2 |= ADC12HIIE;
            ADC12IFGR2 &= ~ADC12HIIFG;
            break;
        default: break;
    }
}

