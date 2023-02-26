#ifndef _ONENET_H_
#define _ONENET_H_

/* 状态寄存器 */
typedef union
{
    struct
    {
        unsigned short net_work : 1;   // 1-OneNET接入成功		0-OneNET接入失败
        unsigned short err_count : 3;  // 错误计数
        unsigned short heart_beat : 1; // 心跳
        unsigned short get_ip : 1;     // 获取到最优登录IP
        unsigned short lbs : 1;        // 1-已获取到了位置坐标
        unsigned short lbs_count : 3;  // 获取计数
        unsigned short connect_ip : 1; // 连接了IP
        unsigned short err_check : 1;  // 错误检测
        unsigned short reverse : 4;
    } bit;
    unsigned short all;

} onenet_statue_t;

/* 文件信息 */
typedef struct
{
    char *file_bin_name;           // bin 文件名称
    const unsigned char *file_bin; // bin 文件
    unsigned int file_bin_size;    // bin 文件大小
} onenet_file_info_t;

/* 用户信息 */
typedef struct
{
    char dev_id[16];  // 设备 ID
    char api_key[32]; // API key

    char pro_id[10]; // 产品ID
    char auif[50];   // 权鉴信息

    char reg_code[24]; // 注册码
} onenet_user_info_t;

/* Server 信息 */
typedef struct
{
    char ip[16];  // Web 服务器IP
    char port[8]; // 协议端口

    const unsigned char protocol; // 协议类型号		1-edp	2-nwx	3-jtext		4-Hiscmd
                                  //				5-jt808			6-modbus	7-mqtt
                                  //				8-gr20			9-reg		10-HTTP(自定义)
} onenet_web_info_t;

/*OneNet结构体*/
typedef struct
{
    onenet_web_info_t web_info;   // 云端信息
    onenet_user_info_t user_info; // 用户信息
    onenet_file_info_t file_info; // 远程升级信息
    onenet_statue_t SR;           // OneNet 连接状态

    unsigned char send_data; // 发送数据的类型
    unsigned char *cmd_ptr;  // 收到的云端命令

} onenet_t;

extern onenet_t G_oneNet;

#define SEND_TYPE_OK			(1 << 0)	// 发送完毕
#define SEND_TYPE_DATA			(1 << 1)	// 需要再次发送
#define SEND_TYPE_HEART			(1 << 2)	// 发送心跳包(保活)
#define SEND_TYPE_PUBLISH		(1 << 3)	// 发送发布
#define SEND_TYPE_SUBSCRIBE		(1 << 4)	// 发送订阅
#define SEND_TYPE_UNSUBSCRIBE	(1 << 5)	// 发送取消订阅
#define SEND_TYPE_BINFILE		(1 << 6)	// 发送二进制文件

uint8_t onenet_dev_link(const char *devid, const char *proid, const char *auth_info);
unsigned char onenet_send_heart(void);
_Bool onenet_check_heart(void);
unsigned char onenet_publish(const char *topic, const char *msg);
void onenet_rev_pro(unsigned char *cmd);

_Bool OneNET_DevLink(void);
uint8_t OneNET_SendData(void);
void OneNET_SendCmd(void);
void OneNET_RevPro(unsigned char *cmd);
unsigned char *OneNET_GetIPD(uint16_t timeOut);
void OneNet_Init(char *device_id, char *api_key, char *auth_info, char *product_id, char *ip, char *port);
#endif
