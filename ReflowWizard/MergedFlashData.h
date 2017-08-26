#ifndef __MergedFlashData__
#define __MergedFlashData__

/* Macros which behave like PSTR and F() BUT allow merging of the constant data, to save flash space */
/* See: http://michael-buschbeck.github.io/arduino/2013/10/22/string-merging-pstr-percent-codes/ */

#define PSTRM(str) \
  (__extension__({ \
    PGM_P ptr;  \
    asm volatile \
    ( \
      ".pushsection .progmem.data2, \"SM\", @progbits, 1" "\n\t" \
      "0: .string " #str                                 "\n\t" \
      ".popsection"                                      "\n\t" \
    ); \
    asm volatile \
    ( \
      "ldi %A0, lo8(0b)"                                 "\n\t" \
      "ldi %B0, hi8(0b)"                                 "\n\t" \
      : "=d" (ptr) \
    ); \
    ptr; \
  }))
  
#define FM(string_literal) (reinterpret_cast<const __FlashStringHelper *>(PSTRM(string_literal)))  

#define FLASH_STRING(name) const PROGMEM char name[]

#endif
