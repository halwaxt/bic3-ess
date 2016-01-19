/*
 * UARTTask.c
 *
 *  Created on: 13.01.2016
 *      Author: Thomas Halwax
 */

#include <stdbool.h>
#include <inc/hw_memmap.h>

#include <xdc/std.h>
#include <stdio.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Event.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
#include <Board.h>
#include <EK_TM4C1294XL.h>
#include <driverlib/uart.h>
#include <ti/drivers/UART.h>
#include <UARTTask.h>
#include <mpu9150data.h>

void initializeUart(void) {
	EK_TM4C1294XL_initUART();

	UART_Params uartParams;

	UART_Params_init(&uartParams);
	uartParams.writeDataMode = UART_DATA_TEXT;
	uartParams.readDataMode = UART_DATA_TEXT;
	uartParams.readReturnMode = UART_RETURN_FULL;
	uartParams.readEcho = UART_ECHO_OFF;
	uartParams.baudRate = 9600;

	// handle is extern because an other task will use it to write some data
	uartHandle = UART_open(Board_UART0, &uartParams);
	if (uartHandle == NULL) {
		System_abort("initialize UART failed!\n");
	}
	System_printf("initialized Uart done!\n");

}

void writeToUartTask(void) {
	if (uartHandle == NULL) {
		System_abort("uartHandle is NULL!");
	}

	Tmpu9150data sensorData;
	Tgyro gyroData;
	char uartBuffer[64];
	int bufferSize;

	while (1) {
		if (Mailbox_pend(rawDataMailbox, &sensorData, BIOS_WAIT_FOREVER)) {

			float temp = getTemperatureInCelsius(&sensorData);
			bufferSize = sprintf(uartBuffer, "current temp in °C: %f\n", temp);
			UART_write(uartHandle, &uartBuffer, bufferSize);

			getGyroValues(&sensorData, &gyroData);
			bufferSize = sprintf(uartBuffer, "gyroscope gx:%f gy:%f gz:%f\n", gyroData.x, gyroData.y, gyroData.z);
			UART_write(uartHandle, &uartBuffer, bufferSize);
		}
	}
}

void setupUartTask(void) {
	Task_Params initializeUartTaskParams;
	Task_Handle initializeUartTaskHandle;
	Error_Block eb;

	Error_init(&eb);
	Task_Params_init(&initializeUartTaskParams);
	initializeUartTaskParams.stackSize = 1024;/*stack in bytes*/
	initializeUartTaskParams.priority = 7;
	initializeUartTaskHandle = Task_create((Task_FuncPtr)writeToUartTask, &initializeUartTaskParams, &eb);
	if (initializeUartTaskHandle == NULL) {
		System_abort("Creating Uart Task failed");
	}
	else {
		System_printf("created task for uart output\n");
	}
}

