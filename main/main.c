/* ===== file header ===== */

/* ===== includes ===== */
#include <msp430.h>
#include <stdint.h>

/* ===== private datatypes ===== */

/* ===== private symbols ===== */
#define LED_RED_OUT   P4OUT
#define LED_GREEN_OUT P1OUT

#define LED_RED_DIR   P4DIR
#define LED_GREEN_DIR P1DIR

#define LED_RED_BIT   BIT6
#define LED_GREEN_BIT BIT0

/* ===== private constants ===== */

/* ===== public constants ===== */

/* ===== private variables ===== */

/* ===== public variables ===== */

/* ===== private functions ===== */

/* ===== callback functions ===== */

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

    while (1)
    {
        uint16_t counter;

        LED_RED_OUT   ^= LED_RED_BIT;
        LED_GREEN_OUT ^= LED_GREEN_BIT;

        /* waste time */
        counter = 60000;
        while(counter--);
    }
}
