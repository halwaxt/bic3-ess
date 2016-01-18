/*
 * UARTTask.h
 *
 *  Created on: 13.01.2016
 *      Author: Thomas Halwax
 */

#ifndef LOCAL_INC_UARTTASK_H_
#define LOCAL_INC_UARTTASK_H_

#include <ti/drivers/UART.h>
#include <ti/sysbios/knl/Mailbox.h>

extern volatile UART_Handle uartHandle;
extern volatile Mailbox_Handle rawDataMailbox;

void setupUartTask(void);
void initializeUart(void);

#endif /* LOCAL_INC_UARTTASK_H_ */
