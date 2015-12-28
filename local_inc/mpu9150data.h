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
#define MPU_GVECTOR_BASE 4096

typedef struct mpu9150data {
	uint8_t rawValues[MPU9150_SUPPORTED_SENSOR_BYTES];
} Tmpu9150data;

typedef struct acceleration {
	float x;
	float y;
	float z;
} Tacceleration;

#endif /* LOCAL_INC_MPU9150DATA_H_ */
