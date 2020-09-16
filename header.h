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
//! - Voltage1[10]  - Last 10 ADCRESULT0 values
//! - Voltage2[10]  - Last 10 ADCRESULT1 values
//! - ConversionCount   - Current result number 0-9
//! - LoopCount     - Idle loop counter
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
