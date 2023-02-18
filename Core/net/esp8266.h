#ifndef _ESP8266_H_
#define _ESP8266_H_

#include "stdint.h"

#define ESP8266_RXBUF_SIZE 128

/*定义接收状态枚举*/
typedef enum Esp8266RxStatus
{
	Esp8266_RxNone,	 // 未开始接收
	Esp8266_RxWait,	 // 正在接收中
	Esp8266_RxFinish // 接收完成
} Esp8266RxStatus_t;

/*定义命定发送结果枚举*/
typedef enum Esp8266TxStatus
{
	Esp8266_TxFial,	   // 发送失败
	Esp8266_RxSucceed, // 发送成功
} Esp8266TxStatus_t;

/*定义传输模式枚举*/
typedef enum Esp8266CIPMode
{
	Esp8266_NormalMode, // 正常模式
	Esp8266_TransMode	// 透传模式
} Esp8266CIPMode_t;

/*定义ESP8266的WIFI模式*/
typedef enum Esp8266CWMode
{
	Esp8266_StationMode, // station 模式
	Esp8266_SoftAPMode,	 // softAP 模式
	Esp8266_MixedMode	 // softAP + station 模式
} Esp8266CWMode_t;

/*定义网络连接信息枚举*/
typedef enum Esp8266CIPStatus
{
	Esp8266_Timeout = 0,	// 超时
	Esp8266_Error = 1,		// 错误
	Esp8266_GotIP = 2,		// 获得IP地址
	Esp8266_Connected = 3,	// 连接正常
	Esp8266_Disconnect = 4, // 断开连接
	Esp8266_NoWifi = 5		// 未连接到 WiFi
} Esp8266CIPStatus_t;

/* 预留拓展
typedef struct Esp8266Info
{
	Esp8266CWMode_t cwMode;	// WIFI模式
	Esp8266CIPMode_t cipMode; // 传输模式，正常或透传
} Esp8266Info_t;
*/
/*定义ESP8266对象*/
typedef struct Esp8266Object
{
	uint16_t rstPin;		  // 复位引脚
	Esp8266CWMode_t cwMode;	  // WIFI模式
	Esp8266CIPMode_t cipMode; // 传输模式，正常或透传
	struct EspRxBuffer
	{
		uint8_t queue[ESP8266_RXBUF_SIZE]; // 数据存储队列
		uint8_t lengthRecieving;		   // 正在接收的数据长度
		uint8_t lengthRecieved;			   // 已经接收的数据长度
	} rxBuffer;
	void (*SendData)(uint8_t *uData, uint16_t uSize); // 数据发送函数指针
	void (*Delayms)(volatile uint32_t nTime);		  // 延时操作指针
} Esp8266Object_t;

/*定义ESP8266数据发送指针类型*/
typedef void (*ESP8266SendData_t)(uint8_t *uData, uint16_t uSize);
/*延时操作指针*/
typedef void (*ESP8266Delayms_t)(volatile uint32_t nTime);

/*ESP8266对象初始化*/
void Esp8266_ObjecInit(Esp8266Object_t *esp,	 // ESP8266对象
					   uint16_t pinNum,			 // 复位引脚
					   Esp8266CWMode_t cwMode,	 // WIFI模式
					   Esp8266CIPMode_t cipMode, // 传输模式，正常或透传
					   char *wifiName,			 // WIFI名称
					   char *wifiPassword,		 // WIFI密码
					   ESP8266SendData_t send,	 // 发送函数指针
					   ESP8266Delayms_t delayms	 // 毫秒延时函数
);

/*ESP8266发送数据*/
void Esp8266_sendData(Esp8266Object_t *esp, uint8_t *uData, uint16_t uSize);

/*检查模块的连接是否正常*/
Esp8266CIPStatus_t Esp8266_CheckConnection(Esp8266Object_t *esp, uint16_t timeout);

#endif
//==================================================End=================================================
