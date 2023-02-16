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
#ifndef __ESP8266_TEST_H
#define __ESP8266_TEST_H

/********************************** 头文件包含区 **********************************/

#include <stdint.h>
#include "esp8266.h"

/********************************** 用户需要设置的参数**********************************/
#define macUser_ESP8266_ApSsid "CCTV6"      // 要连接的热点的名称
#define macUser_ESP8266_ApPwd "15905890644" // 要连接的热点的密钥

#define macUser_ESP8266_TcpServer_IP "183.230.40.39" // 要连接的服务器的 IP
#define macUser_ESP8266_TcpServer_Port "6002"        // 要连接的服务器的端口

#define macUser_ESP8266_RESTART_PIN 34

/********************************** 外部全局变量 ***************************************/
extern Esp8266Object_t esp8266;

/********************************** 测试函数声明 ***************************************/
void ESP8266_StaTcpClient_Unvarnish_ConfigTest(void);
// 此函数放到主循环中
void ESP8266_CheckRecvDataTest(void);

void esp8266Init(void);
#endif
/************************************* END *****************************************/
