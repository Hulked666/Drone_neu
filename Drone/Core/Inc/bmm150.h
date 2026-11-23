

#ifndef INC_BMM150_H_
#define INC_BMM150_H_

typedef struct{
	int16_t mag_x;
	int16_t mag_y;
	int16_t mag_z;
	uint16_t hall;
}MagnetRaw_t;

#endif /* INC_BMM150_H_ */
