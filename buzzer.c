i
#include <stdbool.h>
#include <inc/hw_memmap.h>


#include <time.h>
#include <stdlib.h>

#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Queue.h>

#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <driverlib/sysctl.h>
#include <EK_TM4C1294XL.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/PWM.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/GPIO.h>
#include <driverlib/ssi.h>
#include <inc/hw_ssi.h>
#include <inc/hw_memmap.h>
#include <ti/sysbios/hal/Timer.h>
#include <ti/sysbios/hal/Hwi.h>


/*Board Header files */
#include <Board.h>
#include <EK_TM4C1294XL.h>
#include "tones.h"


/* Infos:
 * PIN: PWM -> PM3 -> P0
 *
 * Seiten 418-440
 *
 * Das Timing muss interrupt-getrieben realisiert sein!
 *
 * als eigener Task:
 * - getValueFromDOOF();
 *
 * dieser ruft die folgenden Funktionen auf:
 * - convertSItoHZ(si);
 *
 * diese ruft das auf:
 * - makeSound(HZ)
 */

uint8_t soundon=1;

Timer_Handle startSoundTimerHandle;
Timer_Handle stopSoundTimerHandle;

void startTimer(void) {
   GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_3, ~(GPIOPinRead(GPIO_PORTM_BASE, GPIO_PIN_3))); // der toggelt den pin ein und aus
   Timer_setPeriod(stopSoundTimerHandle,(Timer_getPeriod(startSoundTimerHandle)/2)); // hier wird der zweite timer gestartet der das dann wieder abdreht. lautstärke wird reguliert durch die dauer. frequenz /2 ist die volle lautstärke.
   Timer_start(stopSoundTimerHandle);

}
void stopTimer(void) {
   GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_3, ~(GPIOPinRead(GPIO_PORTM_BASE, GPIO_PIN_3)));
}

void initBuzzer() {
	//GPIO_init();
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
	SysCtlPeripheralEnable(GPIO_PORTM_BASE); /*PWM Pin fuer den unteren booster platz*/
	GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_3);
	GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_3,0);
}

void initSoundTimer() {
    Timer_Params startTimerParameter;
    Timer_Params stopTimerParameter;

	Error_Block eb;
	Error_init(&eb);

    Timer_Params_init(&startTimerParameter);
    startTimerParameter.period = 0;
    //startTimerParameter.period = 700;
    startTimerParameter.periodType=Timer_PeriodType_MICROSECS;
	startTimerParameter.runMode=Timer_RunMode_CONTINUOUS;
    startTimerParameter.startMode=Timer_StartMode_AUTO;
    startSoundTimerHandle = Timer_create(2, (ti_sysbios_interfaces_ITimer_FuncPtr) startTimer, &startTimerParameter, &eb);
    Timer_start(startSoundTimerHandle);

    Timer_Params_init(&stopTimerParameter);
    stopTimerParameter.period = 0;
    //stopTimerParameter.period = 350;
    stopTimerParameter.periodType=Timer_PeriodType_MICROSECS;
	stopTimerParameter.runMode=Timer_RunMode_ONESHOT;
    stopTimerParameter.startMode=Timer_StartMode_USER;
    stopSoundTimerHandle = Timer_create(3, (ti_sysbios_interfaces_ITimer_FuncPtr) stopTimer, &stopTimerParameter, &eb);
}

void pwmEmulate(uint32_t microsec_period, uint16_t length)
{
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

void makeSound(float gValue) {

	pwmEmulate((gValue*100),50);
	pwmEmulate(0,1);
}

void testSound() {
	uint8_t i;
	float int2float;
	for (i=0; i<=10; i++) {
		int2float = (float)i/10;
		makeSound(int2float);
	}
}

int main() {

	Task_Params musicTaskParameter;

	initBuzzer();
	initSoundTimer();

    /* create a music task */
    Task_Params_init(&musicTaskParameter);
    musicTaskParameter.priority = 15;
    musicTaskParameter.arg0 = NULL;
    musicTaskParameter.arg1 = NULL;
    Task_create(&testSound, &musicTaskParameter, NULL);

    BIOS_start();
	return(0);
}

