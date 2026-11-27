#include "main.h"
#include "bmm150.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define DEV_ADDR (0x13 << 1)
#define CHIP_ID 0x40
#define RESET_REG 0x4B
#define MODE_ADDR 0x4C
#define NORMAL_MODE 0x00
#define FASTER_MODE 0x28
#define DATA_ADDR_XY 0x42
#define DATA_ADDR_Z 0x46
#define FIRST_TRIM 0x5D
#define SECOND_TRIM 0x62
#define THIRD_TRIM 0x68


extern I2C_HandleTypeDef hi2c2;

static TrimStruct_t trimData;

HAL_StatusTypeDef BMM150_ReadTrim(TrimStruct_t *trimData);

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
	HAL_StatusTypeDef status;
	uint8_t id_value = 0;
	status = read_data(CHIP_ID,&id_value,1);
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

	status = BMM150_ReadTrim(&trimData);
	if(status != HAL_OK){
		return status;
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

HAL_StatusTypeDef BMM150_ReadTrim(TrimStruct_t *trimData){
	HAL_StatusTypeDef status;
	uint8_t first_trim[2];
	uint8_t second_trim[4];
	uint8_t third_trim[10];
	status = read_data(FIRST_TRIM,first_trim,2);
	if(status != HAL_OK){
		return status;
	}
	status = read_data(SECOND_TRIM,second_trim,4);
	if(status != HAL_OK){
		return status;
	}
	status = read_data(THIRD_TRIM,third_trim,10);
	if(status != HAL_OK){
		return status;
	}
	uint16_t dig_xyz1_raw = (uint16_t)(third_trim[5] << 8) | (third_trim[4]);
	dig_xyz1_raw &= 0x7FFF;

	trimData->dig_x1 = (int8_t)(first_trim[0]);
	trimData->dig_y1 = (int8_t)(first_trim[1]);
	trimData->dig_z4 = (int16_t)(second_trim[1]<<8) | (second_trim[0]);
	trimData->dig_x2 = (int8_t)(second_trim[2]);
	trimData->dig_y2 = (int8_t)(second_trim[3]);
	trimData->dig_z2 = (int16_t)(third_trim[1] << 8) | (third_trim[0]);
	trimData->dig_z1 = (uint16_t)(third_trim[3] << 8) | (third_trim[2]);
	trimData->dig_xyz1 = (uint16_t)dig_xyz1_raw;
	trimData->dig_z3 = (int16_t)(third_trim[7] << 8) | (third_trim[6]);
	trimData->dig_xy2 = (int8_t)(third_trim[8]);
	trimData->dig_xy1= (uint8_t)(third_trim[9]);

	return HAL_OK;
}

HAL_StatusTypeDef BMM150_Compens(MagData_t * compData){
	if(compData == NULL){
		return HAL_ERROR;
	}
	HAL_StatusTypeDef status;
	MagnetRaw_t magData;
	status = BMM150_Raw(&magData);
	if(status != HAL_OK){
		return status;
	}

	if(magData.hall == 0){
		return HAL_ERROR;
	}

	if(magData.mag_x == -4096){
		return HAL_ERROR;
	}

	if(magData.mag_y == -4096){
		return HAL_ERROR;
	}

	if(magData.mag_z == -16384){
		return HAL_ERROR;
	}

	if((trimData.dig_xyz1 == 0) || (trimData.dig_z1 == 0) || (trimData.dig_z2 == 0)){
		return HAL_ERROR;
	}

	float devider = (64.0f*((float)trimData.dig_z2 + (((float)trimData.dig_z1 * (float)magData.hall)/32768.0f)));
	if(devider == 0.0f){
		return HAL_ERROR;
	}
	float hall_corr = (((float)trimData.dig_xyz1 * 16384.0f) / (float)magData.hall) - 16384.0f;
	float XY_corr  = ((float)trimData.dig_xy2 *((hall_corr * hall_corr) / 268435456.0f)) + ((float)trimData.dig_xy1*(hall_corr / 16384.0f));

	compData->mag_x = (((float)magData.mag_x *(XY_corr + 256.0f)*((float)trimData.dig_x2 +160.0f)/8192.0f) + (8.0f * (float)trimData.dig_x1)) / 16.0f;
	compData->mag_y = (((float)magData.mag_y *(XY_corr + 256.0f)*((float)trimData.dig_y2 +160.0f)/8192.0f) + (8.0f * (float)trimData.dig_y1)) / 16.0f;
	compData->mag_z = (((float)magData.mag_z - (float)trimData.dig_z4) * 131072.0f - (float)trimData.dig_z3 *((float)magData.hall - (float)trimData.dig_xyz1)) / devider ;

	return HAL_OK;

}

