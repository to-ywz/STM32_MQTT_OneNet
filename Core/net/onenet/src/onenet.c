/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	onenet.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2017-05-27
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		OneNETƽ̨Ӧ��ʾ��
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��ͷ�ļ�
#include "stm32f407xx.h"

//�����豸
#include "esp8266.h"

//Э���ļ�
#include "onenet.h"
#include "fault.h"
#include "mqttkit.h"

//Э�����
#include "protocol_parser.h"

//����ص�
#include "cmd_callback.h"

//Ӳ������
#include "usart.h"
#include "delay.h"

//C��
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


//����ʽ����ע����ﵽ16���ַ��������Զ��������ܣ���������
//���Ҫ�����Զ������豸�ķ�ʽ��apikey����Ϊmaster-key������ʽ����ע������Ч
ONETNET_INFO onenet_info = {"1043147880", "m=cXh=O8EFYps6fn7rKDcVZZrVU=",
							"570783", "2018101045",
							"",
							"183.230.40.39", "6002",
							NULL, NULL, 0, 7, NULL, 0,
							0, 0, 1, 0, 0, 0, 0, 0, 0};


static _Bool get_location_flag = 0;


//==========================================================
//	�������ƣ�	OneNET_RepetitionCreateFlag
//
//	�������ܣ�	�����ظ�ע���豸
//
//	��ڲ�����	apikey��������masterkey
//
//	���ز�����	0-�ɹ�		1-ʧ��
//
//	˵����		�����ظ�ע�ᣬ�����һ�δ����ɹ�֮���ٴδ�����ʧ��
//==========================================================
_Bool OneNET_RepetitionCreateFlag(const char *apikey)
{

	_Bool result = 1;
	char send_buf[136];
	unsigned char time_out = 200;
	
	if(!net_device_info.net_work)
		return result;
	
	NET_DEVICE_Close();
	
	if(NET_DEVICE_Connect("TCP", "183.230.40.33", "80") == 0)
	{
		snprintf(send_buf, sizeof(send_buf), "PUT /register_attr HTTP/1.1\r\napi-key:%s\r\nHost:api.heclouds.com\r\n"
						"Content-Length:19\r\n\r\n"
						"{\"allow_dup\": true}", apikey);
		
		if(!NET_DEVICE_SendData((unsigned char *)send_buf, strlen(send_buf)))
		{
			net_device_info.cmd_ipd = NULL;
			
			while(--time_out)
			{
				if(net_device_info.cmd_ipd != NULL)
					break;
				
				DelayXms(10);
			}
			
			if(time_out)
			{
				if(strstr(net_device_info.cmd_ipd, "succ"))
				{
					UsartPrintf(USART_DEBUG, "Tips:	OneNET_RepetitionCreateFlag Ok\r\n");
					result = 0;
				}
				else if(strstr(net_device_info.cmd_ipd, "auth failed"))
				{
					UsartPrintf(USART_DEBUG, "WARN:	��ǰʹ�õĲ���masterkey �� apikey����\r\n");
				}
				else
					UsartPrintf(USART_DEBUG, "Tips:	OneNET_RepetitionCreateFlag Err\r\n");
			}
			else
				UsartPrintf(USART_DEBUG, "Tips:	OneNET_RepetitionCreateFlag Time Out\r\n");
		}
		
		NET_DEVICE_Close();
	}
	
	return result;

}

//==========================================================
//	�������ƣ�	OneNET_CreateDevice
//
//	�������ܣ�	�ڲ�Ʒ�д���һ���豸
//
//	��ڲ�����	reg_code����ʽ����ע����
//				dev_name���豸��
//				auth_info����Ȩ��Ϣ
//				devid�����淵�ص�devid
//				apikey�����淵�ص�apikey
//
//	���ز�����	0-�ɹ�		1-ʧ��
//
//	˵����		
//==========================================================
_Bool OneNET_CreateDevice(const char *reg_code, const char *dev_name, const char *auth_info, char *devid, char *apikey)
{

	_Bool result = 1;
	unsigned short send_len = 20 + strlen(dev_name) + strlen(auth_info);
	unsigned char time_out = 200;
	char *send_ptr = NULL, *data_ptr = NULL;
	
	if(!net_device_info.net_work)
		return result;
	
	send_ptr = NET_MallocBuffer(send_len + 140);
	if(send_ptr == NULL)
		return result;
	
	NET_DEVICE_Close();
	
	if(NET_DEVICE_Connect("TCP", "183.230.40.33", "80") == 0)
	{
		snprintf(send_ptr, 140 + send_len, "POST /register_de?register_code=%s HTTP/1.1\r\n"
						"Host: api.heclouds.com\r\n"
						"Content-Length:%d\r\n\r\n"
						"{\"sn\":\"%s\",\"title\":\"%s\"}",
		
						reg_code, send_len, auth_info, dev_name);
		
		if(!NET_DEVICE_SendData((unsigned char *)send_ptr, strlen(send_ptr)))
		{
			//{"device_id":"12345678","key":"gZrujZgtH3ivgNk33Qy3HgdTErg="}
			
			net_device_info.cmd_ipd = NULL;
			
			while(--time_out)
			{
				if(net_device_info.cmd_ipd != NULL)
					break;
				
				DelayXms(10);
			}
			
			if(time_out)
			{
				data_ptr = strstr(net_device_info.cmd_ipd, "device_id");
				
				if(strstr(net_device_info.cmd_ipd, "auth failed"))
				{
					UsartPrintf(USART_DEBUG, "WARN:	��ʽ����ע�������\r\n");
				}
			}
			
			if(data_ptr)
			{
				if(sscanf(data_ptr, "device_id\":\"%[^\"]\",\"key\":\"%[^\"]\"", devid, apikey) == 2)
				{
					UsartPrintf(USART_DEBUG, "create device: %s, %s\r\n", devid, apikey);
					result = 0;
				}
			}
		}
		
		NET_DEVICE_Close();
	}
	
	NET_FreeBuffer(send_ptr);
	
	return result;

}

//==========================================================
//	�������ƣ�	OneNET_GetLinkIP
//
//	�������ܣ�	��ȡʹ��Э��ĵ�¼IP��PORT
//
//	��ڲ�����	protocol��Э���
//				ip�����淵��IP�Ļ�����
//				port�����淵��port�Ļ�����
//
//	���ز�����	0-�ɹ�		1-ʧ��
//
//	˵����		1-edp	2-nwx	3-jtext		4-Hiscmd
//				5-jt808			6-modbus	7-mqtt
//				8-gr20			9-reg		10-HTTP(�Զ���)
//				��ȡIP����֧��HTTPЭ�飬�����Զ���һ����־
//==========================================================
_Bool OneNET_GetLinkIP(unsigned char protocol, char *ip, char *port)
{
	
	_Bool result = 1;
	char *data_ptr = NULL;
	char send_buf[128];
	unsigned char time_out = 200;
	
	if(!net_device_info.net_work)
		return result;
	
	if(onenet_info.get_ip)
		return !result;
	
	if(protocol == 10)													//�����HTTPЭ��
	{
		strcpy(ip, "183.230.40.33");
		strcpy(port, "80");
		
		onenet_info.get_ip = 1;
		
		return !result;
	}
	
	NET_DEVICE_Close();
	
	if(NET_DEVICE_Connect("TCP", "183.230.40.33", "80") == 0)
	{
		memset(send_buf, 0, sizeof(send_buf));
		snprintf(send_buf, sizeof(send_buf), "GET http://api.heclouds.com/s?t=%d HTTP/1.1\r\n"
												"api-key:=sUT=jsLGXkQcUz3Z9EaiNQ80U0=\r\n"
												"Host:api.heclouds.com\r\n\r\n",
												protocol);
		
		net_device_info.cmd_ipd = NULL;
		
		NET_DEVICE_SendData((unsigned char *)send_buf, strlen(send_buf));
		
		while(--time_out)
		{
			if(net_device_info.cmd_ipd != NULL)
				break;
			
			DelayXms(10);
		}
		
		if(time_out)
		{
			data_ptr = strstr(net_device_info.cmd_ipd, "no-cache");				//�ҵ����Ĺؼ���
		}
		
		if(data_ptr != NULL)
		{
			if(strstr(data_ptr, "unsupportted") != NULL)						//��֧�ֵ�Э������
			{
				UsartPrintf(USART_DEBUG, "��֧�ָ�Э������\r\n");
				
				onenet_info.get_ip = 1;
			}
			else if(strstr(data_ptr, "can't find a available") != NULL)			//��֧�ֵ�Э������
			{
				UsartPrintf(USART_DEBUG, "can't find a available IP\r\n");
				
				onenet_info.get_ip = 1;
			}
			else
			{
				if(sscanf(data_ptr, "no-cache\r\n%[^:]:%s", ip, port) == 2)
				{
					onenet_info.get_ip = 1;
					result = 0;
					
					UsartPrintf(USART_DEBUG, "Get ip: %s, port: %s\r\n", ip, port);
				}
			}
		}
		
		NET_DEVICE_Close();
	}
	
	return result;

}

//==========================================================
//	�������ƣ�	OneNET_GetLocation
//
//	�������ܣ�	��ȡLBS��λ����
//
//	��ڲ�����	device_id���豸ID
//				api_key��apikey
//				lon������lon����
//				lat������lat����
//
//	���ز�����	0-�ɹ�	����������
//
//	˵����		
//==========================================================
unsigned char OneNET_GetLocation(const char *device_id, const char *api_key, char *lon, char *lat)
{
	
	unsigned char result = 255;

#if(LBS_EN == 1 || LBS_WIFI_EN == 1)
	char *data_ptr = NULL;
	char send_buf[128];
	unsigned char time_out = 200;
	
	if(!get_location_flag)
		return 255;
	
	if(!net_device_info.net_work)
		return result;
	
	if(strlen(device_id) < 6 || strlen(api_key) != 28)
		return result;
	
	if(lon == NULL || lat == NULL)
		return result;
	
	onenet_info.connect_ip = 0;
	onenet_info.net_work = 0;
	
	if(NET_DEVICE_Connect("TCP", "183.230.40.33", "80") == 0)
	{
		memset(send_buf, 0, sizeof(send_buf));
		snprintf(send_buf, sizeof(send_buf), "GET /devices/%s/lbs/%s HTTP/1.1\r\n"
												"api-key:%s\r\n"
												"Host:api.heclouds.com\r\n\r\n",
												device_id,
#if(LBS_WIFI_EN == 1)
												"latestWifiLocation"
#elif(LBS_EN == 1)
												"latestLocation"
#endif
												,api_key);
		
		net_device_info.cmd_ipd = NULL;
		
		NET_DEVICE_SendData((unsigned char *)send_buf, strlen(send_buf));
		
		while(--time_out)
		{
			if(net_device_info.cmd_ipd != NULL)
				break;
			
			DelayXms(15);
		}
		
		//{"errno":0,"data":{"at":"2018-12-07 15:28:05.348","accuracy":696,"lon":106.xxx,"lat":29.xxx},"error":"succ"}
		
		if(time_out)
		{
			data_ptr = strstr(net_device_info.cmd_ipd, "\"lon\":");
		}
		
		if(data_ptr != NULL)
		{
			if(sscanf(data_ptr, "\"lon\":%[^,],\"lat\":%[^}]},\"error\":\"succ\"}", lon, lat) == 2)
			{
				UsartPrintf(USART_DEBUG, "lon:%s, lat: %s\r\n", lon, lat);
				
				onenet_info.lbs = 1;
				
				result = 0;
			}
		}
		else
		{
			data_ptr = strstr(net_device_info.cmd_ipd, "\"error\":");
			if(data_ptr)
				UsartPrintf(USART_DEBUG, "������Ϣ:%s\r\n", data_ptr);
			
			if(++onenet_info.lbs_count >= 4)
			{
				onenet_info.lbs_count = 0;
				
				UsartPrintf(USART_DEBUG, "WARN:	�����Ʒ�Ƿ�ͨλ������!!!\r\n");
			}
		}
	}
#endif
	
	return result;

}

//==========================================================
//	�������ƣ�	OneNET_ConnectIP
//
//	�������ܣ�	����IP
//
//	��ڲ�����	ip��IP��ַ����ָ��
//				port���˿ڻ���ָ��
//
//	���ز�����	���س�ʼ�����
//
//	˵����		0-�ɹ�		1-ʧ��
//==========================================================
_Bool OneNET_ConnectIP(char *ip, char *port)
{

	_Bool result = 1;
	
	if(!net_device_info.net_work)									//����ģ��߱�������������
		return result;
	
	if(onenet_info.connect_ip)										//�Ѿ�������IP
		return !result;
	
	if(!NET_DEVICE_Connect("TCP", ip, port))
	{
		result = 0;
		net_fault_info.net_fault_count = 0;
		onenet_info.connect_ip = 1;
	}
	else
	{
		DelayXms(500);
		
		if(++onenet_info.err_count >= 5)
		{
			onenet_info.err_count = 0;
			UsartPrintf(USART_DEBUG, "����IP��ַ��PORT�Ƿ���ȷ\r\n");
		}
	}
	
	return result;

}

//==========================================================
//	�������ƣ�	OneNET_DevLink
//
//	�������ܣ�	��onenet��������
//
//	��ڲ�����	devid�������豸��devid
//				proid����ƷID
//				auth_key�������豸��masterKey��apiKey���豸��Ȩ��Ϣ
//
//	���ز�����	��
//
//	˵����		��onenetƽ̨�������ӣ��ɹ������oneNetInfo.netWork����״̬��־
//==========================================================
void OneNET_DevLink(const char* devid, const char *proid, const char* auth_info)
{
	
	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0};					//Э���
	
	unsigned char time_out = 200;
	
	UsartPrintf(USART_DEBUG, "OneNET_DevLink\r\n"
							"PROID: %s,	AUIF: %s,	DEVID:%s\r\n"
                        , proid, auth_info, devid);
	
	if(MQTT_PacketConnect(proid, auth_info, devid, 256, 0, MQTT_QOS_LEVEL0, NULL, NULL, 0, &mqtt_packet) == 0)
	{
		Protocol_Parser_Print(mqtt_packet._data);
		
		NET_DEVICE_SendData(mqtt_packet._data, mqtt_packet._len);			//�ϴ�ƽ̨
		//NET_DEVICE_AddDataSendList(mqtt_packet._data, mqtt_packet._len, 1);
		
		MQTT_DeleteBuffer(&mqtt_packet);									//ɾ��
		
		while(--time_out)
		{
			if(onenet_info.cmd_ptr != NULL)
			{
				OneNET_RevPro(onenet_info.cmd_ptr);
				
				onenet_info.cmd_ptr = NULL;
				
				break;
			}
			
			DelayXms(10);
		}
	}
	else
		UsartPrintf(USART_DEBUG, "WARN:	MQTT_PacketConnect Failed\r\n");
	
	if(onenet_info.net_work)											//�������ɹ�
	{
		onenet_info.err_count = 0;
	}
	else
	{
		if(++onenet_info.err_count >= 5)								//��������趨�����󣬻�δ����ƽ̨
		{
			onenet_info.net_work = 0;
			onenet_info.err_count = 0;
			
			onenet_info.err_check = 1;
		}
	}
	
}

//==========================================================
//	�������ƣ�	OneNET_DisConnect
//
//	�������ܣ�	��ƽ̨�Ͽ�����
//
//	��ڲ�����	��
//
//	���ز�����	0-�ɹ�		1-ʧ��
//
//	˵����		
//==========================================================
_Bool OneNET_DisConnect(void)
{

	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0};						//Э���

	if(!onenet_info.net_work)
		return 1;
	
	if(MQTT_PacketDisConnect(&mqtt_packet) == 0)
	{
		Protocol_Parser_Print(mqtt_packet._data);
		
		//NET_DEVICE_SendData(mqtt_packet._data, mqtt_packet._len);				//��ƽ̨���Ͷ�������
		NET_DEVICE_AddDataSendList(mqtt_packet._data, mqtt_packet._len, 1);		//��������
		
		MQTT_DeleteBuffer(&mqtt_packet);										//ɾ��
	}
	
	return 0;

}

//==========================================================
//	�������ƣ�	OneNET_SendData
//
//	�������ܣ�	�ϴ����ݵ�ƽ̨
//
//	��ڲ�����	type���������ݵĸ�ʽ
//				devid���豸ID
//				apikey���豸apikey
//				streamArray��������
//				streamArrayNum������������
//
//	���ز�����	SEND_TYPE_OK-���ͳɹ�	SEND_TYPE_DATA-��Ҫ����
//
//	˵����		
//==========================================================
unsigned char OneNET_SendData(FORMAT_TYPE type, char *devid, char *apikey, DATA_STREAM *streamArray, unsigned short streamArrayCnt)
{
	
	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0};										//Э���
	
	unsigned char status = SEND_TYPE_OK;
	short body_len = 0;
	
	if(!onenet_info.net_work)
		return SEND_TYPE_DATA;
	
	UsartPrintf(USART_DEBUG, "Tips:	OneNET_SendData-MQTT_TYPE%d\r\n", type);
	
	body_len = DSTREAM_GetDataStream_Body_Measure(type, streamArray, streamArrayCnt, 0);		//��ȡ��ǰ��Ҫ���͵����������ܳ���
	if(body_len > 0)
	{
		if(MQTT_PacketSaveData(devid, body_len, NULL, (uint8)type, &mqtt_packet) == 0)
		{
			body_len = DSTREAM_GetDataStream_Body(type, streamArray, streamArrayCnt, mqtt_packet._data, mqtt_packet._size, mqtt_packet._len);
			
			if(body_len > 0)
			{
				mqtt_packet._len += body_len;
				UsartPrintf(USART_DEBUG, "Send %d Bytes\r\n", mqtt_packet._len);
				
				Protocol_Parser_Print(mqtt_packet._data);
				
				//NET_DEVICE_SendData(mqtt_packet._data, mqtt_packet._len);						//�ϴ����ݵ�ƽ̨
				NET_DEVICE_AddDataSendList(mqtt_packet._data, mqtt_packet._len, 1);				//��������
			}
			else
				UsartPrintf(USART_DEBUG, "WARN:	DSTREAM_GetDataStream_Body Failed\r\n");
				
			MQTT_DeleteBuffer(&mqtt_packet);													//ɾ��
		}
		else
			UsartPrintf(USART_DEBUG, "WARN:	MQTT_NewBuffer Failed\r\n");
	}
	else if(body_len < 0)
		status = SEND_TYPE_OK;
	else
		status = SEND_TYPE_DATA;
	
	net_fault_info.net_fault_level_r = NET_FAULT_LEVEL_0;										//����֮��������
	
	return status;
	
}

//==========================================================
//	�������ƣ�	OneNET_Send_BinFile
//
//	�������ܣ�	�ϴ��������ļ���ƽ̨
//
//	��ڲ�����	name����������
//				file���ļ�
//				file_size���ļ�����
//
//	���ز�����	SEND_TYPE_OK-���ͳɹ�	SEND_TYPE_BINFILE-��Ҫ����
//
//	˵����		
//==========================================================
#define PKT_SIZE 1024
unsigned char OneNET_Send_BinFile(char *name, const unsigned char *file, unsigned int file_size)
{
	
	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0};					//Э���
	
	unsigned char status = SEND_TYPE_BINFILE;

	char *type_bin_head = NULL;												//ͼƬ����ͷ
	unsigned char *file_t = (unsigned char *)file;
	
	if(name == NULL || file == NULL || file_size == 0)
		return status;
	
	if(!onenet_info.net_work)
		return SEND_TYPE_BINFILE;
	
	type_bin_head = (char *)NET_MallocBuffer(13 + strlen(name));
	if(type_bin_head == NULL)
		return status;
	
	sprintf(type_bin_head, "{\"ds_id\":\"%s\"}", name);
	
	if(MQTT_PacketSaveBinData(name, file_size, &mqtt_packet) == 0)
	{
		UsartPrintf(USART_DEBUG, "Send %d Bytes\r\n", mqtt_packet._len);
		
		Protocol_Parser_Print(mqtt_packet._data);
		
		NET_DEVICE_SendData(mqtt_packet._data, mqtt_packet._len);			//�ϴ����ݵ�ƽ̨
		//NET_DEVICE_AddDataSendList(edp_packet._data, edp_packet._len, 1);	//��������
		
		MQTT_DeleteBuffer(&mqtt_packet);									//ɾ��
		
		UsartPrintf(USART_DEBUG, "Image Len = %d\r\n", file_size);
		
		while(file_size > 0)
		{
			DelayXms(net_device_info.send_time);
			UsartPrintf(USART_DEBUG, "Image Reamin %d Bytes\r\n", file_size);
			
			if(file_size >= PKT_SIZE)
			{
				if(!NET_DEVICE_SendData(file_t, PKT_SIZE))					//���ڷ��ͷ�Ƭ
				{
					file_t += PKT_SIZE;
					file_size -= PKT_SIZE;
				}
			}
			else
			{
				if(!NET_DEVICE_SendData(file_t, (unsigned short)file_size))	//���ڷ������һ����Ƭ
					file_size = 0;
			}
		}
		
		UsartPrintf(USART_DEBUG, "Tips:	Image Send Ok\r\n");
		
		status = SEND_TYPE_OK;
	}
	else
		UsartPrintf(USART_DEBUG, "MQTT_PacketSaveData Failed\r\n");
	
	NET_FreeBuffer(type_bin_head);
	
	return status;

}

//==========================================================
//	�������ƣ�	OneNET_SendData_Heart
//
//	�������ܣ�	�������
//
//	��ڲ�����	��
//
//	���ز�����	SEND_TYPE_OK-���ͳɹ�	SEND_TYPE_DATA-��Ҫ����
//
//	˵����		
//==========================================================
unsigned char OneNET_SendData_Heart(void)
{
	
	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0};				//Э���
	
	if(!onenet_info.net_work)											//�������Ϊ����
		return SEND_TYPE_HEART;
	
	if(MQTT_PacketPing(&mqtt_packet))
		return SEND_TYPE_HEART;
	
	onenet_info.heart_beat = 0;
	
	Protocol_Parser_Print(mqtt_packet._data);
	
	//NET_DEVICE_SendData(mqtt_packet._data, mqtt_packet._len);			//��ƽ̨�ϴ���������
	NET_DEVICE_AddDataSendList(mqtt_packet._data, mqtt_packet._len, 0);	//��������
	
	MQTT_DeleteBuffer(&mqtt_packet);									//ɾ��
	
	return SEND_TYPE_OK;

}

//==========================================================
//	�������ƣ�	OneNET_Check_Heart
//
//	�������ܣ�	������������������
//
//	��ڲ�����	��
//
//	���ز�����	0-�ɹ�	1-�ȴ�
//
//	˵����		���ڵ���ʱ����runCountÿ���˺�������һ�ε�ʱ������
//				�ﵽ�趨���޼��������־λ�Ƿ����
//				����ʱ����Բ���̫��ȷ
//==========================================================
_Bool OneNET_Check_Heart(void)
{
	
	static unsigned char runCount = 0;
	
	if(!onenet_info.net_work)
		return 1;

	if(onenet_info.heart_beat == 1)
	{
		runCount = 0;
		onenet_info.err_count = 0;
		
		return 0;
	}
	
	if(++runCount >= 40)
	{
		runCount = 0;
		
		UsartPrintf(USART_DEBUG, "HeartBeat TimeOut: %d\r\n", onenet_info.err_count);
		onenet_info.send_data |= SEND_TYPE_HEART;		//������������
		
		if(++onenet_info.err_count >= 3)
		{
			onenet_info.err_count = 0;
			
			onenet_info.err_check = 1;
		}
	}
	
	return 1;

}

//==========================================================
//	�������ƣ�	OneNET_Publish
//
//	�������ܣ�	������Ϣ
//
//	��ڲ�����	topic������������
//				msg����Ϣ����
//
//	���ز�����	SEND_TYPE_OK-�ɹ�	SEND_TYPE_PUBLISH-��Ҫ����
//
//	˵����		
//==========================================================
unsigned char OneNET_Publish(const char *topic, const char *msg)
{

	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0};						//Э���

	if(!onenet_info.net_work)
		return SEND_TYPE_PUBLISH;
	
	UsartPrintf(USART_DEBUG, "Publish Topic: %s, Msg: %s\r\n", topic, msg);
	
	if(MQTT_PacketPublish(MQTT_PUBLISH_ID, topic, msg, strlen(msg), MQTT_QOS_LEVEL2, 0, 1, &mqtt_packet) == 0)
	{
		Protocol_Parser_Print(mqtt_packet._data);
		
		//NET_DEVICE_SendData(mqtt_packet._data, mqtt_packet._len);				//��ƽ̨���Ͷ�������
		NET_DEVICE_AddDataSendList(mqtt_packet._data, mqtt_packet._len, 1);		//��������
		
		MQTT_DeleteBuffer(&mqtt_packet);										//ɾ��
	}
	
	return SEND_TYPE_OK;

}

//==========================================================
//	�������ƣ�	OneNET_Subscribe
//
//	�������ܣ�	����
//
//	��ڲ�����	topics�����ĵ�topic
//				topic_cnt��topic����
//
//	���ز�����	SEND_TYPE_OK-�ɹ�	SEND_TYPE_SUBSCRIBE-��Ҫ�ط�
//
//	˵����		
//==========================================================
unsigned char OneNET_Subscribe(const char *topics[], unsigned char topic_cnt)
{
	
	unsigned char i = 0;
	
	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0};						//Э���

	if(!onenet_info.net_work)
		return SEND_TYPE_SUBSCRIBE;
	
	for(; i < topic_cnt; i++)
		UsartPrintf(USART_DEBUG, "Subscribe Topic: %s\r\n", topics[i]);
	
	if(MQTT_PacketSubscribe(MQTT_SUBSCRIBE_ID, MQTT_QOS_LEVEL2, topics, topic_cnt, &mqtt_packet) == 0)
	{
		Protocol_Parser_Print(mqtt_packet._data);
		
		//NET_DEVICE_SendData(mqtt_packet._data, mqtt_packet._len);				//��ƽ̨���Ͷ�������
		NET_DEVICE_AddDataSendList(mqtt_packet._data, mqtt_packet._len, 1);		//��������
		
		MQTT_DeleteBuffer(&mqtt_packet);										//ɾ��
	}
	
	return SEND_TYPE_OK;

}

//==========================================================
//	�������ƣ�	OneNET_UnSubscribe
//
//	�������ܣ�	ȡ������
//
//	��ڲ�����	topics�����ĵ�topic
//				topic_cnt��topic����
//
//	���ز�����	SEND_TYPE_OK-���ͳɹ�	SEND_TYPE_UNSUBSCRIBE-��Ҫ�ط�
//
//	˵����		
//==========================================================
unsigned char OneNET_UnSubscribe(const char *topics[], unsigned char topic_cnt)
{
	
	unsigned char i = 0;
	
	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0};							//Э���

	if(!onenet_info.net_work)
		return SEND_TYPE_UNSUBSCRIBE;
	
	for(; i < topic_cnt; i++)
		UsartPrintf(USART_DEBUG, "UnSubscribe Topic: %s\r\n", topics[i]);
	
	if(MQTT_PacketUnSubscribe(MQTT_UNSUBSCRIBE_ID, topics, topic_cnt, &mqtt_packet) == 0)
	{
		Protocol_Parser_Print(mqtt_packet._data);
		
		//NET_DEVICE_SendData(mqtt_packet._data, mqtt_packet._len);				//��ƽ̨����ȡ����������
		NET_DEVICE_AddDataSendList(mqtt_packet._data, mqtt_packet._len, 1);		//��������
		
		MQTT_DeleteBuffer(&mqtt_packet);										//ɾ��
	}
	
	return SEND_TYPE_OK;

}

//==========================================================
//	�������ƣ�	OneNET_CmdHandle
//
//	�������ܣ�	��ȡƽ̨rb�е�����
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void OneNET_CmdHandle(void)
{
	
	unsigned char *dataPtr = NULL, *ipdPtr = NULL;		//����ָ��

	dataPtr = NET_DEVICE_Read();						//�ȴ�����

	if(dataPtr != NULL)									//������Ч
	{
		ipdPtr = NET_DEVICE_GetIPD(dataPtr);			//����Ƿ���ƽ̨����
		if(ipdPtr != NULL)
		{
			net_device_info.send_count = 0;
			
			if(onenet_info.connect_ip)
				onenet_info.cmd_ptr = ipdPtr;			//���д���
			
			net_device_info.cmd_ipd = (char *)ipdPtr;
		}
		else
		{
			if(strstr((char *)dataPtr, "SEND OK") != NULL)
			{
				net_device_info.send_count = 0;
			}
			else if(strstr((char *)dataPtr, "CLOSE") != NULL && onenet_info.net_work)
			{
				UsartPrintf(USART_DEBUG, "WARN:	���ӶϿ���׼������\r\n");
				
				onenet_info.err_check = 1;
			}
			else
				NET_DEVICE_CmdHandle((char *)dataPtr);
		} 
	}

}

//==========================================================
//	�������ƣ�	OneNET_RevPro
//
//	�������ܣ�	ƽ̨�������ݼ��
//
//	��ڲ�����	dataPtr��ƽ̨���ص�����
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void OneNET_RevPro(unsigned char *cmd)
{
	
	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0};							//Э���
	
	char *req_payload = NULL;
	char *cmdid_topic = NULL;
	
	unsigned short topic_len = 0;
	unsigned short req_len = 0;
	
	unsigned char qos = 0;
	static unsigned short pkt_id = 0;
	
	Protocol_Parser_Print(cmd);
	
	switch(MQTT_UnPacketRecv(cmd))
	{
		case MQTT_PKT_CONNACK:
		
			switch(MQTT_UnPacketConnectAck(cmd))
			{
				case 0:
					UsartPrintf(USART_DEBUG, "Tips:	���ӳɹ�\r\n");
					onenet_info.net_work = 1;
				break;
				
				case 1:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ�Э�����\r\n");break;
				case 2:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ��Ƿ���clientid\r\n");break;
				case 3:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ�������ʧ��\r\n");break;
				case 4:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ��û������������\r\n");break;
				case 5:UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ��Ƿ�����(����token�Ƿ�)\r\n");break;
				
				default:UsartPrintf(USART_DEBUG, "ERR:	����ʧ�ܣ�δ֪����\r\n");break;
			}
		
		break;
		
		case MQTT_PKT_PINGRESP:
		
			UsartPrintf(USART_DEBUG, "Tips:	HeartBeat OK\r\n");
			onenet_info.heart_beat = 1;
		
		break;
		
		case MQTT_PKT_CMD:																//�����·�
			
			if(MQTT_UnPacketCmd(cmd, &cmdid_topic, &req_payload, &req_len) == 0)		//���topic����Ϣ��
			{
				UsartPrintf(USART_DEBUG, "cmdid: %s, req: %s, req_len: %d\r\n", cmdid_topic, req_payload, req_len);
				
				//ִ������ص�------------------------------------------------------------
				CALLBACK_Execute(req_payload);
				
				if(MQTT_PacketCmdResp(cmdid_topic, req_payload, &mqtt_packet) == 0)		//����ظ����
				{
					UsartPrintf(USART_DEBUG, "Tips:	Send CmdResp\r\n");
					
					Protocol_Parser_Print(mqtt_packet._data);
					
					//NET_DEVICE_SendData(mqtt_packet._data, mqtt_packet._len);			//�ظ�����
					NET_DEVICE_AddDataSendList(mqtt_packet._data, mqtt_packet._len, 1);	//��������
					MQTT_DeleteBuffer(&mqtt_packet);									//ɾ��
				}
				
				MQTT_FreeBuffer(cmdid_topic);
				MQTT_FreeBuffer(req_payload);
				onenet_info.send_data |= SEND_TYPE_DATA;
			}
		
		break;
			
		case MQTT_PKT_PUBLISH:															//���յ�Publish��Ϣ
		
			if(MQTT_UnPacketPublish(cmd, &cmdid_topic, &topic_len, &req_payload, &req_len, &qos, &pkt_id) == 0)
			{
				UsartPrintf(USART_DEBUG, "topic: %s, topic_len: %d, payload: %s, payload_len: %d\r\n",
																	cmdid_topic, topic_len, req_payload, req_len);
				
				//ִ������ص�------------------------------------------------------------
				CALLBACK_Execute(req_payload);
				
				switch(qos)
				{
					case 1:																//�յ�publish��qosΪ1���豸��Ҫ�ظ�Ack
					
						if(MQTT_PacketPublishAck(pkt_id, &mqtt_packet) == 0)
						{
							UsartPrintf(USART_DEBUG, "Tips:	Send PublishAck\r\n");
							
							Protocol_Parser_Print(mqtt_packet._data);
							
							//NET_DEVICE_SendData(mqtt_packet._data, mqtt_packet._len);
							NET_DEVICE_AddDataSendList(mqtt_packet._data, mqtt_packet._len, 1);//��������
							MQTT_DeleteBuffer(&mqtt_packet);
						}
					
					break;
					
					case 2:																//�յ�publish��qosΪ2���豸�Ȼظ�Rec
																						//ƽ̨�ظ�Rel���豸�ٻظ�Comp
						if(MQTT_PacketPublishRec(pkt_id, &mqtt_packet) == 0)
						{
							UsartPrintf(USART_DEBUG, "Tips:	Send PublishRec\r\n");
							
							Protocol_Parser_Print(mqtt_packet._data);
							
							//NET_DEVICE_SendData(mqtt_packet._data, mqtt_packet._len);
							NET_DEVICE_AddDataSendList(mqtt_packet._data, mqtt_packet._len, 1);//��������
							MQTT_DeleteBuffer(&mqtt_packet);
						}
					
					break;
					
					default:
						break;
				}
				
				MQTT_FreeBuffer(cmdid_topic);
				MQTT_FreeBuffer(req_payload);
				onenet_info.send_data |= SEND_TYPE_DATA;
			}
		
		break;
			
		case MQTT_PKT_PUBACK:															//����Publish��Ϣ��ƽ̨�ظ���Ack
		
			if(MQTT_UnPacketPublishAck(cmd) == 0)
			{
				UsartPrintf(USART_DEBUG, "Tips:	MQTT Publish Send OK\r\n");
				
#if(LBS_WIFI_EN == 1 || LBS_EN == 1)
				if(
#if(LBS_WIFI_EN == 1)
					lbs_wifi_info.lbs_wifi_ok == 1 &&
#elif(LBS_EN == 1)
					lbs_info.lbs_ok == 1 &&
#endif
					!onenet_info.lbs && onenet_info.lbs_count < 4)							//�����ȡ���˻�վ��Ϣ �� δ��ȡ��λ������ �� ��ȡ����С��һ��ֵ
					{
						onenet_info.net_work = 0;											//�����»�ȡһ��λ����Ϣ
						get_location_flag = 1;
					}
#endif
			}
			
		break;
			
		case MQTT_PKT_PUBREC:															//����Publish��Ϣ��ƽ̨�ظ���Rec���豸��ظ�Rel��Ϣ
		
			if(MQTT_UnPacketPublishRec(cmd) == 0)
			{
				UsartPrintf(USART_DEBUG, "Tips:	Rev PublishRec\r\n");
				if(MQTT_PacketPublishRel(MQTT_PUBLISH_ID, &mqtt_packet) == 0)
				{
					UsartPrintf(USART_DEBUG, "Tips:	Send PublishRel\r\n");
					
					Protocol_Parser_Print(mqtt_packet._data);
					
					//NET_DEVICE_SendData(mqtt_packet._data, mqtt_packet._len);
					NET_DEVICE_AddDataSendList(mqtt_packet._data, mqtt_packet._len, 1);	//��������
					MQTT_DeleteBuffer(&mqtt_packet);
				}
			}
		
		break;
			
		case MQTT_PKT_PUBREL:															//�յ�Publish��Ϣ���豸�ظ�Rec��ƽ̨�ظ���Rel���豸���ٻظ�Comp
			
			if(MQTT_UnPacketPublishRel(cmd, pkt_id) == 0)
			{
				UsartPrintf(USART_DEBUG, "Tips:	Rev PublishRel\r\n");
				if(MQTT_PacketPublishComp(pkt_id, &mqtt_packet) == 0)
				{
					UsartPrintf(USART_DEBUG, "Tips:	Send PublishComp\r\n");
					
					Protocol_Parser_Print(mqtt_packet._data);
					
					//NET_DEVICE_SendData(mqtt_packet._data, mqtt_packet._len);
					NET_DEVICE_AddDataSendList(mqtt_packet._data, mqtt_packet._len, 1);	//��������
					MQTT_DeleteBuffer(&mqtt_packet);
				}
			}
		
		break;
		
		case MQTT_PKT_PUBCOMP:															//����Publish��Ϣ��ƽ̨����Rec���豸�ظ�Rel��ƽ̨�ٷ��ص�Comp
		
			if(MQTT_UnPacketPublishComp(cmd) == 0)
			{
				UsartPrintf(USART_DEBUG, "Tips:	Rev PublishComp\r\n");
			}
		
		break;
			
		case MQTT_PKT_SUBACK:															//����Subscribe��Ϣ��Ack
		
			if(MQTT_UnPacketSubscribe(cmd) == 0)
				UsartPrintf(USART_DEBUG, "Tips:	MQTT Subscribe OK\r\n");
			else
				UsartPrintf(USART_DEBUG, "Tips:	MQTT Subscribe Err\r\n");
		
		break;
			
		case MQTT_PKT_UNSUBACK:															//����UnSubscribe��Ϣ��Ack
		
			if(MQTT_UnPacketUnSubscribe(cmd) == 0)
				UsartPrintf(USART_DEBUG, "Tips:	MQTT UnSubscribe OK\r\n");
			else
				UsartPrintf(USART_DEBUG, "Tips:	MQTT UnSubscribe Err\r\n");
		
		break;
		
		default:
			
		break;
	}

}
