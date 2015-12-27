/*
 * mpu9150data.h
 *
 *  Created on: 27.12.2015
 *      Author: Thomas Halwax
 */

#ifndef LOCAL_INC_MPU9150DATA_H_
#define LOCAL_INC_MPU9150DATA_H_

#include <stdint.h>

#define MPU9150_SUPPORTED_SENSOR_BYTES 14

typedef struct mpu9150data {
	uint8_t rawValues[MPU9150_SUPPORTED_SENSOR_BYTES];
} Tmpu9150data;

typedef struct acceleration {
	int16_t x;
	int16_t y;
	int16_t z;
} Tacceleration;

void getAcceleration(const Tmpu9150data * const sensorData, Tacceleration *sensorAcceleration);

#endif /* LOCAL_INC_MPU9150DATA_H_ */
