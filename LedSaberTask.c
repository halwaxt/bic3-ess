
#include <stdbool.h>
#include <inc/hw_memmap.h>


/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Memory.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Semaphore.h>


/*Board Header files */
#include <Board.h>
#include <EK_TM4C1294XL.h>


#include <ctype.h>
#include <string.h>
#include <math.h>

#include <inc/hw_memmap.h>/*supplies GPIO_PORTx_BASE*/
#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>/*supplies GPIO_PIN_x*/
#include <driverlib/sysctl.h>

#include <LedSaberTask.h>
#include <mpu9150data.h>

#define LED1 GPIO_PIN_1
#define LED2 GPIO_PIN_0
#define LED3 GPIO_PIN_4
#define LED4 GPIO_PIN_0



void initializeLEDPort(uint32_t port, uint8_t pins) {
    uint32_t ui32Strength;
    uint32_t ui32PinType;

    /* read current config */
	GPIOPadConfigGet(port, pins, &ui32Strength, &ui32PinType);
	/* then set */
	GPIOPadConfigSet(port, pins, ui32Strength, GPIO_PIN_TYPE_STD);
	/* use as output */
	GPIOPinTypeGPIOOutput(port, pins);
}

void initializeLEDGroup(void) {

	/* activate gpio port N */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
	/* activate gpio port F */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    /* D1 and D2 */
    initializeLEDPort(GPIO_PORTN_BASE, LED1 | LED2);
    /* D3 and D4 */
    initializeLEDPort(GPIO_PORTF_BASE, LED3 | LED4);
}

/* only the 4 lower bits are used to set the status */
void setLEDGroupStatus(uint8_t status) {
	uint8_t bitPackedValue = 0x0;

	/* D1 and D2 */
	if (status & (0b00000001 << 0)) bitPackedValue |= LED1;
	if (status & (0b00000001 << 1)) bitPackedValue |= LED2;

	GPIOPinWrite (GPIO_PORTN_BASE, LED1 | LED2, bitPackedValue);

	/* D3 and D4 */
	bitPackedValue = 0x0;

	if (status & (0b00000001 << 2)) bitPackedValue |= LED3;
	if (status & (0b00000001 << 3)) bitPackedValue |= LED4;

	GPIOPinWrite (GPIO_PORTF_BASE, LED3 | LED4, bitPackedValue);
}

void DisplayLedSaberTask()
{
	const uint8_t ledPatterns[] = {0b00000000, 0b00001000, 0b00001100, 0b00001110, 0b00001111};
	Tacceleration acceleration;
	float gVector;
	float gValue;


	while (1) {
		if (Mailbox_pend(accelerationDataMailbox, &acceleration, BIOS_WAIT_FOREVER)) {

			gVector = fabsf(sqrtf(acceleration.x * acceleration.x + acceleration.y * acceleration.y + acceleration.z * acceleration.z));
			gValue = fabsf(gVector/9.81 - 1);

			if (gValue < 0.1) setLEDGroupStatus(ledPatterns[0]);
			else if (gValue < 0.2) setLEDGroupStatus(ledPatterns[1]);
			else if (gValue < 0.3) setLEDGroupStatus(ledPatterns[2]);
			else if (gValue < 0.4) setLEDGroupStatus(ledPatterns[3]);
			else setLEDGroupStatus(ledPatterns[4]);
		}
	}
}

/*
 *  setup task function
 */
void setupLedSaber(void)
{
	Task_Params taskLedParams;
	Task_Handle taskLed;
	Error_Block eb;
	
	initializeLEDGroup();

    Error_init(&eb);
    Task_Params_init(&taskLedParams);
    taskLedParams.stackSize = 1024;/*stack in bytes*/
    taskLedParams.priority = 7;
    taskLed = Task_create((Task_FuncPtr)DisplayLedSaberTask, &taskLedParams, &eb);
    if (taskLed == NULL) {
    	System_abort("Task DisplayLedSaber create failed");
    }
    else {
    	System_printf("Setup for DisplayLedSaber Task finished :-)\n");
    }
}
