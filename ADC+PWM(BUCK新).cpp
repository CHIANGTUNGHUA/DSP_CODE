//###########################################################################
// Description:
//! \addtogroup f2803x_example_list
//! <h1> ADC Start of Conversion (adc_soc)</h1>
//!
//! This ADC example uses ePWM1 to generate a periodic ADC SOC - ADCINT1.
//! Two channels are converted, ADCINA4 and ADCINA2.
//!
//! \b Watch \b Variables \n
//! - Voltage1[10]    - Last 10 ADCRESULT0 values
//! - Voltage2[10]    - Last 10 ADCRESULT1 values
//! - ConversionCount - Current result number 0-9
//! - LoopCount       - Idle loop counter
//
//
//###########################################################################
// $TI Release: F2803x C/C++ Header Files and Peripheral Examples V130 $
// $Release Date: May  8, 2015 $
// $Copyright: Copyright (C) 2009-2015 Texas Instruments Incorporated -
//             http://www.ti.com/ ALL RIGHTS RESERVED $
//###########################################################################

#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File

// Prototype statements for functions found within this file.
__interrupt void adc_isr(void);
__interrupt void xint_isr(void);
__interrupt void cputimer_isr(void);
void Adc_Config(void);
void InitEPwm1Example(void);

// Global variables used in this example:
Uint16 LoopCount;
Uint16 ConversionCount;
Uint16 Voltage1[255];
int xintcount = 0;
float k1, k2, v1, v2, verr, vk1, vk2, vout, vr, z1, z2, vko1, vko2, vou1, vou2;
//extern Uint16 RamfuncsLoadStart;
//extern Uint16 RamfuncsLoadEnd;
//extern Uint16 RamfuncsRunStart;
main() {
	InitSysCtrl();
	DINT;
	// ��l��PIE
	InitPieCtrl();
	// ��l��ADC
	InitAdc();

	//AdcOffsetSelfCal();

	InitPieVectTable();
//	InitFlash();
	//MemCopy(&RamfuncsLoadStart, &RamfuncsLoadEnd, &RamfuncsRunStart);

	// ��l��system

	IER = 0x0000;
	IFR = 0x0000;
	// ��l�Ƥ��_�V�q��

	GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;  // GPIO0 = PWM1A
	GpioCtrlRegs.GPADIR.bit.GPIO0 = 1;   //GPIO0 = output
	SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;
	InitEPwm1Example();

	// �]�w���_�V�q���m
	EALLOW;
	PieVectTable.XINT1 = &xint_isr;
//	PieVectTable.ADCINT1 = &adc_isr;
	PieVectTable.TINT0 = &cputimer_isr;
	EDIS;

	// �]�m�~�����_�}��οO���}��
	EALLOW;
	GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 0;         // GPIO
	GpioCtrlRegs.GPADIR.bit.GPIO1 = 0;          // input
	GpioCtrlRegs.GPAQSEL1.bit.GPIO1 = 0;        // XINT1 Synch to SYSCLKOUT only
	GpioIntRegs.GPIOXINT1SEL.bit.GPIOSEL = 1;   // XINT1 is GPIO1
	GpioCtrlRegs.GPAPUD.bit.GPIO1 = 0;

	GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 0;         // GPIO
	GpioCtrlRegs.GPADIR.bit.GPIO2 = 1;          // OUTput
	GpioDataRegs.GPACLEAR.bit.GPIO2 = 1; 		// ��ADC ON/OFF

	GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 0;         // GPIO
	GpioCtrlRegs.GPADIR.bit.GPIO3 = 1;          // OUTput �{�{��
	GpioDataRegs.GPACLEAR.bit.GPIO3 = 1;
	EDIS;

	IER |= M_INT1;  // �}��GROUP1���_
	EINT;
	ERTM;

	EALLOW;
	PieCtrlRegs.PIECTRL.bit.ENPIE = 1;  // �P��PIE(�D���n)
	PieCtrlRegs.PIEIER1.bit.INTx1 = 0;
	PieCtrlRegs.PIEIER1.bit.INTx4 = 1;
	//PieCtrlRegs.PIEIER1.bit.INTx7 = 1;  // !!!!!!!! ����P�ɶ}��  , �_�h���[  !!!!!!!!!
	XIntruptRegs.XINT1CR.bit.POLARITY = 3; // interrupt occur on both falling and rising edge
	XIntruptRegs.XINT1CR.bit.ENABLE = 1; // Enable Xint1
	EDIS;

	LoopCount = 0;
	ConversionCount = 0;

// Configure ADC
// Note: Channel ADCINA4  will be double sampled to workaround the ADC 1st sample issue for rev0 silicon errata

	EALLOW;
	AdcRegs.ADCCTL1.bit.ADCENABLE = 0;  // ���P��ADC (�binitadc���w�g�P��)
	AdcRegs.ADCCTL1.bit.INTPULSEPOS = 1; //ADCINT1 trips after AdcResults latch

	//AdcRegs.INTSEL1N2.bit.INT1E = 1;	//Enabled ADCINT1
	AdcRegs.INTSEL1N2.bit.INT1CONT = 0;	//Disable ADCINT1 Continuous mode
	AdcRegs.INTSEL1N2.bit.INT1SEL = 0;	//setup EOC0 to trigger ADCINT1 to fire

	AdcRegs.ADCSOC0CTL.bit.CHSEL = 1;	//set SOC0 channel select to ADCINA1
	AdcRegs.ADCSOC0CTL.bit.TRIGSEL = 1;	//set SOC0 start trigger on CPUtimer
	AdcRegs.ADCSOC0CTL.bit.ACQPS = 6;//set SOC0 S/H Window to 7 ADC Clock Cycles, (6 ACQPS plus 1)
	//AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;
	EDIS;

	//GpioCtrlRegs.AIOMUX1 &= ~(0x08);  // ADCINA1
//
	EALLOW;
	SysCtrlRegs.XCLK.bit.XCLKOUTDIV = 2;
	EDIS;

	InitCpuTimers();
	ConfigCpuTimer(&CpuTimer0, 1, 20); //////////�������t�m  �A�P�� !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	EALLOW;
	SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
	CpuTimer0Regs.TCR.all = 0x4000;   // �P��CPUTIMER0
	EDIS;

	for (;;) {
		if (xintcount == 1) {
			Voltage1[0] = AdcResult.ADCRESULT0;
			volatile long double i;
			for (i = 0; i < 50000; i++);
			EALLOW;
			GpioDataRegs.GPATOGGLE.bit.GPIO3 = 1;
			EDIS;
			if (Voltage1[0] > 5000) {
				EALLOW;
				SysCtrlRegs.PCLKCR1.bit.EPWM1ENCLK = 0;
				SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;
				SysCtrlRegs.LPMCR0.bit.LPM = 3;  // ������v
				EDIS;
				EALLOW;
				SysCtrlRegs.CLKCTL.bit.WDHALTI = 1;  // �q��v���}�� , �������]!!
				EDIS;
			}

			v1 = 4014.08;
			v2 = Voltage1[0];
			k1 = 0.0001;
			k2 = 0.000055;
			verr = v1 - v2;
			vk2 = verr + z1;
			z1 = verr;
			vko2 = vk2 * k2;
			vou2 = vko2 + z2;
			z2 = vou2;
			if (vou2 >= 1500)
				vou2 = 1500;
			else if (vou2 <= -1500)
				vou2 = -1500;
			vk1 = verr;
			vko1 = vk1 * k1;
			vout = vou2 + vko1;
			if (vout >= 1500) {
				vout = 1500;
			} else if (vout <= 0) {
				vout = 0;
			}
			vout = 1500 - (int) vout;
			EALLOW;
			EPwm1Regs.CMPA.half.CMPA = vout;
			EDIS;
		}

		else{
		GpioDataRegs.GPACLEAR.bit.GPIO3 = 1;
		}
		LoopCount++;
	}

}

__interrupt void xint_isr(void) {
	if (xintcount == 0) {
		xintcount = 1;
		AdcRegs.ADCCTL1.bit.ADCENABLE = 0;
		GpioDataRegs.GPASET.bit.GPIO2 = 1;
	} else if (xintcount == 1) {
		xintcount = 0;
		GpioDataRegs.GPACLEAR.bit.GPIO2 = 1;
		AdcRegs.ADCCTL1.bit.ADCENABLE = 1;
	}
	EALLOW;
	PieCtrlRegs.PIEIER1.bit.INTx4 = 1;
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;   // Acknowledge interrupt to PIE
	EDIS;
}

__interrupt void cputimer_isr(void) {
	/*	Voltage1[ConversionCount] = AdcResult.ADCRESULT0;

	 //If 20 conversions have been logged, start over
	 if (ConversionCount == 255) {
	 ConversionCount = 0;
	 } else

	 AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //Clear ADCINT1 flag reinitialize for next SOC

	 PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;   // Acknowledge interrupt to PIE

	 return;
	 */
}

void InitEPwm1Example() {
	SysCtrlRegs.PCLKCR1.bit.EPWM1ENCLK = 1;

	// Setup TBCLK
	EPwm1Regs.TBPRD = 1500;           // Set timer period 801 TBCLKs
	// EPwm1Regs.TBPHS.half.TBPHS = 0x0000;           // Phase is 0
	EPwm1Regs.TBCTR = 0x0000;                      // Clear counter

	// Set Compare values
	//EPwm1Regs.CMPA.half.CMPA = 1050;     // Set compare A value

	// Setup counter mode
	EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Count up down
	EPwm1Regs.TBCTL.bit.PHSEN = 0x00;        // Enable phase loading
	EPwm1Regs.TBCTL.bit.HSPCLKDIV = 0;       // Clock ratio to SYSCLKOUT
	EPwm1Regs.TBCTL.bit.CLKDIV = 0;
	EPwm1Regs.TBCTL.bit.SYNCOSEL = 0x01;

	// Setup shadowing
	EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
	EPwm1Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
	EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;  // Load on Zero
	EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

	// Set actions
	EPwm1Regs.AQCTLA.bit.CAU = AQ_SET;    // Set PWM1A on event A, up count
	EPwm1Regs.AQCTLA.bit.CAD = AQ_CLEAR;   // Clear PWM1A on event A, down count

	EPwm1Regs.DBCTL.bit.IN_MODE = 0;
	EPwm1Regs.DBCTL.bit.POLSEL = 2;
	EPwm1Regs.DBCTL.bit.OUT_MODE = 0;
	EPwm1Regs.DBRED = 300;
	EPwm1Regs.DBFED = 300;

}

