/*
 * Size specific types. They are always unsigned and fixed in size.
 */
#ifndef SNTYPES_H
#define SNTYPES_H

typedef unsigned char SnByte;           /* Unsigned single byte (8 bit) quantity */
typedef unsigned short SnWord;            /* Unsigned two byte (16 bit) quantity */
typedef unsigned long SnQByte;          /* Unsigned four byte (32 bit) quantity */

typedef short SnSWord;                    /* Signed two byte (16 bit) quantity */
typedef long SnSQByte;                  /* Signed four byte (32 bit) quantity */

typedef unsigned long SnAddr;           /* Unsigned quantity that can hold an address */

typedef unsigned long SnBool;

#endif
