#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h> 

// a pointer to this is a null pointer, but the compiler does not
// know that because "sram" is a linker symbol from sections.lds.
extern uint32_t sram;

#define reg_spictrl (*(volatile uint32_t *)0x02000000)
#define reg_uart_clkdiv (*(volatile uint32_t *)0x02000004)
#define reg_uart_data (*(volatile uint32_t *)0x02000008)
#define clock (*(volatile uint32_t *)0x03000200)
#define reg_leds (*(volatile uint32_t *)0x03000100)
#define gpio (*(volatile uint32_t *)0x03000000)

extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss, _heap_start;

uint32_t set_irq_mask(uint32_t mask);
asm(
    ".global set_irq_mask\n"
    "set_irq_mask:\n"
    ".word 0x0605650b\n"
    "ret\n");

void main()
{
    set_irq_mask(0xff);

    // zero out .bss section
    for (uint32_t *dest = &_sbss; dest < &_ebss;)
    {
        *dest++ = 0;
    }

    float clockA = clock;
    reg_leds = 0;
    while (1)
    {
        if (clockA + 16000 < clock && reg_leds == 0)
        {
            clockA = clock;
            reg_leds = ((int)(cos(M_PI)));
        }
        else if (clockA + 16000 < clock && reg_leds == -1)
        {
            clockA = clock;
            reg_leds = 0;
        }
    }
}