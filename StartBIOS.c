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
#include <ti/sysbios/hal/Hwi.h>

#include <ti/drivers/I2C.h>
#include <ti/drivers/PWM.h>


/* TI-RTOS Header files */
#include <ti/drivers/UART.h>


/*Board Header files */
#include <Board.h>
#include <EK_TM4C1294XL.h>

/*application header files*/
#include <ctype.h>
#include <string.h>
#include <mpu.h>
#include <mpu9150data.h>
#include <LedSaberTask.h>
#include <UARTTask.h>
#include <buzzer.h>

#define READ_SENSOR_INTERVAL 50000

volatile I2C_Handle i2c;
volatile Event_Handle readSensorElapsedEventHandle;

volatile Mailbox_Handle rawDataMailbox;
volatile Mailbox_Handle accelerationDataMailbox;
volatile UART_Handle uartHandle;

/* is there a "better" way to share the Event_Handle? */
void onReadSensorClockElapsed(void) {
	Event_post(readSensorElapsedEventHandle, Event_Id_00);
}

/*
 *  setup clock task function
 */
void setupReadSensorClockTask()
{
	UInt key;
	volatile Timer_Handle sensorTimerHandle;
 	Timer_Params sensorTimerParameter;

	Error_Block eb;
	Error_init(&eb);

 	Timer_Params_init(&sensorTimerParameter);
 	sensorTimerParameter.period = READ_SENSOR_INTERVAL;
	sensorTimerParameter.periodType=Timer_PeriodType_MICROSECS;
	sensorTimerParameter.runMode=Timer_RunMode_CONTINUOUS;
	sensorTimerParameter.startMode=Timer_StartMode_AUTO;
	sensorTimerHandle = Timer_create(4, (ti_sysbios_interfaces_ITimer_FuncPtr)onReadSensorClockElapsed, &sensorTimerParameter, &eb);
	key = Hwi_disable();
	Timer_start(sensorTimerHandle);
	Hwi_restore(key);
}

int main(void) {

	Error_Block errorBlock;
	Error_init(&errorBlock);

    uint32_t ui32SysClock;

	/* Call board init functions. */
	ui32SysClock = Board_initGeneral(120*1000*1000);

	readSensorElapsedEventHandle = Event_create(NULL, &errorBlock);
	if (readSensorElapsedEventHandle == NULL) {
		System_abort("creating read sensor clock handle failed!\n");
	}

	Mailbox_Params mailboxParams;
	Mailbox_Params_init(&mailboxParams);

	rawDataMailbox = Mailbox_create(sizeof(Tmpu9150data), 1, &mailboxParams, &errorBlock);
	if (rawDataMailbox == NULL) {
		System_abort("creating mailbox for raw failed!\n");
	}

	accelerationDataMailbox = Mailbox_create(sizeof(Tacceleration), 1, &mailboxParams, &errorBlock);
	if (accelerationDataMailbox == NULL) {
		System_abort("creating mailbox for acceleration failed!\n");
	}


	initializeBus();
	initializeUart();

	//initBuzzer();
	//startSoundTimer();
	//makeSoundTask();

	setupSensor();
	setupPeriodicRead();
	setupReadSensorClockTask();
	setupLedSaber();

	setupUartTask();

    System_printf("Start BIOS\n");
    System_flush();

    /* Start BIOS */
    BIOS_start();
}
