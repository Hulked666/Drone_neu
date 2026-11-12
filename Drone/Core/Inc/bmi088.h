

#ifndef INC_BMI088_H_
#define INC_BMI088_H_

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"

typedef struct{
	int16_t x;
	int16_t y;
	int16_t z;

}dataStruct_t;

typedef struct{
	dataStruct_t Accel;
	dataStruct_t Gyro;
}IMURAWStruct_t;


HAL_StatusTypeDef BMI088_Read_Raw(IMURAWStruct_t *imuData);



#endif /* INC_BMI088_H_ */
