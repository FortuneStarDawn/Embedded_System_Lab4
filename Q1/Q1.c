#include <msp430.h>
#include <intrinsics.h>
#define LED1 BIT0
#define LED2 BIT6
#define B1 BIT3

volatile float v=0, c=0;
volatile unsigned int mode=0, fast=0;

void main(void)
{
    WDTCTL = WDTPW + WDTHOLD; // Stop WDT
    // TA1 trigger sample start
    ADC10CTL1 = INCH_10;
    ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + ADC10IE;
    __delay_cycles(1000);
    ADC10CTL0 |= ENC;    // ADC10 Enable

    P1DIR = LED1 + LED2;
    P1OUT = LED1 + B1;
    P1REN = B1;
    P1IE |= B1;
    P1IES |= B1;
    P1IFG &= ~B1;
    _BIS_SR(GIE);

    BCSCTL3 |= LFXT1S_2;
    TA0CCR0 = 449;
	TA0CCTL0 = CCIE;
	TA0CTL = MC_1|ID_3|TASSEL_1|TACLR; //1500 Hz
	__enable_interrupt();

    while(1);
 }
// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
	ADC10CTL0 &= ~ADC10SC;
	v = 1.5 * ADC10MEM / 1024;
	c = (v - 0.986) / 0.00355;
	if(v>=1.125) fast = 1;
	else fast = 0;
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	P1IFG &= ~B1;
	if(P1IES & B1) ADC10CTL0 |= ADC10SC;
	else
	{
		if(mode==0)
		{
			mode = 1;
			P1OUT &= ~LED1;
			P1OUT |= LED2;
		}
		else
		{
			mode = 0;
			P1OUT &= ~LED2;
			P1OUT |= LED1;
		}
		TA0CCR0 = 449;
		TA0CTL |= TACLR;
	}
	P1IES ^= B1;
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_ISR (void)
{
	if(mode==0) P1OUT ^= LED1;
	else P1OUT ^= LED2;
	if(TA0CCR0==449)
	{
		if(fast) TA0CCR0 = 299;
		else TA0CCR0 = 1049;
	}
	else TA0CCR0 = 449;
	TA0CTL |= TACLR;
}
