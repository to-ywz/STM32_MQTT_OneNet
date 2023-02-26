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

onenet_t G_oneNet;

// 当正式环境注册码达到16个字符则启用自动创建功能，否则不启用
// 如果要采用自动创建设备的方式，apikey必须为master-key，且正式环境注册码有效

onenet_t G_oneNet = {.web_info = {"183.230.40.39", "6002", 7},
					 .user_info = {"1043147880", "m=cXh=O8EFYps6fn7rKDcVZZrVU=",
								   "570783", "2018101045",
								   ""},
					 .file_info = {NULL, NULL, 0},
					 .SR = {0, 0, 1, 0, 0, 0, 0, 0, 0},
					 .send_data = 0,
					 .cmd_ptr = NULL};

void OneNet_Init(char *device_id,
				 char *api_key,
				 char *auth_info,
				 char *product_id,
				 char *ip,
				 char *port)
{
	G_oneNet.send_data = 7;
	G_oneNet.cmd_ptr = NULL;

	G_oneNet.SR.all = 0x0010; // HeartBeat 置位

	G_oneNet.file_info.file_bin = NULL;
	G_oneNet.file_info.file_bin_name = NULL;
	G_oneNet.file_info.file_bin_size = 0;

	memmove(ip, G_oneNet.web_info.ip, strlen(ip));
	memmove(port, G_oneNet.web_info.port, strlen(port));

	memmove(device_id, G_oneNet.user_info.dev_id, strlen(device_id));
	memmove(api_key, G_oneNet.user_info.api_key, strlen(api_key));
	memmove(auth_info, G_oneNet.user_info.auif, strlen(auth_info));
	memmove(product_id, G_oneNet.user_info.pro_id, strlen(product_id));
}

uint8_t onenet_dev_link(const char *devid, const char *proid, const char *auth_info)
{
	/* 协议帧 */
	uint8_t time_out = 200;
	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0};

	printf("OneNET_DevLink\r\n"
		   "PROID: %s,	AUIF: %s,	DEVID:%s\r\n",
		   proid, auth_info, devid);

	if (MQTT_PacketConnect(proid, auth_info, devid, 256, 0, MQTT_QOS_LEVEL0, NULL, NULL, 0, &mqtt_packet))
	{
		esp8266.SendData(mqtt_packet._data, mqtt_packet._len);

		MQTT_DeleteBuffer(&mqtt_packet);

		while (--time_out)
		{
			if (G_oneNet.cmd_ptr != NULL)
			{
				onenet_rev_pro(G_oneNet.cmd_ptr);
				G_oneNet.cmd_ptr = NULL;
				break;
			}
			delay_ms(10);
		}
	}
	else
		printf("WARN:	MQTT_PacketConnect Failed\r\n");
	if (G_oneNet.SR.bit.net_work) // 如果接入成功
	{
		G_oneNet.SR.bit.err_count = 0;
		return 0;
	}

	if (++G_oneNet.SR.bit.err_count >= 5) // 如果超过设定次数后，还未接入平台
	{
		G_oneNet.SR.bit.net_work = 0;
		G_oneNet.SR.bit.err_count = 0;

		G_oneNet.SR.bit.err_check = 1;
		
	}return 1;
}

/**
 * @brief 		心跳检测
 *
 * @retval 		SEND_TYPE_OK-发送成功
 * 				SEND_TYPE_DATA-需要重送
 */
unsigned char onenet_send_heart(void)
{

	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0}; // 协议包

	if (!G_oneNet.SR.bit.net_work) // 如果网络为连接
		return SEND_TYPE_HEART;

	if (MQTT_PacketPing(&mqtt_packet))
		return SEND_TYPE_HEART;

	G_oneNet.SR.bit.heart_beat = 0;

	printf("%s\r\n", mqtt_packet._data);

	// NET_DEVICE_SendData(mqtt_packet._data, mqtt_packet._len);			//向平台上传心跳请求
	esp8266.SendData(mqtt_packet._data, mqtt_packet._len); // 加入链表

	MQTT_DeleteBuffer(&mqtt_packet); // 删包

	return SEND_TYPE_OK;
}

#if 0
/**
 * @brief 				暂时废弃
 * 
 * @param devid 
 * @param apikey 
 * @param streamArray 
 * @param streamArrayCnt 
 * @retval 
 */
unsigned char onenet_send_data(char *devid, char *apikey, DATA_STREAM *streamArray, unsigned short streamArrayCnt)
{
	uint8_t type = 3;
	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0}; // 协议包

	unsigned char status = SEND_TYPE_OK;
	short body_len = 0;

	if (!G_oneNet.SR.bit.net_work)
		return SEND_TYPE_DATA;

	printf("Tips:	OneNET_SendData-MQTT_TYPE%d\r\n", type);

	body_len = DSTREAM_GetDataStream_Body_Measure(type, streamArray, streamArrayCnt, 0); // 获取当前需要发送的数据流的总长度
	if (body_len > 0)
	{
		if (MQTT_PacketSaveData(devid, body_len, NULL, (uint8)type, &mqtt_packet) == 0)
		{
			body_len = DSTREAM_GetDataStream_Body(type, streamArray, streamArrayCnt, mqtt_packet._data, mqtt_packet._size, mqtt_packet._len);

			if (body_len > 0)
			{
				mqtt_packet._len += body_len;
				printf("Send %d Bytes\r\n", mqtt_packet._len);

				printf("%s\r\n", mqtt_packet._data);

				// NET_DEVICE_SendData(mqtt_packet._data, mqtt_packet._len);						//上传数据到平台
				esp8266.SendData(mqtt_packet._data, mqtt_packet._len); // 加入链表
			}
			else
				printf("WARN:	DSTREAM_GetDataStream_Body Failed\r\n");

			MQTT_DeleteBuffer(&mqtt_packet); // 删包
		}
		else
			printf("WARN:	MQTT_NewBuffer Failed\r\n");
	}
	else if (body_len < 0)
		status = SEND_TYPE_OK;
	else
		status = SEND_TYPE_DATA;

	return status;
}
#endif

/**
 * @brief 	发送心跳后的心跳检测
 *
 * @retval 	0-成功
 * 			1-等待
 * @note
 * 			基于调用时基，runCount每隔此函数调用一次的时间自增
 * 			达到设定上限检测心跳标志位是否就绪
 * 			上限时间可以不用太精确
 */
_Bool onenet_check_heart(void)
{

	static unsigned char runCount = 0;

	if (!G_oneNet.SR.bit.net_work)
		return 1;

	if (G_oneNet.SR.bit.heart_beat == 1)
	{
		runCount = 0;
		G_oneNet.SR.bit.err_count = 0;

		return 0;
	}

	if (++runCount >= 40)
	{
		runCount = 0;

		printf("HeartBeat TimeOut: %d\r\n", G_oneNet.SR.bit.err_count);
		G_oneNet.send_data |= SEND_TYPE_HEART; // 发送心跳请求

		if (++G_oneNet.SR.bit.err_count >= 3)
		{
			G_oneNet.SR.bit.err_count = 0;

			G_oneNet.SR.bit.err_check = 1;
		}
	}

	return 1;
}

/**
 * @brief 			发布消息
 *
 * @param topic 	发布的主题
 * @param msg 		消息内容
 * @retval 			SEND_TYPE_OK:		成功
 * 					SEND_TYPE_PUBLISH:	需要重送
 */
unsigned char onenet_publish(const char *topic, const char *msg)
{

	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0}; // 协议包

	if (!G_oneNet.SR.bit.net_work)
		return SEND_TYPE_PUBLISH;

	printf("Publish Topic: %s, Msg: %s\r\n", topic, msg);

	if (MQTT_PacketPublish(MQTT_PUBLISH_ID, topic, msg, strlen(msg), MQTT_QOS_LEVEL2, 0, 1, &mqtt_packet) == 0)
	{
		printf("%s\r\n", mqtt_packet._data);

		// NET_DEVICE_SendData(mqtt_packet._data, mqtt_packet._len);				//向平台发送订阅请求
		esp8266.SendData(mqtt_packet._data, mqtt_packet._len); // 加入链表

		MQTT_DeleteBuffer(&mqtt_packet); // 删包
	}

	return SEND_TYPE_OK;
}

#if 0
void OneNET_CmdHandle(void)
{

	unsigned char *dataPtr = NULL, *ipdPtr = NULL; // 数据指针

	dataPtr = NET_DEVICE_Read(); // 等待数据

	if (dataPtr != NULL) // 数据有效
	{
		ipdPtr = NET_DEVICE_GetIPD(dataPtr); // 检查是否是平台数据
		if (ipdPtr != NULL)
		{
			net_device_info.send_count = 0;

			if (onenet_info.connect_ip)
				onenet_info.cmd_ptr = ipdPtr; // 集中处理

			net_device_info.cmd_ipd = (char *)ipdPtr;
		}
		else
		{
			if (strstr((char *)dataPtr, "SEND OK") != NULL)
			{
				net_device_info.send_count = 0;
			}
			else if (strstr((char *)dataPtr, "CLOSE") != NULL && onenet_info.net_work)
			{
				UsartPrintf(USART_DEBUG, "WARN:	连接断开，准备重连\r\n");

				onenet_info.err_check = 1;
			}
			else
				NET_DEVICE_CmdHandle((char *)dataPtr);
		}
	}
}
#endif
/**
 * @brief 		平台返回数据检测
 *
 * @param cmd 	平台返回的数据
 */
void onenet_rev_pro(unsigned char *cmd)
{

	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0}; // 协议包

	char *req_payload = NULL;
	char *cmdid_topic = NULL;

	unsigned short topic_len = 0;
	unsigned short req_len = 0;

	unsigned char qos = 0;
	static unsigned short pkt_id = 0;

	printf("%s\r\n", cmd);

	switch (MQTT_UnPacketRecv(cmd))
	{
	case MQTT_PKT_CONNACK:

		switch (MQTT_UnPacketConnectAck(cmd))
		{
		case 0:
			printf("Tips:	连接成功\r\n");
			G_oneNet.SR.bit.net_work = 1;
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

		break;

	case MQTT_PKT_PINGRESP:

		printf("Tips:	HeartBeat OK\r\n");
		G_oneNet.SR.bit.heart_beat = 1;

		break;

	case MQTT_PKT_CMD: // 命令下发

		if (MQTT_UnPacketCmd(cmd, &cmdid_topic, &req_payload, &req_len) == 0) // 解出topic和消息体
		{
			printf("cmdid: %s, req: %s, req_len: %d\r\n", cmdid_topic, req_payload, req_len);

			// 执行命令回调------------------------------------------------------------
			// CALLBACK_Execute(req_payload);

			if (MQTT_PacketCmdResp(cmdid_topic, req_payload, &mqtt_packet) == 0) // 命令回复组包
			{
				printf("Tips:	Send CmdResp\r\n");

				// 解析数据
				printf("%s", mqtt_packet._data);

				// NET_DEVICE_SendData(mqtt_packet._data, mqtt_packet._len);			//回复命令
				// NET_DEVICE_AddDataSendList(mqtt_packet._data, mqtt_packet._len, 1); // 加入链表
				MQTT_DeleteBuffer(&mqtt_packet); // 删包
			}

			MQTT_FreeBuffer(cmdid_topic);
			MQTT_FreeBuffer(req_payload);
			G_oneNet.send_data |= SEND_TYPE_DATA;
		}

		break;

	case MQTT_PKT_PUBLISH: // 接收的Publish消息

		if (MQTT_UnPacketPublish(cmd, &cmdid_topic, &topic_len, &req_payload, &req_len, &qos, &pkt_id) == 0)
		{
			printf("topic: %s, topic_len: %d, payload: %s, payload_len: %d\r\n",
				   cmdid_topic, topic_len, req_payload, req_len);

			// 执行命令回调------------------------------------------------------------
			// CALLBACK_Execute(req_payload);

			switch (qos)
			{
			case 1: // 收到publish的qos为1，设备需要回复Ack

				if (MQTT_PacketPublishAck(pkt_id, &mqtt_packet) == 0)
				{
					printf("Tips:	Send PublishAck\r\n");

					// 解析数据
					printf("%s\r\n", mqtt_packet._data);

					// NET_DEVICE_SendData(mqtt_packet._data, mqtt_packet._len);
					// NET_DEVICE_AddDataSendList(mqtt_packet._data, mqtt_packet._len, 1); // 加入链表
					MQTT_DeleteBuffer(&mqtt_packet);
				}

				break;

			case 2: // 收到publish的qos为2，设备先回复Rec
					// 平台回复Rel，设备再回复Comp
				if (MQTT_PacketPublishRec(pkt_id, &mqtt_packet) == 0)
				{
					printf("Tips:	Send PublishRec\r\n");

					// 解析数据
					printf("%s\r\n", mqtt_packet._data);

					// NET_DEVICE_SendData(mqtt_packet._data, mqtt_packet._len);
					// NET_DEVICE_AddDataSendList(mqtt_packet._data, mqtt_packet._len, 1); // 加入链表
					MQTT_DeleteBuffer(&mqtt_packet);
				}

				break;

			default:
				break;
			}

			MQTT_FreeBuffer(cmdid_topic);
			MQTT_FreeBuffer(req_payload);
			G_oneNet.send_data |= SEND_TYPE_DATA;
		}

		break;

	case MQTT_PKT_PUBACK: // 发送Publish消息，平台回复的Ack

		if (MQTT_UnPacketPublishAck(cmd) == 0)
		{
			printf("Tips:	MQTT Publish Send OK\r\n");

#if (LBS_WIFI_EN == 1 || LBS_EN == 1)
			if (
#if (LBS_WIFI_EN == 1)
				lbs_wifi_info.lbs_wifi_ok == 1 &&
#elif (LBS_EN == 1)
				lbs_info.lbs_ok == 1 &&
#endif
				!onenet_info.lbs && onenet_info.lbs_count < 4) // 如果获取到了基站信息 且 未获取到位置坐标 且 获取次数小于一定值
			{
				onenet_info.net_work = 0; // 则重新获取一下位置信息
				get_location_flag = 1;
			}
#endif
		}

		break;

	case MQTT_PKT_PUBREC: // 发送Publish消息，平台回复的Rec，设备需回复Rel消息

		if (MQTT_UnPacketPublishRec(cmd) == 0)
		{
			printf("Tips:	Rev PublishRec\r\n");
			if (MQTT_PacketPublishRel(MQTT_PUBLISH_ID, &mqtt_packet) == 0)
			{
				printf("Tips:	Send PublishRel\r\n");

				printf("%s\r\n", mqtt_packet._data);

				// NET_DEVICE_SendData(mqtt_packet._data, mqtt_packet._len);
				esp8266.SendData(mqtt_packet._data, mqtt_packet._len); // 直接发送
				MQTT_DeleteBuffer(&mqtt_packet);
			}
		}

		break;

	case MQTT_PKT_PUBREL: // 收到Publish消息，设备回复Rec后，平台回复的Rel，设备需再回复Comp

		if (MQTT_UnPacketPublishRel(cmd, pkt_id) == 0)
		{
			printf("Tips:	Rev PublishRel\r\n");
			if (MQTT_PacketPublishComp(pkt_id, &mqtt_packet) == 0)
			{
				printf("Tips:	Send PublishComp\r\n");

				printf("%s\r\n", mqtt_packet._data);

				// NET_DEVICE_SendData(mqtt_packet._data, mqtt_packet._len);
				esp8266.SendData(mqtt_packet._data, mqtt_packet._len); // 加入链表
				MQTT_DeleteBuffer(&mqtt_packet);
			}
		}

		break;

	case MQTT_PKT_PUBCOMP: // 发送Publish消息，平台返回Rec，设备回复Rel，平台再返回的Comp

		if (MQTT_UnPacketPublishComp(cmd) == 0)
		{
			printf("Tips:	Rev PublishComp\r\n");
		}

		break;

	case MQTT_PKT_SUBACK: // 发送Subscribe消息的Ack

		if (MQTT_UnPacketSubscribe(cmd) == 0)
			printf("Tips:	MQTT Subscribe OK\r\n");
		else
			printf("Tips:	MQTT Subscribe Err\r\n");

		break;

	case MQTT_PKT_UNSUBACK: // 发送UnSubscribe消息的Ack

		if (MQTT_UnPacketUnSubscribe(cmd) == 0)
			printf("Tips:	MQTT UnSubscribe OK\r\n");
		else
			printf("Tips:	MQTT UnSubscribe Err\r\n");

		break;

	default:

		break;
	}
}

/**
 * @brief 			获取平台返回的数据
 *
 * @param timeOut 	等待的时间(乘以10ms)
 * @retval 			平台返回的原始数据
 *
 * @n ote			不同网络设备返回的格式不同，需要去调试
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

/**
 * @brief 	检测 OneNET_SendData 后云端是否应答
 *
 * @retval 	0: 应答
 * 			1: 未应答
 */
static uint8_t get_rev_result(void)
{

	/* 检测是否接受完毕 */
	uint16_t rev_cnt = 0;
	while (Esp8266_RxFinish != isRecieveFinished(&esp8266))
	{
		delay_ms(10);
		if (rev_cnt++ > 100)
		{
			/* 输出错误数据*/
			/*			printf("%d%d%d%d\r\n", esp8266.rxBuffer.queue[0],
							   esp8266.rxBuffer.queue[1],
							   esp8266.rxBuffer.queue[2],
							   esp8266.rxBuffer.queue[3]);
							   */
			return 1;
		}
	}

	/* 校验数据是否发送成功 */
	if (esp8266.rxBuffer.queue[0] == '@')
	{
		return 0;
	}

	return 1;
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
uint8_t OneNET_SendData(void)
{
	static uint8_t err_cnt = 0;

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

			clearReciveBuffer(&esp8266);
			esp8266.SendData(mqtt_packet._data, mqtt_packet._len);

			MQTT_DeleteBuffer(&mqtt_packet);
		}
		else
			printf("WARN:	MQTT_NewBuffer Failed\r\n");
	}

	if (!get_rev_result())
	{
		err_cnt = 0;
		return 0;
	}

	/* 15ms后重新连接 */
	err_cnt++;
	if (err_cnt > 3)
	{
		err_cnt = 0;
		while (!OneNET_DevLink())
		{
			err_cnt++;
			// 重启, 超过30次复位8266
			if (err_cnt > 15)
			{
				err_cnt = 0;
				clearReciveBuffer(&esp8266);
				esp8266Init();
				OneNET_DevLink();
				delay_ms(1000);
			}
		}
		printf("ReConnect Server.\r\n");
	}

	return 1;
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

// void NET_Task(void)
// {
// 	static uint32_t err_cnt = 0; // 错误计数

// 	if(MQTT_PacketPing)
// }

//==================================================End=================================================
