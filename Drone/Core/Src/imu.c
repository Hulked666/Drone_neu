#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include "bmi088.h"
#include "imu.h"

extern TIM_HandleTypeDef htim2;



HAL_StatusTypeDef IMU_Data(IMUStruct_t *imuData){
	IMURAWStruct_t rawData;
	HAL_StatusTypeDef status;
	if(imuData == NULL){
		return HAL_ERROR;
	}

	uint32_t timestamp_us = __HAL_TIM_GET_COUNTER(&htim2);
	status = BMI088_Read_Raw(&rawData);
	if(status!= HAL_OK){
		return status;
	}

	imuData->acc_x = ((float)rawData.Accel.x / 32768.0f) * 6.0f * 9.80665f;
	imuData->acc_y = ((float)rawData.Accel.y / 32768.0f) * 6.0f * 9.80665f;
	imuData->acc_z = ((float)rawData.Accel.z / 32768.0f) * 6.0f * 9.80665f;

	imuData->gyr_x = (((float)rawData.Gyro.x/32768.0f)*2000.0f * 0.01745329252f)- 0.00202f;
	imuData->gyr_y = (((float)rawData.Gyro.y/32768.0f)*2000.0f * 0.01745329252f)- 0.00392f;
	imuData->gyr_z = (((float)rawData.Gyro.z/32768.0f)*2000.0f * 0.01745329252f)- 0.00070f;

	imuData->time_stamp = timestamp_us;

	return HAL_OK;

}

float IMU_X_Angle(float gyro_x_rad_s, uint32_t time_stamp_us){
	static float angle = 0.0f;
	static uint32_t prev_time_stamp;
	static bool first_stamp = true;

	if(first_stamp){
		prev_time_stamp = time_stamp_us;
		first_stamp = false;
		return angle;
	}

	uint32_t elapsed_time = time_stamp_us - prev_time_stamp;
	float elapsed_s = (float)elapsed_time / 1000000.0f;

	angle += gyro_x_rad_s * elapsed_s;
	prev_time_stamp = time_stamp_us;

	return angle;
}

float IMU_Accel_X_Angle(float acc_y, float acc_z)
{
    return atan2f(acc_y, acc_z);
}

float IMU_X_Filter(IMUStruct_t *imuData){
	static float angle = 0.0f;
	static uint32_t prev_time_stamp;
	static bool first_stamp = true;
	float acc_angle = IMU_Accel_X_Angle(imuData->acc_y, imuData->acc_z);

	if(first_stamp){
		prev_time_stamp = imuData->time_stamp;
		first_stamp = false;
		return angle = acc_angle;
	}
	uint32_t elapsed = imuData->time_stamp - prev_time_stamp;
	float elapsed_t = (float)elapsed/ 1000000.0f;
	angle += imuData->gyr_x *elapsed_t;
	prev_time_stamp = imuData->time_stamp;

	angle = (0.98f * (angle)) + (0.02f * (acc_angle));

	return angle;

}

float IMU_Y_Angle(float gyro_y, uint32_t time_stamp){
	static float angle = 0.0f;
	static uint32_t prev_time_stamp;
	static bool first_stamp = true;

	if(first_stamp){
		prev_time_stamp = time_stamp;
		first_stamp = false;
		return angle;
	}

	uint32_t elapsed_time = time_stamp - prev_time_stamp;
	float elapsed_s = (float)elapsed_time / 1000000.0f;

	angle += gyro_y * elapsed_s;
	prev_time_stamp = time_stamp;

	return angle;
}

float IMU_Accel_Y_Angle(float accel_x, float accel_z){
	return atan2f((accel_x)*(-1), accel_z);
}

float IMU_Y_Filter(IMUStruct_t *imuData){
	static float angle = 0.0f;
	static uint32_t last_stamp;
	static bool first_stamp = true;
	float corr_angle = IMU_Accel_Y_Angle(imuData->acc_x, imuData->acc_z);

	if(first_stamp){
		last_stamp = imuData->time_stamp;
		first_stamp = false;
		return angle = corr_angle;
	}

	uint32_t elapsed_t = imuData->time_stamp - last_stamp;
	float elapsed_s = (float)elapsed_t / 1000000.0f;
	last_stamp = imuData->time_stamp;

	angle += imuData->gyr_y * elapsed_s;

	angle = (0.98f * (angle)) + (0.02f * (corr_angle));

	return angle;


}


