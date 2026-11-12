#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "bmi088.h"



HAL_StatusTypeDef IMU_Data(IMUStruct_t *imuData){
	IMURAWStruct_t rawData;
	HAL_StatusTypeDef status;
	if(imuData == NULL){
		return HAL_ERROR;
	}
	status = BMI088_Read_Raw(&rawData);
	if(status!= HAL_OK){
		return status;
	}

	imuData->acc_x = ((float)rawData.Accel.x / 32768.0f) * 6.0f * 9.80665f;
	imuData->acc_y = ((float)rawData.Accel.y / 32768.0f) * 6.0f * 9.80665f;
	imuData->acc_z = ((float)rawData.Accel.z / 32768.0f) * 6.0f * 9.80665f;

	imuData->gyr_x = ((float)rawData.Gyro.x/32768.0f)*2000.0f * 0.01745329252f;
	imuData->gyr_y = ((float)rawData.Gyro.y/32768.0f)*2000.0f * 0.01745329252f;
	imuData->gyr_z = ((float)rawData.Gyro.z/32768.0f)*2000.0f * 0.01745329252f;

	return HAL_OK;

}


