/*
 * footer.h
 *
 *  Created on: 14 Sep. 2020
 *      Author: fjaksfj
 */

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

void InitEPwm1Example()
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

    EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO; //An attempt to implement synchronization

    //initialize part of the ADC code

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
    EPwm1Regs.CMPA.half.CMPA = 0;    // Set compare A value (from 0 to 449)

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
//    epwm1_info.EPwm_CMPA_Direction = EPWM_CMP_UP;
//    epwm1_info.EPwm_CMPB_Direction = EPWM_CMP_UP;
//    epwm1_info.EPwmTimerIntCount = 0;      // Zero the interrupt counter
//    epwm1_info.EPwmRegHandle = &EPwm1Regs; //Set the pointer to the ePWM module
//    epwm1_info.EPwmMaxCMPA = ((EPWM1_TIMER_TBPRD/2)-PWM_DEAD_BAND);  // Setup min/max CMPA/CMPB values
//    epwm1_info.EPwmMinCMPA = 0;
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
    EPwm2Regs.TBPHS.half.TBPHS = 0;       // Phase is 0
    EPwm2Regs.TBCTR = 0x0000;                  // Clear counter

    EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN; // sync flow-through
    //intialize part of the ADC code


    EPwm2Regs.ETSEL.bit.SOCAEN = 1;                 // Enable SOC on A group
    EPwm2Regs.ETSEL.bit.SOCASEL = Soc_position;     // Select SOC from from CPMA on upcount
                                                    // set to 1 for soC at pulse rising edge
                                                    // set to 4 for soC at pulse falling edge

    EPwm2Regs.ETPS.bit.SOCAPRD = 1;     // Generate pulse on 1st event

    //
    // Setup shadow register load on ZERO
    //
    EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm2Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm2Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm2Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

    //set sampling PWM (EPWM 2) to 50%
        EPwm2Regs.CMPA.half.CMPA = (EPWM2_TIMER_TBPRD/2);

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
//    epwm2_info.EPwm_CMPA_Direction = EPWM_CMP_UP;
//    epwm2_info.EPwm_CMPB_Direction = EPWM_CMP_UP;
//    epwm2_info.EPwmTimerIntCount = 0;      // Zero the interrupt counter
//    epwm2_info.EPwmRegHandle = &EPwm1Regs; //Set the pointer to the ePWM module
//    epwm2_info.EPwmMaxCMPA = EPWM2_TIMER_TBPRD/2;  // Setup min/max CMPA/CMPB values
//    epwm2_info.EPwmMinCMPA = 0;
}

void InitEPwm3Example()
{
    //
    // Setup TBCLK
    //

    EPwm3Regs.TBCTL.bit.CTRMODE = 0;  // count up and start // ADC implementation

    //EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; // Count up
    EPwm3Regs.TBPRD = EPWM1_TIMER_TBPRD;       // Set timer period
    EPwm3Regs.TBCTL.bit.PHSEN = TB_ENABLE;    // Disable phase loading
    EPwm3Regs.TBPHS.half.TBPHS = ((EPWM1_TIMER_TBPRD/2));       // Phase is shifted by 180 degrees
    EPwm3Regs.TBCTR = 0x0000;                  // Clear counter

    EPwm3Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO; //An attempt to implement synchronization

    //intialize part of the ADC code

    //
    // Setup shadow register load on ZERO
    //

    EPwm3Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm3Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm3Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm3Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

    //
    // Set Compare values
    //

    EPwm3Regs.CMPA.half.CMPA = 0;    // Set compare A value (from 0 to 449)

    //
    // Set actions
    //
    EPwm3Regs.AQCTLA.bit.ZRO = AQ_SET;      // Set PWM1A on Zero
    EPwm3Regs.AQCTLA.bit.CAU = AQ_CLEAR;    // Clear PWM1A on event A, up count
    EPwm3Regs.AQCTLB.bit.ZRO = AQ_SET;      // Set PWM1B on Zero
    EPwm3Regs.AQCTLB.bit.CBU = AQ_CLEAR;    // Clear PWM1B on event B, up count

//    epwm1_info.EPwm_CMPA_Direction = EPWM_CMP_UP;
//    epwm1_info.EPwm_CMPB_Direction = EPWM_CMP_UP;
//    epwm1_info.EPwmTimerIntCount = 0;      // Zero the interrupt counter
//    epwm1_info.EPwmRegHandle = &EPwm1Regs; //Set the pointer to the ePWM module
//    epwm1_info.EPwmMaxCMPA = ((EPWM1_TIMER_TBPRD/2)-PWM_DEAD_BAND);  // Setup min/max CMPA/CMPB values
//    epwm1_info.EPwmMinCMPA = 0;
}

void InitEPwm4Example()
{
    //
    // Setup TBCLK
    //

    EPwm4Regs.TBCTL.bit.CTRMODE = 0;  // count up and start // ADC implementation

    //EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; // Count up
    EPwm4Regs.TBPRD = EPWM1_TIMER_TBPRD;       // Set timer period
    EPwm4Regs.TBCTL.bit.PHSEN = TB_ENABLE;    // Disable phase loading
    EPwm4Regs.TBPHS.half.TBPHS = 0;       // Phase is 0
    EPwm4Regs.TBCTR = 0x0000;                  // Clear counter

    EPwm4Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO; //An attempt to implement synchronization

    //intialize part of the ADC code

    //
    // Setup shadow register load on ZERO
    //

    EPwm4Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm4Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm4Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm4Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

    //
    // Set Compare values
    //
    EPwm4Regs.CMPA.half.CMPA = 0;    // Set compare A value (from 0 to 449)

    //
    // Set actions
    //
    EPwm4Regs.AQCTLA.bit.ZRO = AQ_SET;      // Set PWM1A on Zero
    EPwm4Regs.AQCTLA.bit.CAU = AQ_CLEAR;    // Clear PWM1A on event A, up count
    EPwm4Regs.AQCTLB.bit.ZRO = AQ_SET;      // Set PWM1B on Zero
    EPwm4Regs.AQCTLB.bit.CBU = AQ_CLEAR;    // Clear PWM1B on event B, up count
    //

}

void InitEPwm5Example()
{
    //
    // Setup TBCLK
    //

    EPwm5Regs.TBCTL.bit.CTRMODE = 0;  // count up and start // ADC implementation

    //EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; // Count up
    EPwm5Regs.TBPRD = EPWM1_TIMER_TBPRD;       // Set timer period
    EPwm5Regs.TBCTL.bit.PHSEN = TB_ENABLE;    // Disable phase loading
    EPwm5Regs.TBPHS.half.TBPHS = ((EPWM1_TIMER_TBPRD/2));       // Phase is 180
    EPwm5Regs.TBCTR = 0x0000;                  // Clear counter

    EPwm5Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO; //An attempt to implement synchronization

    //intialize part of the ADC code

    //
    // Setup shadow register load on ZERO
    //

    EPwm5Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm5Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm5Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm5Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

    //
    // Set Compare values
    //

    EPwm5Regs.CMPA.half.CMPA = 0;    // Set compare A value (from 0 to 449)

    //
    // Set actions
    //
    EPwm5Regs.AQCTLA.bit.ZRO = AQ_SET;      // Set PWM1A on Zero
    EPwm5Regs.AQCTLA.bit.CAU = AQ_CLEAR;    // Clear PWM1A on event A, up count
    EPwm5Regs.AQCTLB.bit.ZRO = AQ_SET;      // Set PWM1B on Zero
    EPwm5Regs.AQCTLB.bit.CBU = AQ_CLEAR;    // Clear PWM1B on event B, up count
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

    EPwm1Regs.CMPA.half.CMPA = (((EPWM1_TIMER_TBPRD/2)-PWM_DEAD_BAND)*pulse_width);
    EPwm3Regs.CMPA.half.CMPA = (((EPWM1_TIMER_TBPRD/2)-PWM_DEAD_BAND)*pulse_width);
    EPwm4Regs.CMPA.half.CMPA = ((EPWM1_TIMER_TBPRD/2)-PWM_DEAD_BAND*pulse_width);
    EPwm5Regs.CMPA.half.CMPA = (((EPWM1_TIMER_TBPRD/2)-PWM_DEAD_BAND)*pulse_width);
    return;
}

// function to select gpio for sampling flag
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
