



#ifndef SHIFTLED_TASK_H_
#define SHIFTLED_TASK_H_

#include <stdbool.h>
#include <stdint.h>
/* Drivers Header files - fall back to driverlib for gpio*/
#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <inc/hw_memmap.h>
#include <ti/sysbios/knl/Mailbox.h>


extern volatile Mailbox_Handle accelerationDataMailbox;

void setupLedSaber(void);

#endif /* UIP_TASK_H_ */
