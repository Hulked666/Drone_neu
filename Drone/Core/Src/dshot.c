#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "main.c"


uint16_t DSHOT_CreatePacket(uint16_t throttle, bool telemetry){
	uint8_t checksum = 0;
	uint16_t packet = 0;
	if(throttle > 2047){
		throttle = 2047;
	}

	uint16_t value = (throttle << 1) | (telemetry);
	uint16_t copy_value = value;
	for(int i = 0; i < 3; i++){
		checksum = checksum ^ copy_value ;
		copy_value >>= 4;
	}
	checksum &= 0x0F;
	packet = (value << 4) | (checksum);

	return packet;
}
