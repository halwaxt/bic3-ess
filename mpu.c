/*
 * mpu.c
 *
 *  Created on: 02.12.2015
 *      Author: Thomas Halwax
 */

#include <math.h>
#include <xdc/std.h>
#include <ctype.h>
#include <ti/drivers/I2C.h>
#include <mpu.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <driverlib/pin_map.h>
#include <driverlib/sysctl.h>
#include <inc/hw_memmap.h>
#include <EK_TM4C1294XL.h>
#include <ti/sysbios/knl/Task.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>



void initializeBus(void) {
	/* This function also calls the I2C_init() to initialize the I2C driver. */
	EK_TM4C1294XL_initI2C();


	I2C_Params i2cParams;
	I2C_Params_init(&i2cParams);
	i2cParams.transferMode = I2C_MODE_BLOCKING;
	i2cParams.transferCallbackFxn = NULL;
	i2cParams.bitRate = I2C_400kHz;

	UInt peripheralNum = EK_TM4C1294XL_I2C8;

	i2c = I2C_open(peripheralNum, &i2cParams);

}

void initializeMPU9150Task(void) {
	if (i2c == NULL) System_abort("initializeMPU9150Task: i2c is not initialized");

	uint8_t readBuffer[5];
	uint8_t writeBuffer[5];

	I2C_Transaction i2cTransaction;
	bool transferOK = true;

	i2cTransaction.slaveAddress = SENSOR_ADDRESS;
	i2cTransaction.writeBuf = &writeBuffer;
	i2cTransaction.readBuf = &readBuffer;

	/* device is in sleep mode after power on */
	/* Put the MPU9150 into reset state */

	writeBuffer[0] = MPU9150_PWR_MGMT_1;
	writeBuffer[1] = MPU9150_PWR_MGMT_1_DEVICE_RESET;

	i2cTransaction.writeCount = 2;
	i2cTransaction.readCount = 0;
	if (!I2C_transfer(i2c, &i2cTransaction)) {
		System_abort("putting sensor into reset mode failed");
	}

	/* loop to verify device has finished resetting and is back into sleep mode */
	writeBuffer[0] = MPU9150_PWR_MGMT_1;
	i2cTransaction.writeCount = 1;
	i2cTransaction.readCount = 1;
	do {
		transferOK = I2C_transfer(i2c, &i2cTransaction);
	} while ((readBuffer[0] != MPU9150_PWR_MGMT_1_SLEEP) || (!transferOK));
	if (!transferOK) {
		System_abort("enabling clock synchronized with gyro X failed");
	}

	/* remove reset and enable the clock locked on gyro X */
   	writeBuffer[0] = MPU9150_PWR_MGMT_1;
   	writeBuffer[1] = MPU9150_PWR_MGMT_1_CLKSEL_XG;
   	i2cTransaction.writeCount = 2;
	i2cTransaction.readCount = 0;
   	if (!I2C_transfer(i2c, &i2cTransaction)) {
   		System_abort("enabling clock synchronized with gyro X failed");
	}

   	writeBuffer[0] = MPU9150_REG_USER_CTRL;
   	writeBuffer[1] = MPU9150_USER_CTRL_MASTER_DISABLE;
   	i2cTransaction.writeCount = 2;
   	i2cTransaction.readCount = 0;
   	if (!I2C_transfer(i2c, &i2cTransaction)) {
   		System_abort("disabling i2c master mode failed");
   	}

   	/* enable low pass filter */
   	writeBuffer[0] = MPU9150_REG_CONFIG;
	writeBuffer[1] = MPU9150_CONFIG_DLPF_21HZ;
	i2cTransaction.writeCount = 2;
	i2cTransaction.readCount = 0;
	if (!I2C_transfer(i2c, &i2cTransaction)) {
		System_abort("setting DLPF failed");
	}

   	/*
   	 * Sample Rate = Gyroscope Output Rate / (1 + SMPLRT_DIV)
   	 * Note: The accelerometer output rate is 1kHz. This means that for a Sample Rate
   	 * greater than 1kHz, the same accelerometer sample may be output to the FIFO, DMP,
   	 * and sensor registers more than once. */

   	/* for 50 HZ: 1000/20 -> 1000 / 1 + 19 -> SMPLRT_DIV = 19 */
   	writeBuffer[0] = MPU9150_SMPLRT_DIV;
   	writeBuffer[1] = 19;
   	i2cTransaction.writeCount = 2;
   	i2cTransaction.readCount = 0;
   	if (!I2C_transfer(i2c, &i2cTransaction)) {
   		System_abort("enabling sample rate failed");
   	}

   	/* set accel sensitivity to 2g */
   	writeBuffer[0] = MPU9150_REG_ACCEL_CONFIG;
   	writeBuffer[1] = MPU9150_ACCEL_CONFIG_AFS_SEL_4G;
   	i2cTransaction.writeCount = 2;
   	i2cTransaction.readCount = 0;
   	if (!I2C_transfer(i2c, &i2cTransaction)) {
   		System_abort("setting sensor range failed");
   	}
}


void readMPU9150Task()
{
	if (i2c == NULL) System_abort("readMPU9150Task: i2c is not initialized");

	UInt eventPendingResult;
    I2C_Transaction i2cTransaction;
    uint8_t         writeBuffer[1];
    uint8_t			readBuffer[MPU9150_ACCEL_ONLY];

	i2cTransaction.slaveAddress = SENSOR_ADDRESS;
	i2cTransaction.writeBuf = writeBuffer;
	i2cTransaction.writeCount = 1;
	i2cTransaction.readBuf = readBuffer;
	i2cTransaction.readCount = MPU9150_ACCEL_ONLY;

	writeBuffer[0] = MPU9150_ACCEL_XOUT_H;

	int16_t x,y,z;
	int32_t val;
	//const float rangeScale = 9.81/MPU9150_ACCEL_CONFIG_2G_RANGE;

	while (1) {
		eventPendingResult = Event_pend(clockElapsedEventHandle, Event_Id_00, Event_Id_00, NULL);
		if (eventPendingResult != 0) {
			if (I2C_transfer(i2c, &i2cTransaction)) {
				x = ((readBuffer[0] << 8) | readBuffer[1]);
				y = ((readBuffer[2] << 8) | readBuffer[3]);
				z = ((readBuffer[4] << 8) | readBuffer[5]);

				if (x < 0) x = x * -1;
				if (y < 0) y = y * -1;
				if (z < 0) z = z * -1;
				val = x + y + z - MPU9150_ACCEL_CONFIG_4G_RANGE;

				System_printf("x: %d - y:%d - z:%d - sum:%d\n", x,y,z, val);
			    System_flush();
				/* do something clever with the values */
			}
		}
	}
}

void setupSensor() {
	Task_Params initializeMPU9150TaskParams;
	Task_Handle initializeMPU9150TaskHandle;
	Error_Block eb;

    Error_init(&eb);
    Task_Params_init(&initializeMPU9150TaskParams);
    initializeMPU9150TaskParams.stackSize = 1024;/*stack in bytes*/
    initializeMPU9150TaskParams.priority = 7;
    initializeMPU9150TaskHandle = Task_create((Task_FuncPtr)initializeMPU9150Task, &initializeMPU9150TaskParams, &eb);
    if (initializeMPU9150TaskHandle == NULL) {
    	System_abort("Creating initializeMPU9150Task failed");
    }
    else {
    	System_printf("created task initializeMPU9150Task\n");
    }
}

void setupPeriodicRead() {
	Task_Params readMPU9150TaskParams;
	Task_Handle readMPU9150TaskHandle;
	Error_Block eb;

    Error_init(&eb);
    Task_Params_init(&readMPU9150TaskParams);
    readMPU9150TaskParams.stackSize = 1024;/*stack in bytes*/
    readMPU9150TaskParams.priority = 5;
    readMPU9150TaskHandle = Task_create((Task_FuncPtr)readMPU9150Task, &readMPU9150TaskParams, &eb);
    if (readMPU9150TaskHandle == NULL) {
    	System_abort("Creating readMPU9150Task failed");
    }
    else {
    	System_printf("created task readMPU9150Task\n");
    }
}
