
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


#define GFACTOR 10000
#define PERIOD_LENGTH 100
#define GMULTIPLY 3

//volatile Mailbox_Handle mailboxHandle;

volatile Timer_Handle startSoundTimerHandle;
volatile Timer_Handle stopSoundTimerHandle;

void initBuzzer() {
	//GPIO_init();
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
	SysCtlPeripheralEnable(GPIO_PORTM_BASE);
	GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_3);
	GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_3,0);
}


void soundTimer(void) {
	UInt key;

	GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_3, ~(GPIOPinRead(GPIO_PORTM_BASE, GPIO_PIN_3)));
	Timer_setPeriod(stopSoundTimerHandle,(Timer_getPeriod(startSoundTimerHandle)));
	key = Hwi_disable();
	Timer_start(stopSoundTimerHandle);
	Hwi_restore(key);

}
void stopTimer(void) {
	GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_3, ~(GPIOPinRead(GPIO_PORTM_BASE, GPIO_PIN_3)));
}

void startSoundTimer() {
	UInt key;
 	Timer_Params startTimerParameter;
 	Timer_Params stopTimerParameter;

	Error_Block eb;
	Error_init(&eb);

 	Timer_Params_init(&startTimerParameter);
 	startTimerParameter.period = 500;
	startTimerParameter.periodType=Timer_PeriodType_MICROSECS;
	startTimerParameter.runMode=Timer_RunMode_CONTINUOUS;
	startTimerParameter.startMode=Timer_StartMode_AUTO;
	startSoundTimerHandle = Timer_create(2, (ti_sysbios_interfaces_ITimer_FuncPtr) soundTimer, &startTimerParameter, &eb);
	key = Hwi_disable();
	Timer_start(startSoundTimerHandle);
	Hwi_restore(key);

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
	//Task_sleep(length*2);
}

void makeSound() {
	float gVector;
	float gValue;
	Tacceleration acceleration;

	while (1) {
		if (Mailbox_pend(accelerationDataMailbox, &acceleration, BIOS_WAIT_FOREVER)) {

			gVector = fabsf(sqrtf(acceleration.x * acceleration.x + acceleration.y * acceleration.y + acceleration.z * acceleration.z));
			gValue = fabsf(gVector/9.81 - 1) * GMULTIPLY;
			if (gValue < 0.001) {
				gValue = 0.001;
			}

			System_printf("gValue: %f\n", gValue);
			System_printf("gValue*GFACTOR: %f\n", gValue*GFACTOR);
			System_flush();

			pwmEmulate((gValue*GFACTOR),PERIOD_LENGTH );
			//pwmEmulate(0,GDELAY);
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
	musicTaskParameter.priority = 7;
	musicTaskParameter.arg0 = NULL;
	musicTaskParameter.arg1 = NULL;

	musicTask = Task_create((Task_FuncPtr)makeSound, &musicTaskParameter, &eb);
	if (musicTask == NULL) {
 		System_abort("musicTask failed\n");
	}
}


