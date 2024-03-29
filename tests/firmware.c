#include <stdint.h>
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
/////////////////
void putchar(char c)
{
	if (c == '\n')
		putchar('\r');
	reg_uart_data = c;
}

void print(const char *p)
{
	while (*p)
		putchar(*(p++));
}

void print_hex(uint32_t v, int digits)
{
	for (int i = 7; i >= 0; i--)
	{
		char c = "0123456789abcdef"[(v >> (4 * i)) & 15];
		if (c == '0' && i >= digits)
			continue;
		putchar(c);
		digits = i;
	}
}

void print_dec(uint32_t v)
{
	if (v >= 100)
	{
		print(">=100");
		return;
	}

	if (v >= 90)
	{
		putchar('9');
		v -= 90;
	}
	else if (v >= 80)
	{
		putchar('8');
		v -= 80;
	}
	else if (v >= 70)
	{
		putchar('7');
		v -= 70;
	}
	else if (v >= 60)
	{
		putchar('6');
		v -= 60;
	}
	else if (v >= 50)
	{
		putchar('5');
		v -= 50;
	}
	else if (v >= 40)
	{
		putchar('4');
		v -= 40;
	}
	else if (v >= 30)
	{
		putchar('3');
		v -= 30;
	}
	else if (v >= 20)
	{
		putchar('2');
		v -= 20;
	}
	else if (v >= 10)
	{
		putchar('1');
		v -= 10;
	}

	if (v >= 9)
	{
		putchar('9');
		v -= 9;
	}
	else if (v >= 8)
	{
		putchar('8');
		v -= 8;
	}
	else if (v >= 7)
	{
		putchar('7');
		v -= 7;
	}
	else if (v >= 6)
	{
		putchar('6');
		v -= 6;
	}
	else if (v >= 5)
	{
		putchar('5');
		v -= 5;
	}
	else if (v >= 4)
	{
		putchar('4');
		v -= 4;
	}
	else if (v >= 3)
	{
		putchar('3');
		v -= 3;
	}
	else if (v >= 2)
	{
		putchar('2');
		v -= 2;
	}
	else if (v >= 1)
	{
		putchar('1');
		v -= 1;
	}
	else
		putchar('0');
}

void reverse(char *str, int len)
{
	int i = 0, j = len - 1, temp;
	while (i < j)
	{
		temp = str[i];
		str[i] = str[j];
		str[j] = temp;
		i++;
		j--;
	}
}

// Converts a given integer x to string str[].  d is the number
// of digits required in output. If d is more than the number
// of digits in x, then 0s are added at the beginning.
int intToStr(int x, char str[], int d, int offset)
{
	int i = 0;
	do
	{
		str[i++] = (x % 10) + '0';
		x = x / 10;
	}while (x);

	// If number of digits required is more, then
	// add 0s at the beginning
	while (i < d)
		str[i++] = '0';

	if(offset>0){
		str[i++]='-';
	}
	reverse(str, i);
	str[i] = '\0';
	return i;
}
// --------------------------------------------------------
// Converts a floating point number to string.
void ftoa(float n2, char *res)
{
	int extra=0;
	float n=n2;
	if(n<0){
		extra=1;
		n=-n2;
	}
	// Extract integer part
	int ipart = (int)n;

	// Extract floating part
	float fpart = n - (float)ipart;

	// convert integer part to string
	int i = intToStr(ipart, res, 0,extra);

	// check for display option after point
	res[i] = '.'; // add dot

	// Get the value of fraction part upto given no.
	// of points after dot. The third parameter is needed
	// to handle cases like 233.007
	fpart = fpart * pow(10, 4);

	intToStr((int)fpart, res + i + 1, 4,0);
}

void printf(float n)
{
	char res[20];
	intToStr(n, res, 0,0);
	ftoa(n, res);
	print(res);
	print("\n");
}
///////////////////
void main()
{
	reg_uart_clkdiv = 139;

	set_irq_mask(0xff);

	// zero out .bss section
	for (uint32_t *dest = &_sbss; dest < &_ebss;)
	{
		*dest++ = 0;
	}

	// switch to dual IO mode
	reg_spictrl = (reg_spictrl & ~0x007F0000) | 0x00400000;

    print("\n");
	print("  ____  _          ____         ____\n");
	print(" |  _ \\(_) ___ ___/ ___|  ___  / ___|\n");
	print(" | |_) | |/ __/ _ \\___ \\ / _ \\| |\n");
	print(" |  __/| | (_| (_) |__) | (_) | |___\n");
	print(" |_|   |_|\\___\\___/____/ \\___/ \\____|\n");

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
			long clock2= clock;
            printf(cos(clock2)*10);
			float a=0.5;
			a=a*7;
			a=a/3;
			printf(a);
        }
    }
}