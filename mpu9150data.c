/*
 * mpu9150data.c
 *
 *  Created on: 27.12.2015
 *      Author: Thomas Halwax
 */

#include <mpu9150data.h>
#include <stdint.h>
#include <xdc/std.h>
#include <ctype.h>

float getTemperatureInCelsius(const Tmpu9150data *sensorData) {
	int16_t tempData = (sensorData->rawValues[6] << 8) | sensorData->rawValues[7];
	/* read the f*****g manual */

	return (tempData / 340.f) + 35.f;
}

void getGyroValues(const Tmpu9150data *sensorData, Tgyro *gyroValues) {
	int16_t x,y,z;

	x = (sensorData->rawValues[8] << 8) | sensorData->rawValues[9];
	y = (sensorData->rawValues[10] << 8) | sensorData->rawValues[11];
	z = (sensorData->rawValues[12] << 8) | sensorData->rawValues[13];

	gyroValues->x = x / 131.f;
	gyroValues->y = y / 131.f;
	gyroValues->z = z / 131.f;
}
