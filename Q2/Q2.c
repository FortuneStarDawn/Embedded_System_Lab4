#include <msp430.h>
#include <intrinsics.h>
#define LED1 BIT6
#define LED2 BIT0
#define B1 BIT3

volatile unsigned int mode=0, last=0, now=0;

void main(void)
{
    WDTCTL = WDTPW + WDTHOLD; // Stop WDT
    // TA1 trigger sample start
    ADC10CTL1 = INCH_10 + SHS_1 + CONSEQ_2;
    ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + ADC10IE;
    __delay_cycles(1000);

    ADC10CTL0 &= ~ENC;

    P1DIR = LED1 + LED2;
    P1OUT = B1;
    P1REN = B1;
    P1IE |= B1;
    P1IES |= B1;
    P1IFG &= ~B1;
    _BIS_SR(GIE);

    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ; // DCO = 1Mz
    BCSCTL2 |= DIVS_3;
    BCSCTL3 |= LFXT1S_2;

    TA0CCR0 = 2999;
	TA0CCTL1 = OUTMOD_3;
	TA0CCR1 = 2998;
	TA0CTL = MC_1|ID_3|TASSEL_1|TACLR; //1500 Hz

	TA1CCTL0 = CCIE;
	TA1CCR0 = 449;
	TA1CTL = MC_1|ID_3|TASSEL_1|TACLR; //1500 Hz
	__enable_interrupt();

    while(1);
 }
// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
	now = ADC10MEM;
	if(mode==1)
	{
		if(now>last)
		{
			mode = 2;
			P1OUT &= ~LED1;
			P1OUT |= LED2;
			TA1CCR0 = 3124;
			TA1CTL = MC_1|ID_3|TASSEL_2|TACLR;
		}
	}
	if(mode==2)
	{
		if(now<=last)
		{
			mode = 1;
			P1OUT |= LED1;
			P1OUT &= ~LED2;
			TA1CCR0 = 449;
			TA1CTL = MC_1|ID_3|TASSEL_1|TACLR;
		}
	}
	last = now;
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	P1IFG &= ~B1;
	if(P1IES & B1)
	{
		mode = 1;
		P1OUT |= LED1;
		TA0CTL |= TACLR;
		TA1CCR0 = 449;
		TA1CTL = MC_1|ID_3|TASSEL_1|TACLR;
	    ADC10CTL0 |= ENC;
	}
	else
	{
		mode = 0;
		P1OUT &= ~LED1;
		P1OUT &= ~LED2;
		ADC10CTL0 &= ~ENC;
	}
	P1IES ^= B1;
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR (void)
{
	if(mode==1)
	{
		P1OUT ^= LED1;
		if(TA1CCR0==449) TA1CCR0 = 1049;
		else TA1CCR0 = 449;
	}
	if(mode==2)
	{
		P1OUT ^= LED2;
		if(TA1CCR0==3124) TA1CCR0 = 12499;
		else TA1CCR0 = 3124;
	}
	TA1CTL |= TACLR;
}
