/**
 ************************************************************
 ************************************************************
 ************************************************************
 *	文件名： 	onenet.c
 *
 *	作者： 		张继瑞
 *
 *	日期： 		2017-05-27
 *
 *	版本： 		V1.0
 *
 *	说明： 		OneNET平台应用示例
 *
 *	修改记录：
 ************************************************************
 ************************************************************
 ************************************************************
 **/

// 单片机头文件
#include "stm32f407xx.h"

// 网络设备
#include "esp8266.h"

// 协议文件
#include "onenet.h"
#include "mqttkit.h"

// 硬件驱动
#include "usart.h"
#include "bsp_uart.h"
#include "delay.h"
#include "dht11.h"

// C库
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PROID "570783" // 产品ID

#define AUTH_INFO "2018101045" // 鉴权信息

#define DEVID "1043147880" // 设备ID

// 当正式环境注册码达到16个字符则启用自动创建功能，否则不启用
// 如果要采用自动创建设备的方式，apikey必须为master-key，且正式环境注册码有效
// ONETNET_INFO onenet_info = {"1043147880", "m=cXh=O8EFYps6fn7rKDcVZZrVU=",
// 							"570783", "2018101045",
// 							"",
// 							"183.230.40.39", "6002",
// 							NULL, NULL, 0, 7, NULL, 0,
// 							0, 0, 1, 0, 0, 0, 0, 0, 0};

/**
 * @brief 			获取平台返回的数据
 *
 * @param timeOut 	等待的时间(乘以10ms)
 * @retval 			平台返回的原始数据
 *
 * @note			不同网络设备返回的格式不同，需要去调试
 * 					ESP8266的返回格式:
 * 					"+IPD,x:yyy"
 * 						x: 		数据长度
 * 						yyy:	是数据内容
 */
unsigned char *OneNET_GetIPD(uint16_t timeOut)
{
	char *ptrIPD = NULL;

	do
	{
		if (isRecieveFinished(&esp8266) == Esp8266_RxFinish) // 如果接收完成
		{
			ptrIPD = strstr((char *)esp8266.rxBuffer.queue, "IPD,"); // 搜索“IPD”头
			if (ptrIPD == NULL)										 // 如果没找到，可能是IPD头的延迟，还是需要等待一会，但不会超过设定的时间
			{
				// printf("\"IPD\" not found\r\n");
			}
			else
			{
				ptrIPD = strchr(ptrIPD, ':'); // 找到':'
				if (ptrIPD != NULL)
				{
					ptrIPD++;
					return (unsigned char *)(ptrIPD);
				}
				else
					return NULL;
			}
		}

		delay_ms(5); // 延时等待
	} while (timeOut--);

	return NULL;
}
//==========================================================
//	函数名称：	OneNet_DevLink
//
//	函数功能：	与onenet创建连接
//
//	入口参数：	无
//
//	返回参数：	1-成功	0-失败
//
//	说明：		与onenet平台建立连接
//==========================================================
_Bool OneNET_DevLink(void)
{

	_Bool status = 1;
	unsigned char *dataPtr;
	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0}; // 协议包

	printf("OneNET_DevLink\r\n"
		   "PROID: %s,	AUIF: %s,	DEVID:%s\r\n",
		   PROID, AUTH_INFO, DEVID);

	if (MQTT_PacketConnect(PROID, AUTH_INFO, DEVID, 256, 0, MQTT_QOS_LEVEL0, NULL, NULL, 0, &mqtt_packet) == 0)
	{
		esp8266.SendData(mqtt_packet._data, mqtt_packet._len);

		dataPtr = OneNET_GetIPD(250);

		if (dataPtr != NULL)
		{
			if (MQTT_UnPacketRecv(dataPtr) == MQTT_PKT_CONNACK)
			{
				switch (MQTT_UnPacketConnectAck(dataPtr))
				{
				case 0:
					printf("Tips: Connection succeeded.\r\n");
					status = 0;
					break;

				case 1:
					printf("WARN:	Connection failed: protocol error!\r\n");
					break;
				case 2:
					printf("WARN:	Connection failed: invalid clientid!\r\n");
					break;
				case 3:
					printf("WARN:	Connection failed: server failed!\r\n");
					break;
				case 4:
					printf("WARN:	Connection failed: wrong username or password!\r\n");
					break;
				case 5:
					printf("WARN:	Connection failed: illegal link (such as illegal token)!\r\n");
					break;

				default:
					printf("ERR:	Connection failed: unknown error!\r\n");
					break;
				}
			}
		}
		MQTT_DeleteBuffer(&mqtt_packet);
	}
	else
		printf("WARN:	MQTT_PacketConnect Failed\r\n");

	return status;
}

unsigned char OneNET_FillBuf(char *buf)
{
	char text[32];
	uint8_t humidityH = 0, humidityL = 0;

	humidityH = (uint8_t)(G_dht11.obj.humidity);
	humidityL = (G_dht11.obj.humidity - (uint8_t)(G_dht11.obj.humidity)) * 100;

	memset(text, 0, sizeof(text));

	strcpy(buf, ",;");

	memset(text, 0, sizeof(text));
	sprintf(text, "Humidity,%d.%d;", humidityH, humidityL);
	strcat(buf, text);

	return strlen(buf);
}

//==========================================================
//	函数名称：	OneNET_SendData
//
//	函数功能：	上传数据到平台
//
//	入口参数：	type：发送数据的格式
//				devid：设备ID
//				apikey：设备apikey
//				streamArray：数据流
//				streamArrayNum：数据流个数
//
//	返回参数：	SEND_TYPE_OK-发送成功	SEND_TYPE_DATA-需要重送
//
//	说明：
//==========================================================
void OneNET_SendData(void)
{

	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0}; // 协议包

	char buf[128];

	short body_len = 0, i = 0;

	memset(buf, 0, sizeof(buf));

	body_len = OneNET_FillBuf(buf);
	if (body_len > 0)
	{
		if (MQTT_PacketSaveData(DEVID, body_len, NULL, (uint8)5, &mqtt_packet) == 0)
		{
			while (i < body_len)
			{
				mqtt_packet._data[mqtt_packet._len++] = buf[i++];
			}

			esp8266.SendData(mqtt_packet._data, mqtt_packet._len);

			MQTT_DeleteBuffer(&mqtt_packet);
		}
		else
			printf("WARN:	MQTT_NewBuffer Failed\r\n");
	}
}

//==========================================================
//	函数名称：	OneNET_RevPro
//
//	函数功能：	平台返回数据检测
//
//	入口参数：	dataPtr：平台返回的数据
//
//	返回参数：	无
//
//	说明：
//==========================================================
void OneNET_RevPro(unsigned char *cmd)
{

	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0}; // 协议包

	char *req_payload = NULL;
	char *cmdid_topic = NULL;

	unsigned short req_len = 0;
	unsigned char type = 0;

	short result = 0;

	char *dataPtr = NULL;
	char numBuf[10];
	int num = 0;

	type = MQTT_UnPacketRecv(cmd); // MQTT数据接收类型判断
	switch (type)
	{
	case MQTT_PKT_CMD: // 命令下发

		// 参数1收到的
		result = MQTT_UnPacketCmd(cmd, &cmdid_topic, &req_payload, &req_len); // 解出topic和消息体
		if (result == 0)
		{
			// 打印收到的信息，参数2数据，参数3数据长度
			printf("cmdid: %s, req: %s, req_len: %d\r\n", cmdid_topic, req_payload, req_len);

			if (MQTT_PacketCmdResp(cmdid_topic, req_payload, &mqttPacket) == 0) // 命令回复组包
			{
				printf("Tips:	Send CmdResp\r\n");

				esp8266.SendData(mqttPacket._data, mqttPacket._len); // 回复命令
				MQTT_DeleteBuffer(&mqttPacket);						 // 删包
			}
		}
		break;

	case MQTT_PKT_PUBACK: // 发送Publish消息，平台回复的Ack

		if (MQTT_UnPacketPublishAck(cmd) == 0)
			printf("Tips:	MQTT Publish Send OK\r\n");
		break;

	default:
		result = -1;
		break;
	}

	clearReciveBuffer(&esp8266); // 清空缓存

	if (result == -1)
		return;

	dataPtr = strchr(req_payload, ':'); // 搜索':'

	if (type == MQTT_PKT_CMD || type == MQTT_PKT_PUBLISH)
	{
		MQTT_FreeBuffer(cmdid_topic);
		MQTT_FreeBuffer(req_payload);
	}
}
