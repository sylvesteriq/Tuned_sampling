#include "header.h"

void main(void)
{
    InitSysCtrl();      //initialize System Control:
                        //PLL, WatchDog, enable Peripheral Clocks
                        //This example function is found in the DSP2833x_SysCtrl.c file

    Gpio_select();      //select gpios which will be used as flags to indicate EoC

    //
    // Set up ADC clock
    //

    EALLOW;
    #if (CPU_FRQ_150MHZ)     // Default - 150 MHz SYSCLKOUT

        #define ADC_MODCLK 0x3  // HSPCLK = SYSCLKOUT/2*ADC_MODCLK2 = 150/(2*3)   = 25.0 MHz
                                //
    #endif
    #if (CPU_FRQ_100MHZ)
        #define ADC_MODCLK 0x2  // HSPCLK = SYSCLKOUT/2*ADC_MODCLK2 = 100/(2*2)   = 25.0 MHz
                                //
    #endif
    EDIS;

    InitEPwmGpio();

    //
    // Define ADCCLK clock frequency ( less than or equal to 25 MHz )
    // Assuming InitSysCtrl() has set SYSCLKOUT to 150 MHz
    //
    EALLOW;
    SysCtrlRegs.HISPCP.all = ADC_MODCLK;
    EDIS;

    DINT;

    InitPieCtrl();      // Initialize the PIE control registers to their default state.
                        // The default state is all PIE interrupts disabled and flags
                        // are cleared.
                        // This function is found in the DSP2833x_PieCtrl.c file.
                        //

    IER = 0x0000;      //
    IFR = 0x0000;      // Disable CPU interrupts and clear all CPU interrupt flags:
                       //

                        //
    InitPieVectTable(); // Initialize the PIE vector table with pointers to the shell Interrupt
                        // Service Routines (ISR).
                        // This will populate the entire table, even if the interrupt
                        // is not used in this example.  This is useful for debug purposes.
                        // The shell ISR routines are found in DSP2833x_DefaultIsr.c.
                        // This function is found in DSP2833x_PieVect.c.
                        //

    EALLOW;  // enable write to EALLOW protected registers

    EDIS;    // Disable write to EALLOW protected registers

    //
    // Interrupts that are used in this example are re-mapped to
    // ISR functions found within this file.
    //
    EALLOW;  // This is needed to write to EALLOW protected register
    PieVectTable.ADCINT = &adc_isr;
    PieVectTable.EPWM1_INT = &epwm1_isr;
    EDIS;    // This is needed to disable write to EALLOW protected registers

    //
    // Step 4. Initialize all the Device Peripherals:
    // This function is found in DSP2833x_InitPeripherals.c
    //

    InitAdc();  // For this example, init the ADC

    //
    // For this example, only initialize the ePWM
    //
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EDIS;
    InitEPwm1Example();

    InitEPwm2Example(set_SoC_at_falling_edge);

    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    //
    // Enable CPU INT3 which is connected to EPWM1-3 INT
    //
    IER |= M_INT3;

    //
    // Enable EPWM INTn in the PIE: Group 3 interrupt 1-3
    //
    PieCtrlRegs.PIEIER3.bit.INTx1 = 1;
    PieCtrlRegs.PIEIER3.bit.INTx2 = 1;
    PieCtrlRegs.PIEIER3.bit.INTx3 = 1;

    //
    // Enable global Interrupts and higher priority real-time debug events
    //
    EINT;   // Enable Global interrupt INTM
    ERTM;   // Enable Global realtime interrupt DBGM

    //
    // Enable ADCINT in PIE
    //
    PieCtrlRegs.PIEIER1.bit.INTx6 = 1;

               /// Test... to see if i
               /// can enable interrupt on ePWM 1
               PieCtrlRegs.PIEIER3.bit.INTx1; // see PIE table in report

    IER |= M_INT1;      // Enable CPU Interrupt 1
    EINT;               // Enable Global interrupt INTM
    ERTM;               // Enable Global realtime interrupt DBGM

    //
    // Configure ADC
    //

    AdcRegs.ADCMAXCONV.all = 0x0001;       // Setup 2 conv's on SEQ1
    AdcRegs.ADCCHSELSEQ1.bit.CONV01 = 0x2; // Setup ADCINA2 as 1st SEQ1 conv.


    //
    // Enable SOCA from ePWM to start SEQ1
    //

    AdcRegs.ADCTRL2.bit.EPWM_SOCA_SEQ1 = 1;
    AdcRegs.ADCTRL2.bit.INT_ENA_SEQ1 = 1;  // Enable SEQ1 interrupt (every EOS)

    // Note: ePWM1 is GPIO0

    //
    // Wait for ADC interrupt
    //
    for(;;)
    {

    }
}

#include "footer.h"
