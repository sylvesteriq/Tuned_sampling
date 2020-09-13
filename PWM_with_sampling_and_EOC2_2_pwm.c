//###########################################################################
//
// FILE:   Example_2833xAdcSoc.c
//
// TITLE:  ADC Start of Conversion Example
//
//! \addtogroup f2833x_example_list
//! <h1> ADC Start of Conversion (adc_soc)</h1>
//!
//! This ADC example uses ePWM1 to generate a periodic ADC SOC on SEQ1.
//! Two channels are converted, ADCINA3 and ADCINA2.
//!
//! \b Watch \b Variables \n
//! - Voltage1[10]	- Last 10 ADCRESULT0 values
//! - Voltage2[10]	- Last 10 ADCRESULT1 values
//! - ConversionCount	- Current result number 0-9
//! - LoopCount		- Idle loop counter
//
//###########################################################################
// $TI Release: F2833x Support Library v2.01.00.00 $
// $Release Date: Thu Mar 19 07:33:36 IST 2020 $
// $Copyright:
// Copyright (C) 2009-2020 Texas Instruments Incorporated - http://www.ti.com/
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions 
// are met:
// 
//   Redistributions of source code must retain the above copyright 
//   notice, this list of conditions and the following disclaimer.
// 
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the 
//   documentation and/or other materials provided with the   
//   distribution.
// 
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// $
//###########################################################################

//
// Included Files
//
#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File



    // My Includes


            //
            // Included Files
            //

            //
            // Typedefs
            //
            typedef struct
            {
                volatile struct EPWM_REGS *EPwmRegHandle;
                Uint16 EPwm_CMPA_Direction;
                Uint16 EPwm_CMPB_Direction;
                Uint16 EPwmTimerIntCount;
                Uint16 EPwmMaxCMPA;
                Uint16 EPwmMinCMPA;
                Uint16 EPwmMaxCMPB;
                Uint16 EPwmMinCMPB;
            } EPWM_INFO;

            //
            // Function Prototypes
            //
void InitEPwm1Example(void);
void InitEPwm2Example();
__interrupt void epwm1_isr(void);

//pwm_2
//__interrupt void epwm2_isr(void); //commented out as we rather not action this event

void update_compare(EPWM_INFO*);
void Gpio_select(void); // function to select GPIO, user added
            //
            // Globals
            //
            EPWM_INFO epwm1_info;
            EPWM_INFO epwm2_info;

            //
            // Defines that configure the period for each timer
            //
            #define EPWM1_TIMER_TBPRD  12000  // Period register // Defines frequency as 12.5Khz with current implementation

            #define EPWM2_TIMER_TBPRD  3000

            //
            // Defines that keep track of which way the compare value is moving
            //
            #define EPWM_CMP_UP   1         //true or false may represent count up or count down
            #define EPWM_CMP_DOWN 0

            #define set_SoC_at_rising_edge 1
            #define set_SoC_at_falling_edge 4

/* self note, Things to check and tick off
 * 1- SMODE is set to 0
 *
 *
 *
 *
 *
 *
 */

/*
 * sequential sampling Mode (single-channel) relevant data
 *
 * we want minimum sample and hold period of 40ns {Acqps = 0 ..note Acqps can vary from 0 to 15}
 *
 *
 */


//
// Function Prototypes
//
__interrupt void adc_isr(void);

__interrupt void epwm1_isr(void);
//
// Globals
//
Uint16 LoopCount;
Uint16 ConversionCount;
Uint16 Voltage1;
Uint16 value;

Uint16 Adc_register_val[1];
//
// Main
//
void main(void)
{
    //
    // Step 1. Initialize System Control:
    // PLL, WatchDog, enable Peripheral Clocks
    // This example function is found in the DSP2833x_SysCtrl.c file.
    //
    InitSysCtrl();
    Gpio_select();

    EALLOW;
    #if (CPU_FRQ_150MHZ)     // Default - 150 MHz SYSCLKOUT
        //
        // HSPCLK = SYSCLKOUT/2*ADC_MODCLK2 = 150/(2*3)   = 25.0 MHz
        //
        #define ADC_MODCLK 0x3
    #endif
    #if (CPU_FRQ_100MHZ)
        //
        // HSPCLK = SYSCLKOUT/2*ADC_MODCLK2 = 100/(2*2)   = 25.0 MHz
        //
        #define ADC_MODCLK 0x2
    #endif
    EDIS;

    //My addition
    InitEPwmGpio();

    //
    // Define ADCCLK clock frequency ( less than or equal to 25 MHz )
    // Assuming InitSysCtrl() has set SYSCLKOUT to 150 MHz
    //
    EALLOW;
    SysCtrlRegs.HISPCP.all = ADC_MODCLK;
    EDIS;

    //
    // Step 2. Initialize GPIO:
    // This example function is found in the DSP2833x_Gpio.c file and
    // illustrates how to set the GPIO to it's default state.
    //
    // InitGpio();  // Skipped for this example

    //
    // Step 3. Clear all interrupts and initialize PIE vector table:
    // Disable CPU interrupts
    //
    DINT;

    //
    // Initialize the PIE control registers to their default state.
    // The default state is all PIE interrupts disabled and flags
    // are cleared.
    // This function is found in the DSP2833x_PieCtrl.c file.
    //
    InitPieCtrl();

    //
    // Disable CPU interrupts and clear all CPU interrupt flags:
    //
    IER = 0x0000;
    IFR = 0x0000;



    //
    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    // This will populate the entire table, even if the interrupt
    // is not used in this example.  This is useful for debug purposes.
    // The shell ISR routines are found in DSP2833x_DefaultIsr.c.
    // This function is found in DSP2833x_PieVect.c.
    //
    InitPieVectTable();

    EALLOW;  // This is needed to write to EALLOW protected registers

    EDIS;    // This is needed to disable write to EALLOW protected registers



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
    // InitPeripherals(); // Not required for this example
    InitAdc();  // For this example, init the ADC

    //
    // Step 5. User specific code, enable interrupts:
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
    // Step 5. User specific code, enable interrupts
    //

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

    LoopCount = 0;
    ConversionCount = 0;










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

    /* The code below sets up a link from EPWM to ADC
     *
     */

    //
    // Assumes ePWM1 clock is already enabled in InitSysCtrl();
    //

            // Note: ePWM1 is GPIO0

    //
    // Wait for ADC interrupt
    //
    for(;;)
    {

    }
}

//
// adc_isr - 
//
__interrupt void  
adc_isr(void)
{
    GpioDataRegs.GPATOGGLE.bit.GPIO30 = 1;   // Toggle GPIO 30
    GpioDataRegs.GPATOGGLE.bit.GPIO30 = 1;   // Toggle GPIO 30


    Voltage1 = AdcRegs.ADCRESULT1 >>4; // This ADC Pin A2

    /*AdcRegs is 16 bit so we need to convert 12 bit value from 12- bit ADC to 16
     * so we use a bit right-shift operation as in >>4
    */
   // Adc_register_val[0] = AdcRegs.ADCRESULT1 >>4;  //This might no longer be useful

    //GpioDataRegs.GPATOGGLE.all=0x01304000;


    //
    // Reinitialize for next ADC sequence
    //
    AdcRegs.ADCTRL2.bit.RST_SEQ1 = 1;         // Reset SEQ1
    AdcRegs.ADCST.bit.INT_SEQ1_CLR = 1;       // Clear INT SEQ1 bit
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;   // Acknowledge interrupt to PIE

    return;
}

            //
            // epwm1_isr -
            //
__interrupt void
 epwm1_isr(void)
{
 //
 // Update the CMPA and CMPB values
 //
 update_compare(&epwm1_info);

 //
 // Clear INT flag for this timer
 //
 EPwm1Regs.ETCLR.bit.INT = 1;

 //
 // Acknowledge this interrupt to receive more interrupts from group 3
 //
 PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}


//We do not need this because it is irrelevant - We don't want to action this isr

//__interrupt void
// epwm2_isr(void)
//{
// //
// // Update the CMPA and CMPB values
// //
// update_compare(&epwm2_info);
//
// //
// // Clear INT flag for this timer
// //
// EPwm2Regs.ETCLR.bit.INT = 1;
//
// //
// // Acknowledge this interrupt to receive more interrupts from group 3
// //
// PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
//}


void   InitEPwm1Example()
                        {
    //
    // Setup TBCLK
    //

    EPwm1Regs.TBCTL.bit.CTRMODE = 0;  // count up and start // ADC implementation

    //EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; // Count up
    EPwm1Regs.TBPRD = EPWM1_TIMER_TBPRD;       // Set timer period
    EPwm1Regs.TBCTL.bit.PHSEN = TB_ENABLE;    // Disable phase loading
    EPwm1Regs.TBPHS.half.TBPHS = 0;       // Phase is 0
    EPwm1Regs.TBCTR = 0x0000;                  // Clear counter
    //EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV2;   // Clock ratio to SYSCLKOUT //SYSCLKOUT/2
    //EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV2;

    EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO; //An attempt to implement synchronization

    //intialize part of the ADC code

//EPWM 2 will now be used to start conversion

//    EPwm1Regs.ETSEL.bit.SOCAEN = 1;     // Enable SOC on A group
//    EPwm1Regs.ETSEL.bit.SOCASEL = 4;    // Select SOC from from CPMA on upcount
//    EPwm1Regs.ETPS.bit.SOCAPRD = 1;     // Generate pulse on 1st event

    // already declared above

    //    EPwm1Regs.CMPA.half.CMPA = 0x0080;    // Set compare A value
    //    EPwm1Regs.TBPRD = 0xFFFF;           // Set period for ePWM1 // 0xFFFF is 65535
    //

    //
    // Setup shadow register load on ZERO
    //

    EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

    //
    // Set Compare values
    //
    EPwm1Regs.CMPA.half.CMPA = (EPWM1_TIMER_TBPRD/2);    // Set compare A value
    EPwm1Regs.CMPB = (EPWM1_TIMER_TBPRD/2);              // Set Compare B value
    //
    // Set actions
    //
    EPwm1Regs.AQCTLA.bit.ZRO = AQ_SET;      // Set PWM1A on Zero
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;    // Clear PWM1A on event A, up count
    EPwm1Regs.AQCTLB.bit.ZRO = AQ_SET;      // Set PWM1B on Zero
    EPwm1Regs.AQCTLB.bit.CBU = AQ_CLEAR;    // Clear PWM1B on event B, up count
    //
    // Interrupt where we will change the Compare Values
    //
    EPwm1Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;     // Select INT on Zero event
    EPwm1Regs.ETSEL.bit.INTEN = 1;                // Enable INT
    EPwm1Regs.ETPS.bit.INTPRD = ET_3RD;           // Generate INT on 3rd event
    //
    // Information this example uses to keep track of the direction the
    // CMPA/CMPB values are moving, the min and max allowed values and
    // a pointer to the correct ePWM registers
    //
    //
    // Start by increasing CMPA & CMPB
    //
    epwm1_info.EPwm_CMPA_Direction = EPWM_CMP_UP;
    epwm1_info.EPwm_CMPB_Direction = EPWM_CMP_UP;
    epwm1_info.EPwmTimerIntCount = 0;      // Zero the interrupt counter
    epwm1_info.EPwmRegHandle = &EPwm1Regs; //Set the pointer to the ePWM module
    epwm1_info.EPwmMaxCMPA = EPWM1_TIMER_TBPRD;  // Setup min/max CMPA/CMPB values
    epwm1_info.EPwmMinCMPA = 0;
    epwm1_info.EPwmMaxCMPB = EPWM1_TIMER_TBPRD;
    epwm1_info.EPwmMinCMPB = 0;
    }

int return_frequency(int frequency_in_khz){

}

void InitEPwm2Example(int Soc_position)
{
    //
    // Setup TBCLK
    //

    EPwm2Regs.TBCTL.bit.CTRMODE = 0;  // count up and start // ADC implementation

    //EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; // Count up
    EPwm2Regs.TBPRD = EPWM2_TIMER_TBPRD;       // Set timer period
    EPwm2Regs.TBCTL.bit.PHSEN = TB_ENABLE;    // Disable phase loading
    EPwm2Regs.TBPHS.half.TBPHS = 1500;       // Phase is 0
    EPwm2Regs.TBCTR = 0x0000;                  // Clear counter

    EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN; // sync flow-through
    //intialize part of the ADC code


    EPwm2Regs.ETSEL.bit.SOCAEN = 1;                 // Enable SOC on A group
    EPwm2Regs.ETSEL.bit.SOCASEL = Soc_position;     // Select SOC from from CPMA on upcount
                                                    // set to 1 for soC at pulse rising edge
                                                    // set to 4 for soC at pulse falling edge

    EPwm2Regs.ETPS.bit.SOCAPRD = 1;     // Generate pulse on 1st event

    // already declared above

    //    EPwm1Regs.CMPA.half.CMPA = 0x0080;    // Set compare A value
    //    EPwm1Regs.TBPRD = 0xFFFF;           // Set period for ePWM1 // 0xFFFF is 65535
    //

    //
    // Setup shadow register load on ZERO
    //
    EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm2Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm2Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm2Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

    //
    // Set Compare values
    //
    EPwm2Regs.CMPA.half.CMPA = EPWM2_TIMER_TBPRD;    // Set compare A value
    EPwm2Regs.CMPB = EPWM2_TIMER_TBPRD;              // Set Compare B value
    //
    // Set actions
    //
    EPwm2Regs.AQCTLA.bit.ZRO = AQ_SET;      // Set PWM1A on Zero
    EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;    // Clear PWM1A on event A, up count
    EPwm2Regs.AQCTLB.bit.ZRO = AQ_SET;      // Set PWM1B on Zero
    EPwm2Regs.AQCTLB.bit.CBU = AQ_CLEAR;    // Clear PWM1B on event B, up count
    //
    // Interrupt where we will change the Compare Values
    //
//    EPwm2Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;     // Select INT on Zero event
//    EPwm2Regs.ETSEL.bit.INTEN = 1;                // Enable INT
//    EPwm2Regs.ETPS.bit.INTPRD = ET_3RD;           // Generate INT on 3rd event
    //
    // Information this example uses to keep track of the direction the
    // CMPA/CMPB values are moving, the min and max allowed values and
    // a pointer to the correct ePWM registers
    //
    //
    // Start by increasing CMPA & CMPB
    //
    epwm2_info.EPwm_CMPA_Direction = EPWM_CMP_UP;
    epwm2_info.EPwm_CMPB_Direction = EPWM_CMP_UP;
    epwm2_info.EPwmTimerIntCount = 0;      // Zero the interrupt counter
    epwm2_info.EPwmRegHandle = &EPwm1Regs; //Set the pointer to the ePWM module
    epwm2_info.EPwmMaxCMPA = EPWM2_TIMER_TBPRD;  // Setup min/max CMPA/CMPB values
    epwm2_info.EPwmMinCMPA = 0;
    epwm2_info.EPwmMaxCMPB = EPWM2_TIMER_TBPRD;
    epwm2_info.EPwmMinCMPB = 0;
    }




      //
      // update_compare -
      //
void
     update_compare(EPWM_INFO *epwm_info)
  {
     //
     // Every 1st interrupt, change the CMPA/CMPB values
     //

    EPwm1Regs.CMPA.half.CMPA = (EPWM1_TIMER_TBPRD/2);
    EPwm2Regs.CMPA.half.CMPA = (EPWM2_TIMER_TBPRD/2);

    //commmented out on purpose. uncomment when voltage read in is valuable
//
//     if (Voltage1 < 1950){
//        EPwm1Regs.CMPA.half.CMPA = Voltage1;
//        EPwm2Regs.CMPA.half.CMPA = Voltage1;
//     }
//     else{
//         EPwm1Regs.CMPA.half.CMPA = (EPWM2_TIMER_TBPRD/2);
//         EPwm2Regs.CMPA.half.CMPA = (EPWM2_TIMER_TBPRD/2);
//     }

     return;
  }


// function to select gpio of interest
void
Gpio_select(void)
{
 EALLOW;
 GpioCtrlRegs.GPAMUX2.bit.GPIO30 = 0; // select GPIO 30
 GpioCtrlRegs.GPADIR.bit.GPIO30 = 1;  // set GPIO as output
 //GpioCtrlRegs.GPBDIR.all = 0x0000000F;   // All outputs
 EDIS;
}

//
// End of File
//
