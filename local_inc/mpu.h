/*
 * mpu.h
 *
 *  Created on: 02.12.2015
 *      Author: Thomas Halwax
 */

#ifndef MPU_H_
#define MPU_H_

#include <ti/sysbios/knl/Event.h>

#define SENSOR_ADDRESS 0b1101001
#define MPU9150_PWR_MGMT_1 0x6B
/* reset device */
#define MPU9150_PWR_MGMT_1_DEVICE_RESET 0b10000000
/* sleep state */
#define MPU9150_PWR_MGMT_1_SLEEP 0b01000000

/* Upon power up, the MPU-9150 clock source defaults to the
 * internal oscillator. However, it is highly recommended that the device be configured
 * to use one of the gyroscopes
 */

// PLL with X-axis gyro reference
#define MPU9150_PWR_MGMT_1_CLKSEL_XG 0b00000001

/* start address of ACCEL_XH register */
#define MPU9150_ACCEL_XOUT_H 0x3B
#define MPU9150_ACCEL_ONLY 6
#define MPU9150_SMPLRT_DIV 0x19
#define MPU9150_REG_ACCEL_CONFIG 0x1C
#define MPU9150_ACCEL_CONFIG_AFS_SEL_2G 0b00000000
#define MPU9150_ACCEL_CONFIG_2G_RANGE 16384
#define MPU9150_ACCEL_CONFIG_AFS_SEL_4G 0b00001000
#define MPU9150_ACCEL_CONFIG_4G_RANGE 8192
#define MPU9150_USER_CTRL_MASTER_DISABLE 0x00
#define MPU9150_REG_USER_CTRL 0x6A
#define MPU9150_REG_CONFIG 0x1A
#define MPU9150_CONFIG_DLPF_44HZ 0b00000011
#define MPU9150_CONFIG_DLPF_21HZ 0b00000100

extern volatile I2C_Handle i2c;
extern volatile Event_Handle clockElapsedEventHandle;


void initializeBus(void);
void setupSensor(void);
void readMPU9150Task(void);
void setupPeriodicRead(void);


#endif /* MPU_H_ */
