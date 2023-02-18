#include "bsp_esp8266.h"
#include "bsp_uart.h"
#include "delay.h"

Esp8266Object_t esp8266;

static void Esp8266_HW_sendData(uint8_t *uData, uint16_t uSize);

void esp8266Init(void)
{
	uart_init();

	Esp8266_ObjecInit(&esp8266,
					  macUser_ESP8266_RESTART_PIN,
					  Esp8266_StationMode,
					  Esp8266_TransMode,
					  macUser_ESP8266_ApSsid,
					  macUser_ESP8266_ApPwd,
					  Esp8266_HW_sendData,
					  delay_ms);

	printf("ESP8266 is initialized.\r\n");
}

static void Esp8266_HW_sendData(uint8_t *uData, uint16_t uSize)
{
	UARTx_SendData(uart2.info.huart, uData, uSize);
}

//==================================================End=================================================
