#include "main.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define DEV_ADDR (0x10 << 1)
#define CHIP_ID 0x40
#define RESET_REG 0x4B
#define MODE_ADDR 0x4C
#define NORMAL_MODE 0x00
#define FASTER_MODE 0x28
#define DATA_ADDR_XY 0x42
#define DATA_ADDR_Z 0x46


extern I2C_HandleTypeDef hi2c2;

HAL_StatusTypeDef read_data(uint16_t regAddr, uint8_t *buffer, uint16_t dataSize){
	HAL_StatusTypeDef status;
	status = HAL_I2C_Mem_Read(&hi2c2,DEV_ADDR,regAddr,I2C_MEMADD_SIZE_8BIT, buffer,dataSize,100);
	return status;
}

HAL_StatusTypeDef write_data(uint16_t regAddr, uint8_t *buffer, uint16_t dataSize){
	HAL_StatusTypeDef status;
	status = HAL_I2C_Mem_Write(&hi2c2, DEV_ADDR, regAddr, I2C_MEMADD_SIZE_8BIT, buffer,dataSize,100);
	return status;
}

HAL_StatusTypeDef BMM150_Wakeup(void){
	HAL_StatusTypeDef status;
	uint8_t power_mode = 0x01;
	uint8_t soft_reset = 0x83;
	status = write_data(RESET_REG, &power_mode,1);
	if(status != HAL_OK){
		return status;
	}
	HAL_Delay(3);

	status = write_data(RESET_REG, &soft_reset,1);
	if(status != HAL_OK){
		return status;
	}
	HAL_Delay(1);

	return HAL_OK;
}

bool chip_id(void){
	uint8_t id_value = 0;
	read_data(CHIP_ID,&id_value,1);
	if((status != HAL_OK) || (id_value != 0x32)){
		return false;
	}

	return true;
}

HAL_StatusTypeDef BMM150_SetMode(uint8_t mode){
	HAL_StatusTypeDef status;
	status = write_data(MODE_ADDR, &mode,1);
	if(status != HAL_OK){
		return status;
	}
	return HAL_OK;
}

HAL_StatusTypeDef BMM150_Init(uint8_t mode){
	HAL_StatusTypeDef status;
	uint8_t mode_value = 0;
	status = BMM150_Wakeup();
	if(status != HAL_OK){
		return status;
	}
	if(chip_id() != true){
		return HAL_ERROR;
	}
	status = BMM150_SetMode(mode);
	if(status != HAL_OK){
		return status;
	}
	status = read_data(MODE_ADDR, &mode_value,1);
	if(status != HAL_OK){
		return status;
	}
	if(mode_value != mode){
		return HAL_ERROR;
	}

	return HAL_OK;
}

HAL_StatusTypeDef BMM150_Raw(MagnetRaw_t *magData){
	HAL_StatusTypeDef status;
	uint8_t data[8];
	status = read_data(DATA_ADDR_XY,data,8);
	if(status != HAL_OK){
		return status;
	}
	uint16_t raw_x = (uint16_t)(data[1]<<5) | (data[0]>>3);
	uint16_t raw_y = (uint16_t)(data[3]<<5) | (data[2]>>3);
	uint16_t raw_z = (uint16_t)(data[5]<<7) | (data[4]>>1);
	magData->hall= (uint16_t)(data[7]<<6) | (data[6] >>2);

	if(raw_x & 0x1000){
		raw_x |= 0xE000;
	}
	if(raw_y & 0x1000){
		raw_y |= 0xE000;
	}
	if(raw_z & 0x4000){
		raw_z |= 0x8000;
	}
	magData->mag_x = (int16_t)raw_x;
	magData->mag_y = (int16_t)raw_y;
	magData->mag_z = (int16_t)raw_z;

	return HAL_OK;
}

