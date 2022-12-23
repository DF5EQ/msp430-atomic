/* ===== file header ===== */

/* ===== includes ===== */
#include <msp430.h>
#include <stdint.h>
#include "atomic.h"

/* ===== private datatypes ===== */

/* ===== private symbols ===== */
#define LED_RED_OUT   P4OUT
#define LED_GREEN_OUT P1OUT

#define LED_RED_DIR   P4DIR
#define LED_GREEN_DIR P1DIR

#define LED_RED_BIT   BIT6
#define LED_GREEN_BIT BIT0

#define TIMER_PERIODE 1000 /* us */

#define SYSTEM_DCOCLK 16000000
#define SYSTEM_DIVM   1
#define SYSTEM_DIVS   16
#define SYSTEM_MCLK   (SYSTEM_DCOCLK/SYSTEM_DIVM)
#define SYSTEM_SMCLK  (SYSTEM_DCOCLK/SYSTEM_DIVS)

#define DIVTA1 1
#define TACLK  (SMCLK/DIVTA1) /* hz */
#define CYCLE  1e-3           /*  s */
#define OFFSET ((unsigned int)(TACLK*CYCLE))

/* ===== private constants ===== */

/* ===== public constants ===== */

/* ===== private variables ===== */

/* simulated shared word */
static volatile uint16_t shared_high_word;
static volatile uint16_t shared_low_word;

/* ===== public variables ===== */

/* ===== private functions ===== */

/* ===== interrupt functions ===== */
#pragma vector=TIMER3_A0_VECTOR
__interrupt void Timer3_A0 (void)
{
    static unsigned led_counter = 0;

    /* next interrupt in TIMER_PERIODE us */
    TA3CCR0 = TIMER_PERIODE;

    /* toggle LED every 1000 interrupts */
    led_counter++;
    if(led_counter == 1000)
    {
        led_counter = 0;
        LED_RED_OUT ^= LED_RED_BIT;
    }

    /* modify the shared word */
    shared_high_word = 0x0000;
    shared_low_word  = 0xffff;
}

/* ===== public functions ===== */

int main(void)
{
    /* disable watchdog */
    WDTCTL = WDTPW + WDTHOLD;

    /* unlock ports */
    PM5CTL0 &= ~LOCKLPM5;

    /* initialise FRAM for 16MHz operation */
    FRCTL0   = FWPW;      /* unlock FR registers */
    FRCTL0_L = NACCESS_1; /* for MCLK = 16MHz we need one wait state for FRAM access */
    FRCTL0_H = 0;         /* lock FR registers */

    /* set MCLK = 16Mhz, SMCLK = 1MHz */
    CSCTL0   = CSKEY;                        /* unlock CS registers */
    CSCTL1   = DCOFSEL_4 | DCORSEL;          /* set DCO to 16MHz */
    CSCTL3   = DIVA__1 | DIVS__16 | DIVM__1; /* ACLK divider = 1, SMCLK divider = 16, MCLK divider = 1 */
    CSCTL0_H = 0;                            /* lock CS registers */

    /* initialise the leds */
    LED_RED_OUT    &= ~LED_RED_BIT;
    LED_RED_DIR    |=  LED_RED_BIT;
    LED_GREEN_OUT  &= ~LED_GREEN_BIT;
    LED_GREEN_DIR  |=  LED_GREEN_BIT;

    /* timer TA3 up mode with SMCLK as source and divider 1 */
    TA3CTL = TASSEL__SMCLK | ID__1 | MC__UP;
    TA3EX0 = TAIDEX_0;

    /* fist interrupt in 1ms */
    TA3CCR0 = TIMER_PERIODE; /* TIMER_PERIODE us at SMCLK 1MHz */

    /* enable CCR0 interrupt */
    TA3CCTL0 = CCIE;

    /* enable general interrupt */
    __bis_status_register(GIE);

#if 1 /* bad, interrupt in between possible */
    while (1)
    {
        shared_low_word  = 0x0000;
        shared_high_word = 0x0001;

        if(shared_low_word == 0x0000)
        {
            if(shared_high_word == 0x0000)
            {
                /* should never be reached */
                LED_GREEN_OUT ^= LED_GREEN_BIT;
                unsigned n = 60000;
                while(n--);
            }
        }
    }
#endif

#if 0 /* good, interrupt in between blocked */
    uint16_t copy_high_word;
    uint16_t copy_low_word;

    while (1)
    {
        shared_low_word  = 0x0000;
        shared_high_word = 0x0001;

        ATOMIC_BLOCK_FORCEON
        (
          copy_low_word  = shared_low_word;
          copy_high_word = shared_high_word;
        )

        if(copy_low_word == 0x0000)
        {
            if(copy_high_word == 0x0000)
            {
                /* should never be reached */
                LED_GREEN_OUT |= LED_GREEN_BIT;
                unsigned n = 60000;
                while(n--);
            }
        }
    }
#endif
}
