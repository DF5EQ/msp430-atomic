# msp-atomic

These macros and functions are inspired by and based on util/atomic.h for avr-gcc written by Dean Camera.

## Copyright

Copyright (c) 2022 Peter Bägel (DF5EQ)  
Details see LICENSE file

## Original copyright from avr-gcc util/atomic.h

Copyright (c) 2007 Dean Camera  
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
                                  
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

* Neither the name of the copyright holders nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

## Changes
                                                                  
* no use of for loop  
* block to be surrounded by () instead of {}  
* four separate macros instead of two with parameters  

|original                               |modified                    |
|---------------------------------------|----------------------------|
|ATOMIC_BLOCK(ATOMIC_RESTORESTATE)      |ATOMIC_BLOCK_RESTORESTATE   | 
|ATOMIC_BLOCK(ATOMIC_FORCEON)           |ATOMIC_BLOCK_FORCEON        |
|NONATOMIC_BLOCK(NONATOMIC_RESTORESTATE)|NONATOMIC_BLOCK_RESTORESTATE|
|NONATOMIC_BLOCK(NONATOMIC_FORCEOFF)    |NONATOMIC_BLOCK_FORCEOFF    |
                                                                          
## From original notes, adapted to MSP430
                                                                          
The macros in this header file deal with code blocks that are guaranteed to be excuted atomically or non-atmomically. The term "atomic" in this context refers to the unability of the respective code to be interrupted.

These macros operate via automatic manipulation of the General Interrupt Enable (GIE) bit of the SR register. Exit paths from both block types are all managed automatically without the need for special considerations, i. e. the interrupt status will be restored to the same   value it has been when entering the respective block.

A typical example that requires atomic access is a 32 (or more) bit variable that is shared between the main execution path and an ISR. While declaring such a variable as volatile ensures that the compiler will not optimize accesses to it away, it does not guarantee atomic access to it. Assuming the following example:

    volatile uint32_t ctr;

    ISR()
    {
        ctr--;
    }

    ...
    int main(void)
    {
        ...
        ctr = 0x00020000;
        start_timer();
        __eint();
        while (ctr != 0)
            // wait                                                           
            ;                                                               
        ...
    }
                                                                          
There is a chance where the main context will exit its wait loop when the variable ctr just reached the value 0x0000FFFF.  This happens because the compiler cannot natively access a 32-bit variable atomically in an 16-bit CPU. So the variable is for example at 0x00010000, the compiler then tests the low word for 0, which succeeds. It then proceeds to test the high word, but that moment the ISR triggers, and the main context is interrupted. The ISR will decrement the variable from 0x00010000 to 0x0000FFFF, and the main context proceeds. It now tests the high word of the variable which is (now) also 0, so it concludes the variable has reached 0, and terminates the loop.
 
Using the macros, the above code can be rewritten: 
                                                                          
    #include "atomic.h"

    volatile uint32_t ctr;

    ISR()
    {
        ctr--;
    }

    ...
    int main(void)
    {
        ...
       ctr = 0x00020000;
        start_timer();
        __eint();
        uint32_t ctr_copy;
        do
        {
            ATOMIC_BLOCK_FORCEON                                                 
            (
                ctr_copy = ctr;
            )
        }
        while (ctr_copy != 0);
        ...
    }

This will install the appropriate interrupt protection before accessing variable ctr, so it is guaranteed to be consistently tested. If the global interrupt state were uncertain before entering the ATOMIC BLOCK, ATOMIC_BLOCK_RESTORESTATE should be used rather than ATOMIC_BLOCK_FORCEON.

## Provided example

The provided example in main() is modified contrary to the above discussed example to show the running interrupt and the consequences of blocking/non-blocking with LEDs.

The blocking/non-blocking parts can be activated/deactivated with #if 1 / #if 0

# Development environment

## Hardware
TI Launchpad EXP430FR5969

## Compiler
msp430-gcc (GCC) 4.6.3 20120301 (mspgcc LTS 20120406 unpatched)  
Used compiler flags and options see makefile

