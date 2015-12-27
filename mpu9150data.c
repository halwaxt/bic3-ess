/*
 * mpu9150data.c
 *
 *  Created on: 27.12.2015
 *      Author: Thomas Halwax
 */

#include <mpu9150data.h>


void getAcceleration(const Tmpu9150data * const sensorData, Tacceleration *sensorAcceleration) {

	sensorAcceleration->x = ((sensorData->rawValues[0] << 8) | sensorData->rawValues[1]);
	sensorAcceleration->y = ((sensorData->rawValues[2] << 8) | sensorData->rawValues[3]);
	sensorAcceleration->z = ((sensorData->rawValues[4] << 8) | sensorData->rawValues[5]);

	/*
				gVector = sqrt(x*x + y*y + z*z) - MPU9150_ACCEL_CONFIG_2G_RANGE;
				System_printf("x: %d # y:%d # z:%d # gVector:%f\n", x,y,z, gVector);
			    System_flush();
*/
}
