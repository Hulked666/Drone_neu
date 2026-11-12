#include "main.h"
#include "bmi088.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define ACC_ADRESS (0x19 << 1)
#define GYRO_ADRESS (0x68 << 1)
#define ID_ADRESS 0x00
#define ACC_ID 0x1E
#define GYR_ID 0x0F
#define ACC_RESET 0x7E
#define GYRO_RESET 0x14
#define ACC_PWR_CONF 0x7C
#define ACC_PWR_CTRL 0x7D
#define ACC_RNG 0x41
#define ACC_CONF 0x40
#define GYRO_RNG 0x0F
#define GYRO_BWP 0x10
#define GYRO_PWR 0x11
#define ACC_DATA 0x12
#define GYRO_DATA 0x02


extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart2;

HAL_StatusTypeDef BMI_ACC_read(uint16_t regAdr, uint8_t *buffer, uint16_t size){
	HAL_StatusTypeDef status;
	status = HAL_I2C_Mem_Read(&hi2c1,ACC_ADRESS, regAdr,I2C_MEMADD_SIZE_8BIT,buffer,size, 100);
	return status;
}

HAL_StatusTypeDef BMI_ACC_write(uint16_t regAdr, uint8_t *buffer, uint16_t size){
	HAL_StatusTypeDef status;
	status = HAL_I2C_Mem_Write(&hi2c1,ACC_ADRESS, regAdr,I2C_MEMADD_SIZE_8BIT,buffer,size, 100);
	return status;
}


HAL_StatusTypeDef BMI_GYRO_read(uint16_t regAdr, uint8_t *pBuffer, uint16_t size){
	HAL_StatusTypeDef status;
	status = HAL_I2C_Mem_Read(&hi2c1, GYRO_ADRESS, regAdr, I2C_MEMADD_SIZE_8BIT,pBuffer,size,100);
	return status;

}
 HAL_StatusTypeDef BMI_GYRO_write(uint16_t regAdr, uint8_t *pBuffer, uint16_t size){
	 HAL_StatusTypeDef status;
	 status = HAL_I2C_Mem_Write(&hi2c1,GYRO_ADRESS, regAdr, I2C_MEMADD_SIZE_8BIT, pBuffer, size,100);

	 return status;
 }

static bool chip_id(void){
	HAL_StatusTypeDef status;
	uint8_t acc_chip_id = 0;
	uint8_t gyr_chip_id = 0;
	status = BMI_ACC_read(ID_ADRESS,&acc_chip_id,1);
	if((status != HAL_OK) || (ACC_ID != acc_chip_id)){
		return false;

	}
	status = BMI_GYRO_read(ID_ADRESS, &gyr_chip_id,1);
	if((status != HAL_OK) || (GYR_ID != gyr_chip_id)){
		return false;
		}
	return true;

}

HAL_StatusTypeDef ACC_reset(void){
	HAL_StatusTypeDef status;
	uint8_t reset_value = 0xB6;
	status = BMI_ACC_write(ACC_RESET,&reset_value,1);
	HAL_Delay(2);
	return status;
}
HAL_StatusTypeDef GYR_reset(void){
	HAL_StatusTypeDef status;
	uint8_t reset_value = 0xB6;
	status = BMI_GYRO_write(GYRO_RESET, &reset_value,1);
	HAL_Delay(30);
	return status;

}

HAL_StatusTypeDef BMI_reset(void){
	HAL_StatusTypeDef status;
	status = ACC_reset();
	if (status != HAL_OK){
		return status;
	}
	HAL_Delay(1);
	status = GYR_reset();
	if(status != HAL_OK){
		return status;
	}

	HAL_Delay(30);
	return status;
}


HAL_StatusTypeDef ACC_Active(void){
	HAL_StatusTypeDef status;
	uint8_t act_value = 0x00;
	uint8_t acc_on = 0x04;

	status = BMI_ACC_write(ACC_PWR_CTRL,&acc_on,1);
	if(status != HAL_OK){
		return status;
	}

	HAL_Delay(5);

	status = BMI_ACC_write(ACC_PWR_CONF,&act_value,1);
	if(status != HAL_OK){
		return status;
	}
	HAL_Delay(1);

	return HAL_OK;
}

HAL_StatusTypeDef ACC_RANGE_set(uint8_t range){
	HAL_StatusTypeDef status;
    if (range > 0x03)
    {
        return HAL_ERROR;
    }
	status = BMI_ACC_write(ACC_RNG,&range,1);
	if(status != HAL_OK){
		return status;
	}

	return HAL_OK;
}

HAL_StatusTypeDef ACC_ODR_set(uint8_t value){
	HAL_StatusTypeDef status;
	uint8_t reg_value;
    if (value < 0x05 || value > 0x0C)
    {
        return HAL_ERROR;
    }
  status = BMI_ACC_read(ACC_CONF, &reg_value,1);
  if(status != HAL_OK){
	  return status;
  }

  reg_value = (reg_value & 0xF0) | value;

  return BMI_ACC_write(ACC_CONF, &reg_value,1);

}

HAL_StatusTypeDef ACC_BWP_set(uint8_t value){
	HAL_StatusTypeDef status;
	uint8_t reg_value;
	if(value < 0x08 || value > 0x0A){
		return HAL_ERROR;
	}
	status = BMI_ACC_read(ACC_CONF, &reg_value,1);
	if(status != HAL_OK){
		return status;
	}

	reg_value = (reg_value & 0x0F) | (value << 4);

	return BMI_ACC_write(ACC_CONF, &reg_value,1);
}

HAL_StatusTypeDef GYRO_RANGE_set(uint8_t value){
	if(value > 0x04){
		return HAL_ERROR;
	}
	return BMI_GYRO_write(GYRO_RNG, &value,1);

}

HAL_StatusTypeDef GYRO_BWP_set(uint8_t value){
	if (value > 0x07){
		return HAL_ERROR;
	}
	return BMI_GYRO_write(GYRO_BWP, &value,1);
}

HAL_StatusTypeDef GYRO_activate(void){
	uint8_t mode_value = 0x00;
	return BMI_GYRO_write(GYRO_PWR,&mode_value,1);
}

HAL_StatusTypeDef BMI088_Init(uint8_t accRng, uint8_t accOdr, uint8_t accBwp,uint8_t gyrRng, uint8_t gyrBwp){
	HAL_StatusTypeDef status;

	if( chip_id() != true){
		return HAL_ERROR;
	}
	status = BMI_reset();
	if(status != HAL_OK){
		return status;
	}
	if( chip_id() != true){
		return HAL_ERROR;
	}
	status = ACC_Active();
	if(status != HAL_OK){
		return status;
	}
	status = GYRO_activate();
	if(status != HAL_OK){
		return status;
	}

	HAL_Delay(30);

	status = ACC_RANGE_set(accRng);
	if(status != HAL_OK){
		return status;
	}
	status = GYRO_RANGE_set(gyrRng);
	if(status != HAL_OK){
		return status;
	}
	status  = ACC_ODR_set(accOdr);
	if(status != HAL_OK){
		return status;
	}
	status = ACC_BWP_set(accBwp);
	if(status != HAL_OK){
		return status;
	}
	status = GYRO_BWP_set(gyrBwp);
	if(status != HAL_OK){
		return status;
	}

	return HAL_OK;

}

HAL_StatusTypeDef ACC_Read_Raw(dataStruct_t *accData){
	HAL_StatusTypeDef status;
	uint8_t buff[6];
	if(accData == NULL){
		return HAL_ERROR;
	}
	status = BMI_ACC_read(ACC_DATA,buff,6);
	if(status != HAL_OK){
		return status;
	}
	accData->x = (int16_t)(((uint16_t) buff[1] << 8) | (buff[0]));
	accData->y = (int16_t)(((uint16_t) buff[3] << 8) | (buff[2]));
	accData->z = (int16_t)(((uint16_t) buff[5] << 8) | (buff[4]));
	return HAL_OK;
}

HAL_StatusTypeDef GYRO_Read_Raw(dataStruct_t *gyroData){
	HAL_StatusTypeDef status;
	uint8_t buff[6];
	if(gyroData == NULL){
		return HAL_ERROR;
	}
	status = BMI_GYRO_read(GYRO_DATA, buff,6);
	if(status != HAL_OK){
		return status;
	}

	gyroData->x = (int16_t)(((uint16_t) buff[1] << 8) | (buff[0]));
	gyroData->y = (int16_t)(((uint16_t) buff[3] << 8) | (buff[2]));
	gyroData->z = (int16_t)(((uint16_t) buff[5] << 8) | (buff[4]));

	return HAL_OK;
}

HAL_StatusTypeDef BMI088_Read_Raw(IMURAWStruct_t *imuData){
	HAL_StatusTypeDef status;
	dataStruct_t accData;
	dataStruct_t gyroData;
	if(imuData == NULL){
		return HAL_ERROR;
	}
	status = ACC_Read_Raw(&accData);
	if(status != HAL_OK){
		return status;
	}
	status = GYRO_Read_Raw(&gyroData);
	if(status != HAL_OK){
		return status;
	}

	imuData->Accel = accData;
	imuData->Gyro = gyroData;

	return HAL_OK;
}




