/**
 ************************************************************
 ************************************************************
 ************************************************************
 *	�ļ����� 	onenet.c
 *
 *	���ߣ� 		�ż���
 *
 *	���ڣ� 		2017-05-09
 *
 *	�汾�� 		V1.1
 *
 *	˵���� 		��onenetƽ̨�����ݽ����ӿڲ�
 *
 *	�޸ļ�¼��
 ************************************************************
 ************************************************************
 ************************************************************
 **/

// ��Ƭ��ͷ�ļ�

// �����豸
#include "bsp_esp8266.h"

// Э���ļ�
#include "onenet.h"

// Ӳ������
#include "usart.h"
#include "delay.h"

// C��
#include <string.h>
#include <stdio.h>

#define DEVID "1043147880"

#define APIKEY "m=cXh=O8EFYps6fn7rKDcVZZrVU="

extern uint8_t humidityH;	 // ʪ����������
extern uint8_t humidityL;	 // ʪ��С������
extern uint8_t temperatureH; // �¶���������
extern uint8_t temperatureL; // �¶�С������

void OneNet_FillBuf(char *buf)
{

	char text[24];	// �����ʱ����
	char buf1[128]; // ���յ�����

	memset(text, 0, sizeof(text));
	memset(buf1, 0, sizeof(buf1));

	// Ҫ��ʼ����json��ʽ��

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

	// ��ʽ������
	sprintf(buf, "POST /devices/%s/datapoints?type=3 HTTP/1.1\r\napi-key:%s\r\nHost:api.heclouds.com\r\n"
				 "Content-Length:%d\r\n\r\n",

			DEVID, APIKEY, strlen(buf1));

	strcat(buf, buf1); // ��Ҫ���͵�����ƴ�ӵ�buf�������ݵĺ���

	// ��ʱbuf����������Ҫ�ϴ������ݣ��豸id,apikey�����ݳ��ȣ��ɼ������ݣ�
	printf("pub_buf=%s\r\n", buf);
}

//==========================================================
//	�������ƣ�	OneNet_SendData
//
//	�������ܣ�	�ϴ����ݵ�ƽ̨
//
//	��ڲ�����	type���������ݵĸ�ʽ
//
//	���ز�����	��
//
//	˵����
//==========================================================
void OneNet_SendData(void)
{

	char buf[256];

	memset(buf, 0, sizeof(buf)); // �����������

	OneNet_FillBuf(buf); // ��װ������������Ҫ�ϴ������ݷ�װ�����棩

	esp8266.SendData();(unsigned char *)buf, strlen(buf)); // �ϴ��Ѿ���װ�õ�����
}

//==========================================================
//	�������ƣ�	OneNet_RevPro
//
//	�������ܣ�	ƽ̨�������ݼ��
//
//	��ڲ�����	dataPtr��ƽ̨���ص�����
//
//	���ز�����	��
//
//	˵����
//==========================================================
void OneNet_RevPro(unsigned char *dataPtr)
{

	if (strstr((char *)dataPtr, "CLOSED"))
	{
		printf("TCP CLOSED\r\n");
	}
	else
	{
		// ������������Ƿ��ͳɹ�
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
