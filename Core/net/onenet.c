/**
 ************************************************************
 ************************************************************
 ************************************************************
 *	文件名： 	onenet.c
 *
 *	作者： 		张继瑞
 *
 *	日期： 		2017-05-09
 *
 *	版本： 		V1.1
 *
 *	说明： 		与onenet平台的数据交互接口层
 *
 *	修改记录：
 ************************************************************
 ************************************************************
 ************************************************************
 **/

// 单片机头文件

// 网络设备
#include "bsp_esp8266.h"

// 协议文件
#include "onenet.h"

// 硬件驱动
#include "usart.h"
#include "delay.h"

// C库
#include <string.h>
#include <stdio.h>

#define DEVID "1043147880"

#define APIKEY "m=cXh=O8EFYps6fn7rKDcVZZrVU="

extern uint8_t humidityH;	 // 湿度整数部分
extern uint8_t humidityL;	 // 湿度小数部分
extern uint8_t temperatureH; // 温度整数部分
extern uint8_t temperatureL; // 温度小数部分

void OneNet_FillBuf(char *buf)
{

	char text[24];	// 存放临时数据
	char buf1[128]; // 最终的数据

	memset(text, 0, sizeof(text));
	memset(buf1, 0, sizeof(buf1));

	// 要开始构造json格式了

	strcpy(buf1, "{");
	// buf1={

	memset(text, 0, sizeof(text));

	sprintf(text, "\"temp\":%d.%d,", temperatureH, temperatureL);
	strcat(buf1, text);
	// buf1={ \"Temperature\":20.3,

	memset(text, 0, sizeof(text));
	// sprintf(text, "\"Humidity\":%0.2f", sht20_info.humidity);
	sprintf(text, "\"hum\":%d.%d", humidityH, humidityL);
	strcat(buf1, text);
	// buf1={ \"Temperature\":20.3 , \"Humidity\":65.0

	strcat(buf1, "}");
	// buf1={ \"Temperature\":20.3 , \"Humidity\":65.0}

	// 格式化数据
	sprintf(buf, "POST /devices/%s/datapoints?type=3 HTTP/1.1\r\napi-key:%s\r\nHost:api.heclouds.com\r\n"
				 "Content-Length:%d\r\n\r\n",

			DEVID, APIKEY, strlen(buf1));

	strcat(buf, buf1); // 把要发送的数据拼接到buf发送数据的后面

	// 此时buf含有所有需要上传的数据（设备id,apikey，数据长度，采集的数据）
	printf("pub_buf=%s\r\n", buf);
}

//==========================================================
//	函数名称：	OneNet_SendData
//
//	函数功能：	上传数据到平台
//
//	入口参数：	type：发送数据的格式
//
//	返回参数：	无
//
//	说明：
//==========================================================
void OneNet_SendData(void)
{

	char buf[256];

	memset(buf, 0, sizeof(buf)); // 清空数组内容

	OneNet_FillBuf(buf); // 封装数据流（把需要上传的数据封装到里面）

	esp8266.SendData();(unsigned char *)buf, strlen(buf)); // 上传已经封装好的数据
}

//==========================================================
//	函数名称：	OneNet_RevPro
//
//	函数功能：	平台返回数据检测
//
//	入口参数：	dataPtr：平台返回的数据
//
//	返回参数：	无
//
//	说明：
//==========================================================
void OneNet_RevPro(unsigned char *dataPtr)
{

	if (strstr((char *)dataPtr, "CLOSED"))
	{
		printf("TCP CLOSED\r\n");
	}
	else
	{
		// 这里用来检测是否发送成功
		if (strstr((char *)dataPtr, "succ"))
		{
			printf("Tips:	Send OK\r\n");
		}
		else
		{
			// 			UsartPrintf(USART_DEBUG, "Tips:	Send Err\r\n");
			printf("Tips:	Send Err\r\n");
		}
	}

	ESP8266_Clear();
}
