#ifndef _PRINTF_H_
#define _PRINTF_H_

/*
 * UART Module Register Structure
 */
typedef volatile struct {
	SnQByte qDBGU_CR;
	SnQByte qDBGU_MR;
	SnQByte qDBGU_IER;
	SnQByte qDBGU_IDR;
	SnQByte qDBGU_IMR;
	SnQByte qDBGU_CSR;
	SnQByte qDBGU_RHR;
	SnQByte qDBGU_THR;
	SnQByte qDBGU_BRGR;
} AtmelDbgUart;

void putchar(const char c);
int printf(const char *format, ...);
int sprintf(char *out, const char *format, ...);

#endif
