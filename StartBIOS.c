/*
 * CCSv6 project using TI-RTOS
 *
 */



/*
 *  ======== StartBIOS.c ========
 */
/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Memory.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/hal/Timer.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/drivers/I2C.h>

/* TI-RTOS Header files */
#include <ti/drivers/UART.h>


/*Board Header files */
#include <Board.h>
#include <EK_TM4C1294XL.h>

/*application header files*/
#include <ctype.h>
#include <string.h>
#include <mpu.h>

volatile I2C_Handle i2c;
volatile Event_Handle clockElapsedEventHandle;

/* is there a "better" way to share the Event_Handle? */
void onClockElapsed(void) {
	Event_post(clockElapsedEventHandle, Event_Id_00);
}

/*
 *  setup clock task function
 */
int setupClockTask()
{
	Clock_Params clockParameters;
    Clock_Params_init(&clockParameters);
    clockParameters.period = 250;
    clockParameters.startFlag = TRUE;
    Clock_create((Clock_FuncPtr)onClockElapsed, 250, &clockParameters, NULL);
    return (0);
}

int main(void) {

    uint32_t ui32SysClock;

	/* Call board init functions. */
	ui32SysClock = Board_initGeneral(120*1000*1000);

	clockElapsedEventHandle = Event_create(NULL, NULL);
	if (clockElapsedEventHandle == NULL) {
		System_abort("creating event handle failed!\n");
	}

	initializeBus();
	setupSensor();
	setupPeriodicRead();
	setupClockTask();

    System_printf("Start BIOS\n");
    System_flush();

    /* Start BIOS */
    BIOS_start();
}
