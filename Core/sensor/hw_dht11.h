#ifndef __HW_DHT11__
#define __HW_DHT11__

#include "dht11.h"

typedef struct DHT11
{
    uint8_t num;         // 编号
    Dht11_Object_t obj; // DHT11 基本操作

    void (*dataUpdate)(void); // 获取数据方法
} Dht11_t;

void DHT11_Init(void);

extern Dht11_t G_dht11;

#endif
//==================================================End=================================================

