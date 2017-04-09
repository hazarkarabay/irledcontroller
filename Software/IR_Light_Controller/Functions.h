/* Getting MCU reset cause 
Currently needs modified Optiboot
https://github.com/Optiboot/optiboot/issues/140#issuecomment-249275329
*/
// Startup value of MCUSR register. Put in .noinit, since it is filled
// by get_mcusr before gcc initializes global variables.
static uint8_t mcusr __attribute__((section(".noinit")));

// Optiboot clears MCUSR and puts the original value in r2. This bit of
// code runs as soon as possible during startup to preserve the value.
// However, if MCUSR is not cleared, just use that instead (future
// versions of optiboot might no longer clear MCUSR, or we might be
// running without a bootloader). Also, make sure to clear MCUSR and
// disable the watchdog, just in case.
// This uses inline assembly, since the compiler might generate code
// that uses the stack, or assumes r0 is set to 0, etc.
void get_mcusr(void) __attribute__((naked)) __attribute__((used)) __attribute__((section(".init0")));
void get_mcusr(void)
{
  asm(
    // Store MCUSR, or r2 if MCUSR is 0
    "in  %[mcusr_var], %[mcusr_reg]\n"
    "subi %[mcusr_var], 0\n"
    "brne mcusr_not_empty\n"
    "mov %[mcusr_var], r2\n"
    "mcusr_not_empty:\n"
    // Clear MCUSR
    "clr r1\n"
    "out %[mcusr_reg], r1\n"
    // Disable watchdog
    "ldi r16, %[change_enable]\n"
    "sts %[wdtcsr_reg], r16\n"
    "sts %[wdtcsr_reg], r1\n"
    : [mcusr_var] "=d" (mcusr)
    : [mcusr_reg] "I" (_SFR_IO_ADDR(MCUSR)),
      [wdtcsr_reg] "M" (_SFR_MEM_ADDR(WDTCSR)),
      [change_enable] "M" ((1 << WDCE) | (1 << WDE))
    : "r1", "r16"
  );
}

/*  PROGMEM print() helper
    display.print((const __FlashStringHelper*)str_lowVoltage);
    ==
    display.print(PGMT(str_lowVoltage));
*/
#define PGMT( pgm_ptr ) ( reinterpret_cast< const __FlashStringHelper * >( pgm_ptr ) )
#define sizepgm( pgm_ptr ) ( sizeof(pgm_ptr) / sizeof(*pgm_ptr) )

// Serial.print functions for debugging.
#ifndef DEBUGUTILS_H
#define DEBUGUTILS_H

#ifdef DEBUG
//#include <Arduino.h>
#include <string.h>
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define DEBUG_PRINT(str)    \
   Serial.print(millis());     \
   Serial.print(": ");    \
   Serial.print(__PRETTY_FUNCTION__); \
   Serial.print(' ');      \
   Serial.print(__FILENAME__);     \
   Serial.print(':');      \
   Serial.print(__LINE__);     \
   Serial.print(' ');      \
   Serial.println(str);

#define DEBUG_PRINT2(str1, str2)    \
	Serial.print(millis());     \
	Serial.print(": ");    \
	Serial.print(__PRETTY_FUNCTION__); \
	Serial.print(' ');      \
	Serial.print(__FILENAME__);     \
	Serial.print(':');      \
	Serial.print(__LINE__);     \
	Serial.print(' ');      \
	Serial.print(str1); \
	Serial.println(str2);
	
#define DEBUG_PRINT3(str1, str2, str3)    \
	Serial.print(millis());     \
	Serial.print(": ");    \
	Serial.print(__PRETTY_FUNCTION__); \
	Serial.print(' ');      \
	Serial.print(__FILENAME__);     \
	Serial.print(':');      \
	Serial.print(__LINE__);     \
	Serial.print(' ');      \
	Serial.print(str1); \
	Serial.print(str2); \
	Serial.println(str3);
	
#define DEBUG_PRINT4(str1, str2, str3, str4)    \
	Serial.print(millis());     \
	Serial.print(": ");    \
	Serial.print(__PRETTY_FUNCTION__); \
	Serial.print(' ');      \
	Serial.print(__FILENAME__);     \
	Serial.print(':');      \
	Serial.print(__LINE__);     \
	Serial.print(' ');      \
	Serial.print(str1); \
	Serial.print(str2); \
	Serial.print(str3); \
	Serial.println(str4);
#else
#define DEBUG_PRINT(str)
#define DEBUG_PRINT2(str1, str2)
#define DEBUG_PRINT3(str1, str2, str3)
#define DEBUG_PRINT4(str1, str2, str3, str4)
#endif

#endif
