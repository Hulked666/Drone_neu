

#ifndef INC_IMU_H_
#define INC_IMU_H_

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"


typedef struct{
	float acc_x;
	float acc_y;
	float acc_z;

	float gyr_x;
	float gyr_y;
	float gyr_z;

	uint32_t time_stamp;
}IMUStruct_t;

HAL_StatusTypeDef IMU_Data(IMUStruct_t *imuData);
float IMU_X_Angle(float gyro_x_rad_s, uint32_t time_stamp_us);
float IMU_Y_Angle(float gyro_y, uint32_t time_stamp);

float IMU_Accel_X_Angle(float acc_y, float acc_z);
float IMU_Accel_Y_Angle(float accel_x, float accel_z);

float IMU_X_Filter(IMUStruct_t *imuData);
float IMU_Y_Filter(IMUStruct_t *imuData);


#endif /* INC_IMU_H_ */
