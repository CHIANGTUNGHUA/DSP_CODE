/* 
�Q��ADC���60HZ���i�H�� , �ϥ�cpuTimerĲ�o , 
�åB���@�}������ ADC�O�_�Ұ� , �Y�Ұ� , �h�O�G , �_�h�O�t 
*/






#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File

// Prototype statements for functions found within this file.
__interrupt void adc_isr(void);
__interrupt void xint_isr(void);
__interrupt void cputimer_isr(void);
void Adc_Config(void);

// Global variables used in this example:
Uint16 LoopCount;
Uint16 ConversionCount;
Uint16 Voltage1[255];
int xintcount = 0;

main() {
	// ��l��system
	InitSysCtrl();
	DINT;
	// ��l��PIE
	InitPieCtrl();
	// ��l��ADC
	InitAdc();

	AdcOffsetSelfCal();
    // ��l�Ƥ��_�V�q�� 
	InitPieVectTable();

	IER = 0x0000;
	IFR = 0x0000;
	
	// �]�w���_�V�q���m
	EALLOW;
	PieVectTable.XINT1 = &xint_isr;
	PieVectTable.ADCINT1 = &adc_isr;
	PieVectTable.TINT0 = &cputimer_isr;
	EDIS;

	// �]�m�~�����_�}��οO���}��
	EALLOW;
	GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 0;         // GPIO �\�ର�@��I/O
	GpioCtrlRegs.GPADIR.bit.GPIO0 = 0;          // GPIO0 = input
	GpioCtrlRegs.GPAQSEL1.bit.GPIO0 = 0;        // XINT1 Synch to SYSCLKOUT only
	GpioIntRegs.GPIOXINT1SEL.bit.GPIOSEL = 0;   // XINT1 is GPIO0

	GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 0;			// GPIO �\�ର�@��I/O 
	GpioCtrlRegs.GPADIR.bit.GPIO1 = 1;          // GPIO1 = output
	GpioDataRegs.GPACLEAR.bit.GPIO1 = 1;		// GPIO1 ��X��0 
	EDIS;

	IER |= M_INT1;  // �}��GROUP1���_
	EINT;
	ERTM;
	EALLOW;
	PieCtrlRegs.PIECTRL.bit.ENPIE = 1;  // �P�� PIE(�D���n)
	PieCtrlRegs.PIEIER1.bit.INTx1 = 0;  // �P�� ADCINT1 
	PieCtrlRegs.PIEIER1.bit.INTx4 = 1;  // �P�� XINT 
//	PieCtrlRegs.PIEIER1.bit.INTx7 = 1;  // �P�� CPUTIMER !!!!!!!! ����P�ɶ}��  , �_�h���[  !!!!!!!!! (���D���ݭn)
	XIntruptRegs.XINT1CR.bit.POLARITY = 3; // interrupt occur on both falling and rising edge
	XIntruptRegs.XINT1CR.bit.ENABLE = 1; // Enable Xint1
	EDIS;

	LoopCount = 0;
	ConversionCount = 0;

// Configure ADC
// Note: Channel ADCINA4  will be double sampled to workaround the ADC 1st sample issue for rev0 silicon errata

	EALLOW;
	//AdcRegs.ADCCTL1.bit.ADCENABLE = 1;  // �P��ADC (�binitadc���w�g�P��)
	AdcRegs.ADCCTL1.bit.INTPULSEPOS = 1; // ADCINT1 trips after AdcResults latch

	AdcRegs.INTSEL1N2.bit.INT1E = 1;	// Enabled ADCINT1
	AdcRegs.INTSEL1N2.bit.INT1CONT = 0;	// Disable ADCINT1 Continuous mode
	AdcRegs.INTSEL1N2.bit.INT1SEL = 0;	// setup EOC0 to trigger ADCINT1 to fire

	AdcRegs.ADCSOC0CTL.bit.CHSEL = 1;	// set SOC0 channel select to ADCINA1
	AdcRegs.ADCSOC0CTL.bit.TRIGSEL = 1;	// set SOC0 start trigger on CPUtimer
	AdcRegs.ADCSOC0CTL.bit.ACQPS = 6;// set SOC0 S/H Window to 7 ADC Clock Cycles, (6 ACQPS plus 1)
	//AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; (�D���n)
	EDIS;

	InitCpuTimers();  // ��l�� cpuTimer 
	ConfigCpuTimer(&CpuTimer0, 1, 3906); // !!!!!!!!!!!!!!!!!!!!!!!!!!�������t�m!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	EALLOW;
	CpuTimer0Regs.TCR.all = 0x4000;   // !!!!!!!!!!!!!!!!!!!!!!!!!!!�A�P��CPUTIMER0!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 
	EDIS;


	for (;;) {
		LoopCount++;
	}

}

__interrupt void xint_isr(void) {
	if (xintcount == 0) {  // xintcount �ΨӧP�_�ثe�O���t�ΫG 
		xintcount++;
	} else {
		xintcount = 0;
	}
	if (xintcount == 0) {   
		PieCtrlRegs.PIEIER1.bit.INTx1 = 0; // Disable ADC convertion 
		GpioDataRegs.GPACLEAR.bit.GPIO1 = 1; // �����O�� 
	} else {

		PieCtrlRegs.PIEIER1.bit.INTx1 = 1; // Enable ADC convertion 
		GpioDataRegs.GPASET.bit.GPIO1 = 1; // �}�ҿO�� 
	}

	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;   // Acknowledge interrupt to PIE   !!!!!!!�@�w�n�� �_�h�u�|���_�@�� !!!!!! 
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

interrupt void adc_isr(void) {

	Voltage1[ConversionCount] = AdcResult.ADCRESULT0;

// If 20 conversions have been logged, start over
	if (ConversionCount == 255) {
		ConversionCount = 0;
	} else
		ConversionCount++;

	AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //Clear ADCINT1 flag reinitialize for next SOC
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;   // Acknowledge interrupt to PIE

	return;
}



/////////////////////////////////����////////////////////////////////////////

// cpuTimer ���_�b���D���i�P Xint ���_�P�ɶ}�� , �]�����W�v�ӧ� , �P�ɶ}�� Xint �N�|�L�k�i���_ 
