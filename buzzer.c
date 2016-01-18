
#include <stdbool.h>
#include <inc/hw_memmap.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>

#include <ti/drivers/GPIO.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/hal/Timer.h>
#include <ti/sysbios/knl/Mailbox.h>

#include <ti/sysbios/hal/Hwi.h>
#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <driverlib/sysctl.h>



/*Board Header files */
#include <Board.h>
#include <EK_TM4C1294XL.h>
#include "mpu9150data.h"
#include "buzzer.h"

/*
 * Development Header
 */
#include <ti/sysbios/knl/Event.h>



#define GFACTOR 100
#define GDELAY 10

volatile Mailbox_Handle mailboxHandle;
volatile Event_Handle clockElapsedEventHandle;

Timer_Handle startSoundTimerHandle;
Timer_Handle stopSoundTimerHandle;


int main() {

	initBuzzer();

	createClockEvent(); // Dev Function
	createMailbox(); // Dev Function
	startClockTask(); // Dev Function

	startSoundTimer();
	startPeriodicWriteTask(); // Dev Function

	makeSoundTask();

	BIOS_start();
	return(0);
}


void initBuzzer() {
	//GPIO_init();
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
	SysCtlPeripheralEnable(GPIO_PORTM_BASE);
	GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_3);
	GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_3,0);
}


void startTimer(void) {
	GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_3, ~(GPIOPinRead(GPIO_PORTM_BASE, GPIO_PIN_3)));
	Timer_setPeriod(stopSoundTimerHandle,(Timer_getPeriod(startSoundTimerHandle)));
	Timer_start(stopSoundTimerHandle);

}
void stopTimer(void) {
	GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_3, ~(GPIOPinRead(GPIO_PORTM_BASE, GPIO_PIN_3)));
}

void startSoundTimer() {
 	Timer_Params startTimerParameter;
 	Timer_Params stopTimerParameter;

	Error_Block eb;
	Error_init(&eb);

 	Timer_Params_init(&startTimerParameter);
 	startTimerParameter.period = 0;
	startTimerParameter.periodType=Timer_PeriodType_MICROSECS;
	startTimerParameter.runMode=Timer_RunMode_CONTINUOUS;
	startTimerParameter.startMode=Timer_StartMode_AUTO;
	startSoundTimerHandle = Timer_create(2, (ti_sysbios_interfaces_ITimer_FuncPtr) startTimer, &startTimerParameter, &eb);
	Timer_start(startSoundTimerHandle);

	Timer_Params_init(&stopTimerParameter);
	stopTimerParameter.period = 0;
	stopTimerParameter.periodType=Timer_PeriodType_MICROSECS;
	stopTimerParameter.runMode=Timer_RunMode_ONESHOT;
	stopTimerParameter.startMode=Timer_StartMode_USER;
	stopSoundTimerHandle = Timer_create(3, (ti_sysbios_interfaces_ITimer_FuncPtr) stopTimer, &stopTimerParameter, &eb);
}

void pwmEmulate(uint32_t microsec_period, uint16_t length) {
	UInt key;
	// disable interrupts if an interrupt could lead to
	// another call to Timer_start().
	key = Hwi_disable();
	Timer_setPeriodMicroSecs(startSoundTimerHandle,microsec_period);
	if (microsec_period != 0) {
		Timer_start(startSoundTimerHandle);
	}
	Hwi_restore(key);
	Task_sleep(length*2);
}

void makeSound() {
	float gVector;
	float gValue;
	Tacceleration acceleration;

	while (1) {
		if (Mailbox_pend(mailboxHandle, &acceleration, BIOS_WAIT_FOREVER)) {

			gVector = fabsf(sqrtf(acceleration.x * acceleration.x + acceleration.y * acceleration.y + acceleration.z * acceleration.z));
			gValue = fabsf(gVector/9.81 - 1);

			System_printf("gValue: %f\n", gValue);
			System_flush();

			if (gValue == 0) {
				gValue = 0.1;
			}
			pwmEmulate((gValue*GFACTOR),GDELAY);
			pwmEmulate(0,1);
		}
	}
}

void makeSoundTask() {
	/* create a music task */
	Error_Block eb;
	Error_init(&eb);
	Task_Handle musicTask;
	Task_Params musicTaskParameter;

	Task_Params_init(&musicTaskParameter);
	musicTaskParameter.priority = 15;
	musicTaskParameter.arg0 = NULL;
	musicTaskParameter.arg1 = NULL;

	musicTask = Task_create((Task_FuncPtr)makeSound, &musicTaskParameter, &eb);
	if (musicTask == NULL) {
 		System_abort("musicTask failed\n");
	}
}


/*
 * ===============================================================================
 * Functions for development only
 */

void onClockElapsed(void) {
	Event_post(clockElapsedEventHandle, Event_Id_00);
}

void startClockTask() {
	Clock_Params clockParameters;
    Clock_Params_init(&clockParameters);
    clockParameters.period = 50;
    clockParameters.startFlag = TRUE;
    Clock_create((Clock_FuncPtr)onClockElapsed, 50, &clockParameters, NULL);
}

void createClockEvent() {
	Error_Block errorBlock;
	Error_init(&errorBlock);

	clockElapsedEventHandle = Event_create(NULL, &errorBlock);
	if (clockElapsedEventHandle == NULL) {
		System_abort("creating event handle failed!\n");
	}
}

void createMailbox() {
	Error_Block errorBlock;
	Error_init(&errorBlock);

	Mailbox_Params mailboxParams;
	Mailbox_Params_init(&mailboxParams);

	mailboxHandle = Mailbox_create(sizeof(Tacceleration), 1, &mailboxParams, &errorBlock);
	if (mailboxHandle == NULL) {
		System_abort("creating mailbox failed!\n");
	}
}

void write2Mailbox() {
	UInt eventPendingResult;
	Tacceleration acceleration;

	while (1) {
		eventPendingResult = Event_pend(clockElapsedEventHandle, Event_Id_00, Event_Id_00, BIOS_WAIT_FOREVER);
		if (eventPendingResult != 0) {

			acceleration.x = ((float)rand()/(float)(RAND_MAX));
			acceleration.y = ((float)rand()/(float)(RAND_MAX));
			acceleration.z = ((float)rand()/(float)(RAND_MAX));

			if (! Mailbox_post(mailboxHandle, &acceleration, BIOS_NO_WAIT)) {
				System_printf("target mailbox is full\n");
				System_flush();
			}
		}
	}
}

void startPeriodicWriteTask() {
	Task_Params periodicReadTaskParams;
	Task_Handle periodicReadTaskHandle;
	Error_Block eb;

    Error_init(&eb);
    Task_Params_init(&periodicReadTaskParams);
    periodicReadTaskParams.stackSize = 1024;/*stack in bytes*/
    periodicReadTaskParams.priority = 5;
    periodicReadTaskHandle = Task_create((Task_FuncPtr)write2Mailbox, &periodicReadTaskParams, &eb);
    if (periodicReadTaskHandle == NULL) {
    	System_abort("Creating periodicReadTask failed");
    }
    else {
    	System_printf("created task periodicReadTask\n");
    }
}

