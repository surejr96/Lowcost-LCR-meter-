// Low Cost LCR Meter
//Project done by Surej Rajkumar
//
//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------
// Target Platform: EK-TM4C123GXL Evaluation Board
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz
// Hardware configuration:
// Red Backlight LED:
//   PB5 drives an NPN transistor that powers the red LED
// Green Backlight LED:
//   PE4 drives an NPN transistor that powers the green LED
// Blue Backlight LED:
//   PE5 drives an NPN transistor that powers the blue LED
// Red LED:
//   PF1 drives an NPN transistor that powers the red LED
// Green LED:
//   PF3 drives an NPN transistor that powers the green LED
// Pushbutton:
//   SW1 pulls pin PF4 low (internal pull-up is used)
// UART Interface:
//   U0TX (PA1) and U0RX (PA0) are connected to the 2nd controller
//   The USB on the 2nd controller enumerates to an ICDI interface and a virtual COM port
//   Configured to 115,200 baud, 8N1
//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <hw_nvic.h>
#include <hw_types.h>
#include <stdio.h>
#include <ctype.h>
#include "tm4c123gh6pm.h"
#include <hw_nvic.h>
#include <hw_types.h>
#define INTEGRATE (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 1*4)))//PE1
#define LOWSIDER  (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 2*4)))//PE2
#define MEASC     (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 3*4)))//PE3
#define HIGHSIDER (*((volatile uint32_t *)(0x42000000 + (0x400073FC-0x40000000)*32 + 2*4)))//PD2
#define MEASLR    (*((volatile uint32_t *)(0x42000000 + (0x400073FC-0x40000000)*32 + 3*4)))//PD3
#define GREEN_LED (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 3*4)))
#define RED_LED   (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 1*4)))
#define BLUE_LED  (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 2*4)))
//#define PUSH_BUTTON  (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 4*4)))
//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------
// Initialize Hardware
void initHw()
{
    // Configure HW to work with 16 MHz XTAL, PLL enabled, system clock of 40 MHz
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);
    // Set GPIO ports to use APB (not needed since default configuration -- for clarity)
    // Note UART on port A must use APB
    SYSCTL_GPIOHBCTL_R = 0;
    // Enable GPIO port A and F peripherals
    SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOA | SYSCTL_RCGC2_GPIOF | SYSCTL_RCGC2_GPIOD | SYSCTL_RCGC2_GPIOE |SYSCTL_RCGC2_GPIOC;
    // Configure LED and pushbutton pins
    GPIO_PORTF_DIR_R = 0x0A;  // bits 1 and 3 are outputs, other pins are inputs
    GPIO_PORTF_DR2R_R = 0x0A; // set drive strength to 2mA (not needed since default configuration -- for clarity)
    GPIO_PORTF_DEN_R = 0x1A;  // enable LEDs and pushbuttons
    GPIO_PORTF_PUR_R = 0x10;  // enable internal pull-up for push button
    // CONFIGURE PE
    GPIO_PORTE_DIR_R = 0x0E;  // bits 1,2 and 3 are outputs, other pins are inputs
    GPIO_PORTE_DR2R_R = 0x0E; // set drive strength to 2mA (not needed since default configuration -- for clarity)
    GPIO_PORTE_DEN_R = 0x0E;  // enable
    // CONFIGURE PD
    GPIO_PORTD_DIR_R = 0x0C;  // bits 2 and 3 are outputs, other pins are inputs
    GPIO_PORTD_DR2R_R =0x0C; // set drive strength to 2mA (not needed since default configuration -- for clarity)
    GPIO_PORTD_DEN_R = 0x0C;  // enable
    // Configure AN0 as an analog input
       SYSCTL_RCGCADC_R |=1;                           // turn on ADC module 0 clocking
       GPIO_PORTE_AFSEL_R |= 0x10;                      // select alternative functions for AN0 (PE3)
       GPIO_PORTE_DEN_R &= ~0x10;                       // turn off digital operation on pin PE3
       GPIO_PORTE_AMSEL_R |= 0x10;                      // turn on analog operation on pin PE3
       ADC0_CC_R = ADC_CC_CS_SYSPLL;                    // select PLL as the time base (not needed, since default value)
       ADC0_ACTSS_R &= ~ADC_ACTSS_ASEN3;                // disable sample sequencer 3 (SS3) for programming
       ADC0_EMUX_R = ADC_EMUX_EM3_PROCESSOR;            // select SS3 bit in ADCPSSI as trigger
       ADC0_SSMUX3_R = 0X08;                               // set first sample to AN0
       ADC0_SSCTL3_R = ADC_SSCTL3_END0;                 // mark first sample as the end
       ADC0_ACTSS_R |= ADC_ACTSS_ASEN3;                 // enable SS3 for operation
       SYSCTL_RCGCADC_R |=2;
       GPIO_PORTE_AFSEL_R |= 0x20;                      // select alternative functions for AN0 (PE3)
       GPIO_PORTE_DEN_R &= ~0x20;                       // turn off digital operation on pin PE3
       GPIO_PORTE_AMSEL_R |= 0x20;                      // turn on analog operation on pin PE3
       ADC1_CC_R = ADC_CC_CS_SYSPLL;                    // select PLL as the time base (not needed, since default value)
       ADC1_ACTSS_R &= ~ADC_ACTSS_ASEN3;                // disable sample sequencer 3 (SS3) for programming
       ADC1_EMUX_R = ADC_EMUX_EM3_PROCESSOR;            // select SS3 bit in ADCPSSI as trigger
       ADC1_SSMUX3_R = 0X09;                               // set first sample to AN0
       ADC1_SSCTL3_R = ADC_SSCTL3_END0;                 // mark first sample as the end
       ADC1_ACTSS_R |= ADC_ACTSS_ASEN3;
    // Configure UART0 pins
    SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R0;         // turn-on UART0, leave other uarts in same status
    GPIO_PORTA_DEN_R |= 3;                           // default, added for clarity
    GPIO_PORTA_AFSEL_R |= 3;                         // default, added for clarity
    GPIO_PORTA_PCTL_R = GPIO_PCTL_PA1_U0TX | GPIO_PCTL_PA0_U0RX;
    // Configure UART0 to 115200 baud, 8N1 format (must be 3 clocks from clock enable and config writes)
    UART0_CTL_R = 0;                                 // turn-off UART0 to allow safe programming
    UART0_CC_R = UART_CC_CS_SYSCLK;                  // use system clock (40 MHz)
    UART0_IBRD_R = 21;                               // r = 40 MHz / (Nx115.2kHz), set floor(r)=21, where N=16
    UART0_FBRD_R = 45;                               // round(fract(r)*64)=45
    UART0_LCRH_R = UART_LCRH_WLEN_8 | UART_LCRH_FEN; // configure for 8N1 w/ 16-level FIFO
    UART0_CTL_R = UART_CTL_TXE | UART_CTL_RXE | UART_CTL_UARTEN; // enable TX, RX, and module
    //Wide Timer
    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R5;     // turn-on timer
    WTIMER5_CTL_R &= ~TIMER_CTL_TAEN;                // turn-off counter before reconfiguring
    WTIMER5_CFG_R = 4;                               // configure as 32-bit counter (A only)
    WTIMER5_TAMR_R = TIMER_TAMR_TACMR | TIMER_TAMR_TAMR_CAP | TIMER_TAMR_TACDIR; // configure for edge time mode, count up
    WTIMER5_CTL_R = TIMER_CTL_TAEVENT_POS;           // measure time from positive edge to positive edge
    WTIMER5_IMR_R = TIMER_IMR_CAEIM;                 // turn-on interrupts
           //WTIMER5_TAV_R = 0;                               // zero counter for first period
           //WTIMER5_CTL_R |= TIMER_CTL_TAEN;                 // turn-on counter
//comp pc7
              GPIO_PORTC_AFSEL_R = 0x80;
              GPIO_PORTC_AMSEL_R = 0x80;
              GPIO_PORTF_DEN_R &= ~0x80;
              SYSCTL_RCGCACMP_R=1;
              COMP_ACRIS_R|=COMP_ACRIS_IN0;
              COMP_ACINTEN_R|=COMP_ACINTEN_IN0;
              COMP_ACREFCTL_R=0x20F;
              COMP_ACCTL0_R=0x40C;
}

// Blocking function that writes a serial character when the UART buffer is not full
void putcUart0(char c)
{
    while (UART0_FR_R & UART_FR_TXFF);
    UART0_DR_R = c;
}

int16_t readAdc0Ss3()
{
    ADC0_PSSI_R |= ADC_PSSI_SS3;                     // set start bit
    while (ADC0_ACTSS_R & ADC_ACTSS_BUSY);           // wait until SS3 is not busy
    return ADC0_SSFIFO3_R;                           // get single result from the FIFO
}
int16_t readAdc1Ss3()
{
    ADC1_PSSI_R |= ADC_PSSI_SS3;                     // set start bit
    while (ADC1_ACTSS_R & ADC_ACTSS_BUSY);           // wait until SS3 is not busy
    return ADC1_SSFIFO3_R;                           // get single result from the FIFO
    }
// Blocking function that writes a string when the UART buffer is not full
void putsUart0(char* str)
{
    uint8_t i;
    for (i = 0; i < strlen(str); i++)
      putcUart0(str[i]);
}
// Blocking function that returns with serial data once the buffer is not empty
char getcUart0()
{
    while (UART0_FR_R & UART_FR_RXFE);
    return UART0_DR_R & 0xFF;
}
void waitMicrosecond(uint32_t us)
{
    __asm("WMS_LOOP0:   MOV  R1, #6");          // 1
    __asm("WMS_LOOP1:   SUB  R1, #1");          // 6
    __asm("             CBZ  R1, WMS_DONE1");   // 5+1*3
    __asm("             NOP");                  // 5
    __asm("             NOP");                  // 5
    __asm("             B    WMS_LOOP1");       // 5*2 (speculative, so P=1)
    __asm("WMS_DONE1:   SUB  R0, #1");          // 1
    __asm("             CBZ  R0, WMS_DONE0");   // 1
    __asm("             NOP");                  // 1
    __asm("             B    WMS_LOOP0");       // 1*2 (speculative, so P=1)
    __asm("WMS_DONE0:");                        // ---
                                                // 40 clocks/us + error
}
//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
char str[80];
uint64_t tcons;
char tc[80],re[80],es[80];
double capi;
uint64_t res;
double capid;
double resis;
float res1;
char vol0[20],vol1[20];
float raw0,raw1,voltage0,voltage1,diff,esrv;
void cisr()
{
    tcons=WTIMER5_TAV_R;
    COMP_ACMIS_R|=COMP_ACMIS_IN0;

}
int voltage()
{
             raw0 = readAdc0Ss3();
             raw1= readAdc1Ss3();
             voltage0 = ((raw0 / 4096.0 * 3.3));
             voltage1 = ((raw1 / 4096.0 * 3.3));
             diff=voltage1-voltage0;
             putsUart0("\n \rdut1 ");
             sprintf(vol0, "%3.1f", voltage0);
             putsUart0(vol0);
             putsUart0("\n \rdut2 ");
             sprintf(vol1, "%3.1f", voltage1);
             putsUart0(vol1);
             putsUart0("\n \r");
             putsUart0("voltage is");
             sprintf(str, "%3.1f", diff);
             RED_LED=1;
             waitMicrosecond(500000);
             RED_LED=0;;
             putsUart0("\n \r");
             putsUart0(str);
             putsUart0("\r");
             return 0;
}
int reset()
{            putsUart0("reset");
             putsUart0("\n");
             putsUart0("\r");
             HWREG(NVIC_APINT)= NVIC_APINT_VECTKEY|NVIC_APINT_SYSRESETREQ;
             return 0;
}
int resistor()
{       putsUart0("Measuring resistance ");
        INTEGRATE=1;
        MEASLR=0;
        MEASC=0;
        HIGHSIDER=0;
        LOWSIDER=1;
        waitMicrosecond(5000000);
        LOWSIDER=0;
        MEASLR=1;
        WTIMER5_TAV_R = 0;                               // zero counter for first period
        WTIMER5_CTL_R |= TIMER_CTL_TAEN;                 // turn-on counter
        NVIC_EN0_R = (1 << (INT_COMP0-16));         // turn-on interrupt 41  Analog Comparator 0
        waitMicrosecond(5000000);
        NVIC_EN0_R &=~ (1 << (INT_COMP0-16));
        if(tcons<2500 && tcons>1000)
            {
                res=(9*tcons);
                resis=(res/172);
                RED_LED=1;
                waitMicrosecond(1000000);
                RED_LED=0;
                //resis=resis-0.98;
                sprintf(re, "%3.1lf", resis);
                putsUart0("resistance is ");
                putsUart0(re);
                putsUart0(" Ohms ");
            }
        else if (tcons<1000)
                    {
                        res1=(15*tcons);
                        resis=(res1/1378);
                        resis=resis+2.0538;
                        RED_LED=1;
                        waitMicrosecond(1000000);
                        RED_LED=0;
                        //resis=resis-0.98;
                        sprintf(re, "%3.1lf", resis);
                        putsUart0("resistance is ");
                        putsUart0(re);
                        putsUart0(" Ohms ");
                    }
        else
            {
                res=(22500*tcons);
                resis=(res/1101851);
                RED_LED=1;
                waitMicrosecond(500000);
                RED_LED=0;
                sprintf(re, "%3.1lf", resis);
                putsUart0("\n");
                putsUart0("\r");
                putsUart0("resistance is ");
                putsUart0(re);
                putsUart0(" Ohms ");
        }
        putsUart0("\n");
        putsUart0("\r");
        return 0;
}
int cap()
{
        putsUart0("Measuring capacitance \n  \r");
        INTEGRATE=0;
        MEASLR=0;
        MEASC=1;
        HIGHSIDER=0;
        LOWSIDER=1;
        waitMicrosecond(5999999);
        LOWSIDER=0;
        MEASC=1;
        HIGHSIDER=1;
        WTIMER5_TAV_R = 0;                               // zero counter for first period
        WTIMER5_CTL_R |=  TIMER_CTL_TAEN;                 // turn-on counter
        NVIC_EN0_R = (1 << (INT_COMP0-16));         // turn-on interrupt 41  Analog Comparator 0
        waitMicrosecond(15000000);
        NVIC_EN0_R &= ~(1 << (INT_COMP0-16));
        if (tcons>5000000)
            {
                capi=(3*tcons);
                capid=(capi/17276977);
                sprintf(tc, "%3.1lf", capid);
                putsUart0("capacitance is ");
                putsUart0(tc);
                putsUart0(" micro Farads ");
                RED_LED=1;
                waitMicrosecond(500000);
                RED_LED=0;
            }
        else if (tcons>7000)
            {
                   capi=(3*tcons);
                   capid=(capi/16673);
                   RED_LED=1;
                   waitMicrosecond(500000);
                   RED_LED=0;
                   sprintf(tc, "%3.1lf", capid);
                   putsUart0("capacitance is ");
                   putsUart0(tc);
                   putsUart0(" nano Farads ");
            }
    putsUart0("\n");
    putsUart0("\r");
    return 0;
}

int ind()
{
        int indc;
        float inh;
        putsUart0("Measuring Inductance \n  \r");
        LOWSIDER=1;
        MEASLR=0;
        MEASC=0;
        INTEGRATE=1;
        HIGHSIDER=0;
        waitMicrosecond(100000);
        INTEGRATE=0;
        LOWSIDER=1;
        MEASLR=1;
        MEASC=0;
        HIGHSIDER=0;
        WTIMER5_TAV_R = 0;                            // zero counter for first period
        WTIMER5_CTL_R |= TIMER_CTL_TAEN;             // turn-on counter
        NVIC_EN0_R = (1 << (INT_COMP0-16));         // turn-on interrupt 41  Analog Comparator 0
        waitMicrosecond(500000);
        NVIC_EN0_R &= ~(1 << (INT_COMP0-16));
        putsUart0("inductance is ");
        if (tcons>=110)
            {
                indc=(371*tcons);
                inh=(indc/500);
            }
        else
            {
                indc=(371*tcons);
                inh=(indc/512);
                inh=inh-5.093;
            }
        RED_LED=1;
        waitMicrosecond(500000);
        RED_LED=0;
        sprintf(tc, "%3.1lf", inh);
        putsUart0(tc);
        putsUart0(" Henry ");
        putsUart0("\n");
        putsUart0("\r");
        return 0;
}

int esr()
{
           MEASLR=1;
           MEASC=0;
           LOWSIDER=1;
           waitMicrosecond(2000000);
           WTIMER5_CTL_R |= TIMER_CTL_TAEN;
           raw0 = readAdc0Ss3();
           raw1= readAdc1Ss3();
           voltage0 = ((raw0 / 4096.0 * 3.3));
           voltage1 = ((raw1 / 4096.0 * 3.3));
           diff=voltage1-voltage0;
           esrv=-33*diff;
           esrv=esrv/voltage1;
           sprintf(es, "%3.3f", esrv);
           putsUart0("\n\r");
           putsUart0("Series resistance");
           putsUart0(es);
           MEASLR=0;
           return 0;
}

// TO CLEAR THE STRING
void bufclr()
{
    int o;
    for (o=0;o<=80;o++)
    {
        str[o]='\0';
    }
}
int main(void)
{
// Initialize hardware
initHw();
GREEN_LED=1;
waitMicrosecond(500000);
GREEN_LED=0;
while(1)
{
    putsUart0("\n \rLow cost LCR Meter \r\n");
    char ch,ch1;
    int i=0;
    putsUart0("Enter String \r\n");
    while(i<80)
             {
                 ch = getcUart0();
                 if(ch==0x08) //BP
                       {
                          if (i>0)
                              {
                                --i;
                              }
                       }
                 else if  (ch==0x0D) //enter
                       {
                           str[i]=0;
                           break;
                        }
                 else if(ch>20)
                       {
                ch1=tolower(ch);
                str[i]=ch1;
                i=i+1;
                       }
             }
 // step 3
     int j,k,flag,count;
     flag=0;
     k=strlen(str);
     putsUart0("\n");
     putsUart0("\r");
     char a[3];
     int b[3];
     int p=0;
     count=0;
 for (j=0;j<k;j++)
     {
         if (isspace (str[j]))
                            {
                                 flag=0;
                                 str[j]='\0';
                                 j--;
                            }
     else if (isalpha(str[j]) && (flag==0))
                          {
                             a[p]='a';
                             flag=1;
                             b[p]= j;
                             p++;
                             count++;
                          }
     else if (isdigit(str[j]) && (flag==0))
                             {
                                  flag=1;
                                  b[p]= j; // pos vect
                                  a[p]='n'; // a or n
                                  p++;
                                  count++;
                              }
     else if (ispunct(str[j]))
                                {
                                    str[j]='\0';
                                    flag=0;
                                    b[p]= j;
                                    a [p]=str[j];
                                    p++;
                                    j--;
                               }
      }
   putsUart0("\n");
// step 4
   if (strcmp ("set", &str[b[0]]) == 0)
   {
       putsUart0("valid");
       putsUart0("\n");
       putsUart0("\r");
       if (strcmp ("measlr", &str[b[1]]) == 0 && (strcmp ("on", &str[b[2]]) == 0))
           {
               putsUart0("valid measlr PD3");//PD2
               MEASLR=1;
               HIGHSIDER=0;
               LOWSIDER=0;
               MEASC=0;
               INTEGRATE=0;
              RED_LED=1;
           }
       else if (strcmp ("measlr", &str[b[1]]) == 0 && (strcmp ("off", &str[b[2]]) == 0))
                  {
                      putsUart0("off measlr");//PD2
                      MEASLR=0;
                      putsUart0("\n");
                      putsUart0("\r");
                  }
       else if (strcmp ("measc", &str[b[1]]) == 0 && (strcmp ("on", &str[b[2]]) == 0))
                  {
                      putsUart0("valid measc PE3");
                      GREEN_LED=1;
                      MEASC=1;
                      MEASLR=0;
                      HIGHSIDER=0;
                      LOWSIDER=0;
                      INTEGRATE=0;
                      putsUart0("\n");
                      putsUart0("\r");
                  }
       else if (strcmp ("measc", &str[b[1]]) == 0 && (strcmp ("off", &str[b[2]]) == 0))
                         {
                             putsUart0("off measc");
                             MEASC=0;
                             putsUart0("\n");
                             putsUart0("\r");
                         }
       else  if (strcmp ("lowsider", &str[b[1]]) == 0 && (strcmp ("on", &str[b[2]]) == 0))
                  {
                      putsUart0("valid lowside PE2");
                      GREEN_LED=1;
                      LOWSIDER=1;
                      MEASLR=0;
                      HIGHSIDER=0;
                      MEASC=0;
                      INTEGRATE=0;
                      putsUart0("\n");
                      putsUart0("\r");
                  }
       else  if (strcmp ("lowsider", &str[b[1]]) == 0 && (strcmp ("off", &str[b[2]]) == 0))
                         {
                             putsUart0("off lowside");
                             LOWSIDER=0;
                             putsUart0("\n");
                             putsUart0("\r");
                         }
       else if (strcmp ("highsider", &str[b[1]]) == 0 && (strcmp ("on", &str[b[2]]) == 0))
                  {
                      putsUart0("valid highsider PD2");//PD3
                      HIGHSIDER=1;
                      MEASLR=0;
                      LOWSIDER=0;
                      MEASC=0;
                      INTEGRATE=0;
                      GREEN_LED=1;
                      putsUart0("\n");
                      putsUart0("\r");
                  }
       else if (strcmp ("highsider", &str[b[1]]) == 0 && (strcmp ("off", &str[b[2]]) == 0))
                        {
                            putsUart0("off highsider");//PD3
                            HIGHSIDER=0;
                            putsUart0("\n");
                            putsUart0("\r");
                        }
       else if (strcmp ("integrate", &str[b[1]]) == 0 && (strcmp ("on", &str[b[2]]) == 0))
                  {
                      putsUart0("valid integrate PE1");
                      INTEGRATE=1;
                      MEASLR=0;
                      HIGHSIDER=0;
                      LOWSIDER=0;
                      MEASC=0;
                      GREEN_LED=1;
                      putsUart0("\n");
                      putsUart0("\r");
                  }
       else if (strcmp ("integrate", &str[b[1]]) == 0 && (strcmp ("off", &str[b[2]]) == 0))
                         {
                             putsUart0("off integrate");
                             INTEGRATE=0;
                             putsUart0("\n");
                             putsUart0("\r");
                         }
           else
           {
               putsUart0("invalid ");
               putsUart0("\n");
               putsUart0("\r");
           }
   }
   else if (strcmp ("resistor", &str[b[0]]) == 0)
                          {
                             resistor();
                          }
   else if (strcmp ("capacitor", &str[b[0]]) == 0)
                             {
                                cap();
                             }
   else if (strcmp ("inductor", &str[b[0]]) == 0)
                             {
                                ind();
                             }
   else if (strcmp ("esr", &str[b[0]]) == 0)
                                {
                                  esr();
                                }
   else if (strcmp ("voltage", &str[b[0]]) == 0)
                            {
                              voltage();
                            }
   else if (strcmp ("reset", &str[b[0]]) == 0)
                           {
                              reset();
                           }
   else
   {
       putsUart0("invalid");
       putsUart0("\n");
       putsUart0("\r");
   }
bufclr();
    }
}


