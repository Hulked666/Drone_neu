

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


#endif /* INC_IMU_H_ */
