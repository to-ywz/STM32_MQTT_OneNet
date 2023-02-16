/**
 * @file esp8266.c
 * @author BlackSheep (blacksheep.208h@gmail.com)
 * @brief 	esp8266 方法
 * @version 0.1
 * @date 2023-02-16
 *
 * @copyright Copyright (c) 2023
 * @note 基于 野火STM32 ESP8266 模板修改
 */
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "bsp_gpio.h"
#include "esp8266.h"

/*定义WIFI模式配置指令*/
char cwModeCmd[3][17] = {"AT+CWMODE_CUR=1\r\n",
						 "AT+CWMODE_CUR=2\r\n",
						 "AT+CWMODE_CUR=3\r\n"};

/*检查数据时是否接受完成*/
static Esp8266RxStatus_t isRecieveFinished(Esp8266Object_t *esp);
/*清除接收缓冲区*/
static void clearReciveBuffer(Esp8266Object_t *esp);
/*ESP8266发送命令*/
static Esp8266TxStatus_t Esp8266_sendCommmand(Esp8266Object_t *esp, char *cmd, char *ack, uint16_t timeOut);
/*ESP8266发送数据*/
// static void Esp8266_sendData(Esp8266Object_t *esp,uint8_t *uData, uint16_t uSize);
/*ESP8266模块进入透传模式*/
static Esp8266TxStatus_t Esp8266_enterTrans(Esp8266Object_t *esp);

static void Esp8266_Restart(Esp8266Object_t *esp);

/**
 * @brief 				ESP8266对象初始化
 *
 * @param esp 			ESP8266对象
 * @param pinNum 		复位引脚
 * @param cwMode 		WIFI模式
 * @param cipMode 		传输模式，正常或透传
 * @param wifiName 		WIFI名称
 * @param wifiPassword 	WIFI密码
 * @param send 			发送函数指针
 * @param delayms 		毫秒延时函数
 */
void Esp8266_ObjecInit(Esp8266Object_t *esp,
					   uint16_t pinNum,
					   Esp8266CWMode_t cwMode,
					   Esp8266CIPMode_t cipMode,
					   char *wifiName,
					   char *wifiPassword,
					   ESP8266SendData_t send,
					   ESP8266Delayms_t delayms)
{
	char cwjap[50];
	char cwsap[50];

	if ((esp == NULL) || (send == NULL) || (delayms == NULL))
	{
		return;
	}
	esp->SendData = send;
	esp->Delayms = delayms;

	esp->rstPin = pinNum;
	esp->cwMode = cwMode;
	esp->cipMode = cipMode;

	esp->rxBuffer.lengthRecieved = 0;
	clearReciveBuffer(esp);

	Esp8266_Restart(esp);

	uint8_t times = 0;
	printf("Configure ESP8266 Mode\r\n");
	// 设置工作模式 1：station模式   2：AP模式  3：兼容 AP+station模式
	while (Esp8266_sendCommmand(esp, cwModeCmd[esp->cwMode], "OK", 50) == Esp8266_TxFial)
	{
		printf("Try to reconfigure the working mode agin.(%d)\r\n", times++);
	}

	times = 0;
	// 让Wifi模块重启的命令
	while (Esp8266_sendCommmand(esp, "AT+RST\r\n", "OK", 20) == Esp8266_TxFial)
	{
		printf("Try to restart the device agin.(%d)\r\n", times++);
	}

	esp->Delayms(3000); // 延时3S等待重启成功

	if (esp->cwMode == Esp8266_StationMode)
	{
		sprintf(cwjap, "AT+CWJAP_CUR=\"%s\",\"%s\"\r\n", wifiName, wifiPassword);

		times = 0;
		// 让模块连接上自己的路由
		while (Esp8266_sendCommmand(esp, cwjap, "OK", 600) == Esp8266_TxFial)
		{
			printf("Try to join the Wi-Fi agin.(%d)\r\n", times++);
		}

		if (esp->cipMode == Esp8266_TransMode)
		{
			times = 0;
			while (Esp8266_enterTrans(esp) == Esp8266_TxFial)
			{
				printf("Try to reconfigure the send mode agin.(%d)\r\n", times++);
			}
		}
		else
		{
			//=0：单路连接模式     =1：多路连接模式
			if (Esp8266_sendCommmand(esp, "AT+CIPMUX=0\r\n", "OK", 20) == Esp8266_TxFial)
			{
				printf("Try to reconfigure the link mode agin.(%d)\r\n", times++);
			}
		}
	}
	else if (esp->cwMode == Esp8266_SoftAPMode)
	{
		sprintf(cwsap, "AT+CWSAP_CUR=\"%s\",\"%s\"\r\n", wifiName, wifiPassword);

		// 设置模块的WIFI名和密码
		if (Esp8266_sendCommmand(esp, cwsap, "OK", 600) == Esp8266_TxFial)
		{
			printf("Try to reconfigure the AP information agin.(%d)\r\n", times++);
		}
	}
	else if (esp->cwMode == Esp8266_MixedMode)
	{
		printf("Try to reconfigure the working mode agin.(%d)\r\n", times++);
		// 尚未使用，有待添加
	}
}

/**
 * @brief 			模块进入透传模式
 *
 * @param esp 		esp 对象
 * @retval 			0: 成功
 * 					1: 失败
 *
 * !@note			连接云端服务器,在这里配置
 */
static Esp8266TxStatus_t Esp8266_enterTrans(Esp8266Object_t *esp)
{
	Esp8266TxStatus_t status = Esp8266_RxSucceed;

	//=0：单路连接模式     =1：多路连接模式
	status = Esp8266_sendCommmand(esp, "AT+CIPMUX=0\r\n", "OK", 20);
	if (status == Esp8266_TxFial)
	{
		return status;
	}

	// 建立TCP连接  这四项分别代表了 要连接的ID号0~4   连接类型  远程服务器IP地址   远程服务器端口号
	//  while(Esp8266_sendCommmand(esp,"AT+CIPSTART=\"TCP\",\"xxx.xxx.xxx.xxx\",xxxx","CONNECT",200));

	// 是否开启透传模式  0：表示关闭 1：表示开启透传
	status = Esp8266_sendCommmand(esp, "AT+CIPMODE=1\r\n", "OK", 200);
	if (status == Esp8266_TxFial)
	{
		return status;
	}

	// 透传模式下 开始发送数据的指令 这个指令之后就可以直接发数据了
	status = Esp8266_sendCommmand(esp, "AT+CIPSEND\r\n", "OK", 50);

	return status;
}

/**
 * @brief 		退出透传
 *
 * @param esp 	esp对象
 * @retval 		0:		成功
 * 				1:		失败
 */
static Esp8266TxStatus_t Esp8266QuitTrans(Esp8266Object_t *esp)
{
	Esp8266TxStatus_t status = Esp8266_RxSucceed;

	status = Esp8266_sendCommmand(esp, "+++", "OK", 20);

	if (status == Esp8266_TxFial)
	{
		return status;
	}

	esp->Delayms(1000);

	status = Esp8266_sendCommmand(esp, "AT\r\n", "OK", 20);

	return status;
}

/*检查模块设备是否存在*/
// void CheckForEquipment(Esp8266Object_t *esp)
// {
// 	Esp8266_sendCommmand(esp, "AT\r\n", "OK", 20);
// }

/**/
/**
 * @brief 				AP连接是否正常
 *
 * @param esp 			esp对象
 * @param timeOut 		超时时间
 * @retval 				Esp8266_Timeout		: 超时
 * 						Esp8266_Error		: 错误
 * 						Esp8266_GotIP		: 获得IP地址
 * 						Esp8266_Connected	: 连接正常
 * 						Esp8266_Disconnect 	: 断开连接
 * 						Esp8266_NoWifi		: 未连接到 WiFi
 */
Esp8266CIPStatus_t Esp8266_CheckConnection(Esp8266Object_t *esp, uint16_t timeOut)
{
	Esp8266CIPStatus_t status = Esp8266_Error;

	if (esp->cipMode == Esp8266_TransMode)
	{
		Esp8266QuitTrans(esp);
	}

	clearReciveBuffer(esp);
	esp->SendData((unsigned char *)"AT+CIPSTATUS\r\n", 14);

	while (--timeOut)
	{
		if (isRecieveFinished(esp) == Esp8266_RxFinish)
		{
			if (strstr((const char *)esp->rxBuffer.queue, "STATUS:2")) // 获得IP
			{
				status = Esp8266_GotIP;
			}
			else if (strstr((const char *)esp->rxBuffer.queue, "STATUS:3")) // 建立连接
			{
				status = Esp8266_Connected;
			}
			else if (strstr((const char *)esp->rxBuffer.queue, "STATUS:4")) // 失去连接
			{
				status = Esp8266_Disconnect;
			}
			else if (strstr((const char *)esp->rxBuffer.queue, "STATUS:5")) // 物理掉线
			{
				status = Esp8266_NoWifi;
			}

			break;
		}

		esp->Delayms(10);
	}

	if (timeOut == 0)
	{
		status = Esp8266_Timeout;
	}

	return status;
}

/**
 * @brief 			发送命令
 *
 * @param esp		esp 对象
 * @param cmd		命令
 * @param ack		响应
 * @param timeOut	超时时间
 * @retval			Esp8266_RxSucceed	: 发送成功
 * 					Esp8266_TxFial		: 发送失败
 */
static Esp8266TxStatus_t Esp8266_sendCommmand(Esp8266Object_t *esp, char *cmd, char *ack, uint16_t timeOut)
{
	esp->SendData((unsigned char *)cmd, strlen((const char *)cmd)); // 写命令到网络设备

	if (ack && timeOut)
	{
		while (timeOut--) // 等待超时
		{
			if (isRecieveFinished(esp) == Esp8266_RxFinish) // 如果数据接收完成
			{
				if (strstr((const char *)esp->rxBuffer.queue, ack) != NULL) // 如果检索到关键词
				{
					clearReciveBuffer(esp);

					return Esp8266_RxSucceed;
				}
			}

			esp->Delayms(10);
		}
	}

	return Esp8266_TxFial;
}

/**
 * @brief 			检测数据是否接受完毕
 *
 * @param esp 		esp对象
 * @retval 			Esp8266_RxNone	: 等待接受数据
 * 					Esp8266_RxWait	: 正在接收数据
 * 					Esp8266_RxFinish: 数据接受完毕
 */
static Esp8266RxStatus_t isRecieveFinished(Esp8266Object_t *esp)
{
	// 尚未开始接收数据
	if (esp->rxBuffer.lengthRecieving == 0)
	{
		return Esp8266_RxNone;
	}

	// 完成接收数据
	if (esp->rxBuffer.lengthRecieving == esp->rxBuffer.lengthRecieved)
	{
		esp->rxBuffer.lengthRecieving = 0;

		return Esp8266_RxFinish;
	}

	// 正在接收数据
	esp->rxBuffer.lengthRecieved = esp->rxBuffer.lengthRecieving;

	return Esp8266_RxWait;
}

/**
 * @brief 		清空缓存区
 *
 * @param esp 	esp对象
 */
static void clearReciveBuffer(Esp8266Object_t *esp)
{
	esp->rxBuffer.lengthRecieving = 0;

	memset(esp->rxBuffer.queue, 0, sizeof(esp->rxBuffer.queue));
}

/**
 * @brief 		复位设备
 *
 * @param esp 	esp对象
 */
static void Esp8266_Restart(Esp8266Object_t *esp)
{
	bsp_pin_write(esp->rstPin, 0);
	esp->Delayms(1000);
	bsp_pin_write(esp->rstPin, 1);
}

//==================================================End=================================================
