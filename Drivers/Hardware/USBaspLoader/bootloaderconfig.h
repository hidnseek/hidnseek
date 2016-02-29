/* Name: bootloaderconfig.h
 * Project: USBaspLoader
 * Author: Christian Starkjohann
 * Creation Date: 2007-12-08
 * Tabsize: 4
 * Copyright: (c) 2007 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt)
 * This Revision: $Id$
 */

#ifndef __bootloaderconfig_h_included__
#define __bootloaderconfig_h_included__

/*
General Description:
This file (together with some settings in Makefile) configures the boot loader
according to the hardware.

This file contains (besides the hardware configuration normally found in
usbconfig.h) two functions or macros: bootLoaderInit() and
bootLoaderCondition(). Whether you implement them as macros or as static
inline functions is up to you, decide based on code size and convenience.

bootLoaderInit() is called as one of the first actions after reset. It should
be a minimum initialization of the hardware so that the boot loader condition
can be read. This will usually consist of activating a pull-up resistor for an
external jumper which selects boot loader mode.

bootLoaderCondition() is called immediately after initialization and in each
main loop iteration. If it returns TRUE, the boot loader will be active. If it
returns FALSE, the boot loader jumps to address 0 (the loaded application)
immediately.

For compatibility with Thomas Fischl's avrusbboot, we also support the macro
names BOOTLOADER_INIT and BOOTLOADER_CONDITION for this functionality. If
these macros are defined, the boot loader usees them.
*/

/* ---------------------------- Hardware Config ---------------------------- */

#define USB_CFG_IOPORTNAME      D
/* This is the port where the USB bus is connected. When you configure it to
 * "B", the registers PORTB, PINB and DDRB will be used.
 */
#define USB_CFG_DMINUS_BIT      4
/* This is the bit number in USB_CFG_IOPORT where the USB D- line is connected.
 * This may be any bit in the port.
 */
#define USB_CFG_DPLUS_BIT       2
/* This is the bit number in USB_CFG_IOPORT where the USB D+ line is connected.
 * This may be any bit in the port. Please note that D+ must also be connected
 * to interrupt pin INT0!
 */
#define USB_CFG_CLOCK_KHZ       (F_CPU/1000)
/* Clock rate of the AVR in MHz. Legal values are 12000, 16000 or 16500.
 * The 16.5 MHz version of the code requires no crystal, it tolerates +/- 1%
 * deviation from the nominal frequency. All other rates require a precision
 * of 2000 ppm and thus a crystal!
 * Default if not specified: 12 MHz
 */

/* ----------------------- Optional Hardware Config ------------------------ */

/* #define USB_CFG_PULLUP_IOPORTNAME   D */
/* If you connect the 1.5k pullup resistor from D- to a port pin instead of
 * V+, you can connect and disconnect the device from firmware by calling
 * the macros usbDeviceConnect() and usbDeviceDisconnect() (see usbdrv.h).
 * This constant defines the port on which the pullup resistor is connected.
 */
/* #define USB_CFG_PULLUP_BIT          4 */
/* This constant defines the bit number in USB_CFG_PULLUP_IOPORT (defined
 * above) where the 1.5k pullup resistor is connected. See description
 * above for details.
 */

/* ------------------------------------------------------------------------- */
/* ---------------------- feature / code size options ---------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------ Entering/Exiting Bootloader -------------------- */
/* There are two mutually exclusive ways to enter the boot loader:
 * 1) Manually by asserting bootLoaderCondition(). The boot loader waits
 *    until it is signaled to exit by either the programmer closing the
 *    connection or bootLooaderCondition() is no longer asserted. See
 *    USING_PUSHBUTTON and BOOTLOADER_CAN_EXIT to learn how to modify
 *    the exit behavior. This is the default entrance method.
 * 2) Automatically but the boot loader exits after a time out period has
 *    elapsed or the programmer closes the connection. See the comments
 *    for BOOTLOADER_CAN_EXIT to learn how to modify the exit behavior.
 */
#define TIMEOUT_ENABLED            1
/* If TIMEOUT_ENABLED is defined to 1 then the boot loader will always load
 * and stay active until the programmer closes the connection or the time
 * out period has elapsed. Since the boot loader always loads there is no
 * need for a jumper on the bootLoaderCondition() pin. Costs ~108 bytes.
 */
#define TIMEOUT_DURATION           60
/* The number of seconds the boot loader waits before exiting if no activity
 * has occured during the timeout interval. If TIMEOUT_ENABLED is defined
 * to 0 this define will be ignored. Maximum value is 255 seconds.
 */
#define USING_PUSHBUTTON           0
/* If USING_PUSHBUTTON is defined to 1, then a press and release of a
 * pushbutton on the bootLoaderCondition() pin can be used to signal that
 * you wish to enter the boot loader. If USING_PUSHBUTTON is defined to 0,
 * then you must use a jumper or the pushbutton must be pressed and held
 * for the duration of the programming (failing to do so will result in
 * the boot loader exiting prematurely).
 */
#define BOOTLOADER_CAN_EXIT         1
/* If this macro is defined to 1, the boot loader will exit shortly after the
 * programmer closes the connection to the device. Costs ~36 bytes.
 */

/* --------------------------- EEPROM Settings ----------------------------- */
#define HAVE_EEPROM_PAGED_ACCESS    1
/* If HAVE_EEPROM_PAGED_ACCESS is defined to 1, page mode access to EEPROM is
 * compiled in. Whether page mode or byte mode access is used by AVRDUDE
 * depends on the target device. Page mode is only used if the device supports
 * it, e.g. for the ATMega88, 168 etc. You can save quite a bit of memory by
 * disabling page mode EEPROM access. Costs ~ 138 bytes.
 */
#define HAVE_EEPROM_BYTE_ACCESS     1
/* If HAVE_EEPROM_BYTE_ACCESS is defined to 1, byte mode access to EEPROM is
 * compiled in. Byte mode is only used if the device (as identified by its
 * signature) does not support page mode for EEPROM. It is required for
 * accessing the EEPROM on the ATMega8. Costs ~54 bytes.
 */
#define HAVE_CHIP_ERASE             0
/* If this macro is defined to 1, the boot loader implements the Chip Erase
 * ISP command. Otherwise pages are erased on demand before they are written.
 */
//#define SIGNATURE_BYTES             0x1e, 0x93, 0x07, 0     /* ATMega8 */
/* This macro defines the signature bytes returned by the emulated USBasp to
 * the programmer software. They should match the actual device at least in
 * memory size and features. If you don't define this, values for ATMega8,
 * ATMega88, ATMega168 and ATMega328 are guessed correctly.
 */

/* The following block guesses feature options so that the resulting code
 * should fit into 2k bytes boot block with the given device and clock rate.
 * Activate by passing "-DUSE_AUTOCONFIG=1" to the compiler.
 * This requires gcc 3.4.6 for small enough code size!
 */
#if USE_AUTOCONFIG
#   undef HAVE_EEPROM_PAGED_ACCESS
#   define HAVE_EEPROM_PAGED_ACCESS     (USB_CFG_CLOCK_KHZ != 15000)
#   undef HAVE_EEPROM_BYTE_ACCESS
#   define HAVE_EEPROM_BYTE_ACCESS      1
#   undef BOOTLOADER_CAN_EXIT
#   define BOOTLOADER_CAN_EXIT          1
#   undef SIGNATURE_BYTES
#endif /* USE_AUTOCONFIG */

/* ------------------------------------------------------------------------- */

/* Example configuration: Port D bit 3 is connected to a jumper which ties
 * this pin to GND if the boot loader is requested. Initialization allows
 * several clock cycles for the input voltage to stabilize before
 * bootLoaderCondition() samples the value.
 * We use a function for bootLoaderInit() for convenience and a macro for
 * bootLoaderCondition() for efficiency.
 */

#ifndef __ASSEMBLER__   /* assembler cannot parse function definitions */

#define JUMPER_BIT  3   /* jumper is connected to this bit in port D, active low */
#define CHG500_BIT  2   /* jumper is connected to this bit in port D, active low */
#define SHDOWN_BIT  1   /* jumper is connected to this bit in port D, active low */

#ifndef MCUCSR          /* compatibility between ATMega8 and ATMega88 */
#   define MCUCSR   MCUSR
#endif

#ifndef TIMEOUT_ENABLED
#   define TIMEOUT_ENABLED 0
#endif

static inline void  bootLoaderInit(void)
{
    DDRB  |= (1 << SHDOWN_BIT);     /* enable supply */
    PORTB &= ~(1 << SHDOWN_BIT);    /* supply off */
    PORTC &= ~(1 << CHG500_BIT);    /* disable fast charge */
    DDRC  |= (1 << CHG500_BIT);     /* output fast charge */
    PORTD |= (1 << JUMPER_BIT);     /* activate pull-up */
/*
    if(!(MCUCSR & (1 << EXTRF)))    If this was not an external reset, ignore
        leaveBootloader();
*/
    MCUCSR = 0;                     /* clear all reset flags for next time */
}

static inline void  bootLoaderExit(void)
{
    // PORTD = 0;                      /* undo bootLoaderInit() changes */
    // DDRC = 0; // turn off LEDs
#if TIMEOUT_ENABLED
    TCCR1B = 0; // turn off timer1 and reset to initial value
#endif

}

#if !TIMEOUT_ENABLED
#define bootLoaderCondition()   ((PIND & (1 << JUMPER_BIT)) == 0)
#else
#define bootLoaderCondition()    1
#endif

#endif /* __ASSEMBLER__ */

/* ------------------------------------------------------------------------- */

#endif /* __bootloader_h_included__ */
