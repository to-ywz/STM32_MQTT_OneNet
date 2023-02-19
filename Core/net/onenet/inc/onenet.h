#ifndef _ONENET_H_
#define _ONENET_H_


#include "dstream.h"




typedef struct
{

    char dev_id[16];
    char api_key[32];
	
	char pro_id[10];
	char auif[50];
	
	char reg_code[24];
	
	char ip[16];
	char port[8];
	
	char *file_bin_name;
	const unsigned char *file_bin;
	unsigned int file_bin_size;
	
	const unsigned char protocol;	//Э�����ͺ�		1-edp	2-nwx	3-jtext		4-Hiscmd
									//				5-jt808			6-modbus	7-mqtt
									//				8-gr20			9-reg		10-HTTP(�Զ���)
	
	unsigned char *cmd_ptr;
	
	unsigned char send_data;
	
	unsigned short net_work : 1;	//1-OneNET����ɹ�		0-OneNET����ʧ��
	unsigned short err_count : 3;	//�������
	unsigned short heart_beat : 1;	//����
	unsigned short get_ip : 1;		//��ȡ�����ŵ�¼IP
	unsigned short lbs : 1;			//1-�ѻ�ȡ����λ������
	unsigned short lbs_count : 3;	//��ȡ����
	unsigned short connect_ip : 1;	//������IP
	unsigned short err_check : 1;	//������
	unsigned short reverse : 4;

} ONETNET_INFO;

extern ONETNET_INFO onenet_info;


#define SEND_TYPE_OK			(1 << 0)	//
#define SEND_TYPE_DATA			(1 << 1)	//
#define SEND_TYPE_HEART			(1 << 2)	//
#define SEND_TYPE_PUBLISH		(1 << 3)	//
#define SEND_TYPE_SUBSCRIBE		(1 << 4)	//
#define SEND_TYPE_UNSUBSCRIBE	(1 << 5)	//
#define SEND_TYPE_BINFILE		(1 << 6)	//


_Bool OneNET_RepetitionCreateFlag(const char *apikey);

_Bool OneNET_CreateDevice(const char *reg_code, const char *dev_name, const char *auth_info, char *devid, char *apikey);

_Bool OneNET_GetLinkIP(unsigned char protocol, char *ip, char *port);

unsigned char OneNET_GetLocation(const char *device_id, const char *api_key, char *lon, char *lat);

_Bool OneNET_ConnectIP(char *ip, char *port);

void OneNET_DevLink(const char* devid, const char *proid, const char* auth_info);

_Bool OneNET_DisConnect(void);

unsigned char OneNET_SendData(FORMAT_TYPE type, char *devid, char *apikey, DATA_STREAM *streamArray, unsigned short streamArrayCnt);

unsigned char OneNET_Send_BinFile(char *name, const unsigned char *file, unsigned int file_size);

unsigned char OneNET_Subscribe(const char *topics[], unsigned char topic_cnt);

unsigned char OneNET_UnSubscribe(const char *topics[], unsigned char topic_cnt);

unsigned char OneNET_Publish(const char *topic, const char *msg);

unsigned char OneNET_SendData_Heart(void);

_Bool OneNET_Check_Heart(void);

void OneNET_CmdHandle(void);

void OneNET_RevPro(unsigned char *dataPtr);

#endif
