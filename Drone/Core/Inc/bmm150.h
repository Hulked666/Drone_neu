

#ifndef INC_BMM150_H_
#define INC_BMM150_H_

typedef struct{
	int16_t mag_x;
	int16_t mag_y;
	int16_t mag_z;
	uint16_t hall;
}MagnetRaw_t;

typedef struct{
	int8_t dig_x1;
	int8_t dig_y1;
	int16_t dig_z4;
	int8_t dig_x2;
	int8_t dig_y2;
	int16_t dig_z2;
	uint16_t dig_z1;
	uint16_t dig_xyz1;
	int16_t dig_z3;
	int8_t dig_xy2;
	uint8_t dig_xy1;
}TrimStruct_t;

typedef struct{
	float mag_x;
	float mag_y;
	float mag_z;
}MagData_t;


HAL_StatusTypeDef BMM150_Init(uint8_t mode);

HAL_StatusTypeDef BMM150_Raw(MagnetRaw_t *magData);
#endif /* INC_BMM150_H_ */
