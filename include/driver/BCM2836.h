#ifndef _DRIVER_BCM2836_H_

#define MMU_BASE_OFFSET 0x3F000000

/* Modusbits aufgeschlüsselt */
enum psr_mode {
	PSR_USR = 0x10,
	PSR_FIQ = 0x11,
	PSR_IRQ = 0x12,
	PSR_SVC = 0x13,
	PSR_ABT = 0x17,
	PSR_UND = 0x1b,
	PSR_SYS = 0x1f,
};

#define _DRIVER_BCM2836_H_
#endif
