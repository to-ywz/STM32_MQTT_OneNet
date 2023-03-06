#ifndef __DHT11__
#define __DHT11__

#include "dht11_ops.h"

void dht11_init(void);
void dht11_data_update(uint8_t id);
float dht11_get_humidity(uint8_t id);
float dht11_get_temperature(uint8_t id);

#endif
//==================================================End=================================================
