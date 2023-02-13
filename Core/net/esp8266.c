/**
 * @brief:                  基于HAL库的ESP8266驱动
 * @Revision:               V1.0
 * @author:                 点灯大师
 * @Affiliated unit：       	黄河科技学院
 * @Email:                  im_leihao@163.com
 * @github:                 https://github.com/zleihao
 * @note                    以下驱动移植野火WIFI全程
 */
#include "esp8266.h"

struct STRUCT_USARTx_Fram strEsp8266_Fram_Record = {0};
struct STRUCT_USARTx_Fram strUSART_Fram_Record = {0};

/**
 * @brief  ESP8266初始化函数
 * @param  无
 * @retval 无
 */
void ESP8266_Init(void)
{
	macESP8266_RST_HIGH_LEVEL();
	macESP8266_CH_DISABLE();
}

/*
 * 函数名：ESP8266_Rst
 * 描述  ：重启WF-ESP8266模块
 * 输入  ：无
 * 返回  : 无
 * 调用  ：被 ESP8266_AT_Test 调用
 */
void ESP8266_Rst(void)
{
#if 0
	 ESP8266_Cmd ( "AT+RST", "OK", "ready", 2500 );

#else
	macESP8266_RST_LOW_LEVEL();
	HAL_Delay(500);
	macESP8266_RST_HIGH_LEVEL();
#endif
}

/*
 * 函数名：ESP8266_Cmd
 * 描述  ：对WF-ESP8266模块发送AT指令
 * 输入  ：cmd，待发送的指令
 *         reply1，reply2，期待的响应，为NULL表不需响应，两者为或逻辑关系
 *         waittime，等待响应的时间
 * 返回  : 1，指令发送成功
 *         0，指令发送失败
 * 调用  ：被外部调用
 */
bool ESP8266_Cmd(char *cmd, char *reply1, char *reply2, uint32_t waittime)
{
	memset(strEsp8266_Fram_Record.Data_RX_BUF, 0, RX_BUF_MAX_LEN);
	strEsp8266_Fram_Record.InfBit.FramLength = 0; // 从新开始接收新的数据包

	macESP8266_Usart("%s\r\n", cmd);

	if ((reply1 == 0) && (reply2 == 0)) // 不需要接收数据
		return true;

	HAL_Delay(waittime); // 延时

	uint8_t temp = 0;

	do // 获取数据
	{
		// 未收到数据
		if (ringbuffer_data_len(uart2.rx_Buffer) == RINGBUFFER_EMPTY)
			break;

		ringbuffer_getchar(uart2.rx_Buffer, &temp);
		strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength] = temp;
		strEsp8266_Fram_Record.InfBit.FramLength++;

	} while (temp != '\0');

	strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength] = '\0';

	macPC_Usart("%s", strEsp8266_Fram_Record.Data_RX_BUF);
	strEsp8266_Fram_Record.InfBit.FramLength = 0; // 清除接收标志
	strEsp8266_Fram_Record.InfBit.FramFinishFlag = 0;

	// 数据校验
	if ((reply1 != 0) && (reply2 != 0))
		return ((bool)strstr(strEsp8266_Fram_Record.Data_RX_BUF, reply1) ||
				(bool)strstr(strEsp8266_Fram_Record.Data_RX_BUF, reply2));

	else if (reply1 != 0)
		return ((bool)strstr(strEsp8266_Fram_Record.Data_RX_BUF, reply1));

	else
		return ((bool)strstr(strEsp8266_Fram_Record.Data_RX_BUF, reply2));
}

bool ESP8266_AT_Test(void)
{
	char count = 0;

	macESP8266_RST_HIGH_LEVEL();
	printf("\r\nAT test.....\r\n");
	HAL_Delay(2000);
	while (count < 10)
	{
		printf("\r\nAT Testing frequency: %d......\r\n", count);
		if (ESP8266_Cmd("AT", "OK", NULL, 500))
		{
			printf("\r\nAT Test started successfully: %d......\r\n", count);
			return 1;
		}
		ESP8266_Rst();
		++count;
	}
	return 0;
}

/*
 * 函数名：ESP8266_Net_Mode_Choose
 * 描述  ：选择WF-ESP8266模块的工作模式
 * 输入  ：enumMode，工作模式
 * 返回  : 1，选择成功
 *         0，选择失败
 * 调用  ：被外部调用
 */
bool ESP8266_Net_Mode_Choose(ENUM_Net_ModeTypeDef enumMode)
{
	switch (enumMode)
	{
	case STA:
		return ESP8266_Cmd("AT+CWMODE=1", "OK", "no change", 2500);

	case AP:
		return ESP8266_Cmd("AT+CWMODE=2", "OK", "no change", 2500);

	case STA_AP:
		return ESP8266_Cmd("AT+CWMODE=3", "OK", "no change", 2500);

	default:
		return false;
	}
}

/*
 * 函数名：ESP8266_JoinAP
 * 描述  ：WF-ESP8266模块连接外部WiFi
 * 输入  ：pSSID，WiFi名称字符串
 *       ：pPassWord，WiFi密码字符串
 * 返回  : 1，连接成功
 *         0，连接失败
 * 调用  ：被外部调用
 */
bool ESP8266_JoinAP(char *pSSID, char *pPassWord)
{
	char cCmd[120];

	sprintf(cCmd, "AT+CWJAP=\"%s\",\"%s\"", pSSID, pPassWord);

	return ESP8266_Cmd(cCmd, "OK", NULL, 5000);
}

/*
 * 函数名：ESP8266_BuildAP
 * 描述  ：WF-ESP8266模块创建WiFi热点
 * 输入  ：pSSID，WiFi名称字符串
 *       ：pPassWord，WiFi密码字符串
 *       ：enunPsdMode，WiFi加密方式代号字符串
 * 返回  : 1，创建成功
 *         0，创建失败
 * 调用  ：被外部调用
 */
bool ESP8266_BuildAP(char *pSSID, char *pPassWord, ENUM_AP_PsdMode_TypeDef enunPsdMode)
{
	char cCmd[120];

	sprintf(cCmd, "AT+CWSAP=\"%s\",\"%s\",1,%d", pSSID, pPassWord, enunPsdMode);

	return ESP8266_Cmd(cCmd, "OK", 0, 1000);
}

/*
 * 函数名：ESP8266_Enable_MultipleId
 * 描述  ：WF-ESP8266模块启动多连接
 * 输入  ：enumEnUnvarnishTx，配置是否多连接
 * 返回  : 1，配置成功
 *         0，配置失败
 * 调用  ：被外部调用
 */
bool ESP8266_Enable_MultipleId(FunctionalState enumEnUnvarnishTx)
{
	char cStr[20];

	sprintf(cStr, "AT+CIPMUX=%d", (enumEnUnvarnishTx ? 1 : 0));

	return ESP8266_Cmd(cStr, "OK", 0, 500);
}

/*
 * 函数名：ESP8266_Link_Server
 * 描述  ：WF-ESP8266模块连接外部服务器
 * 输入  ：enumE，网络协议
 *       ：ip，服务器IP字符串
 *       ：ComNum，服务器端口字符串
 *       ：id，模块连接服务器的ID
 * 返回  : 1，连接成功
 *         0，连接失败
 * 调用  ：被外部调用
 */
bool ESP8266_Link_Server(ENUM_NetPro_TypeDef enumE, char *ip, char *ComNum, ENUM_ID_NO_TypeDef id)
{
	char cStr[100] = {0}, cCmd[120];

	switch (enumE)
	{
	case enumTCP:
		sprintf(cStr, "\"%s\",\"%s\",%s", "TCP", ip, ComNum);
		break;

	case enumUDP:
		sprintf(cStr, "\"%s\",\"%s\",%s", "UDP", ip, ComNum);
		break;

	default:
		break;
	}

	if (id < 5)
		sprintf(cCmd, "AT+CIPSTART=%d,%s", id, cStr);

	else
		sprintf(cCmd, "AT+CIPSTART=%s", cStr);

	return ESP8266_Cmd(cCmd, "OK", "ALREAY CONNECT", 4000);
}

/*
 * 函数名：ESP8266_StartOrShutServer
 * 描述  ：WF-ESP8266模块开启或关闭服务器模式
 * 输入  ：enumMode，开启/关闭
 *       ：pPortNum，服务器端口号字符串
 *       ：pTimeOver，服务器超时时间字符串，单位：秒
 * 返回  : 1，操作成功
 *         0，操作失败
 * 调用  ：被外部调用
 */
bool ESP8266_StartOrShutServer(FunctionalState enumMode, char *pPortNum, char *pTimeOver)
{
	char cCmd1[120], cCmd2[120];

	if (enumMode)
	{
		sprintf(cCmd1, "AT+CIPSERVER=%d,%s", 1, pPortNum);

		sprintf(cCmd2, "AT+CIPSTO=%s", pTimeOver);

		return (ESP8266_Cmd(cCmd1, "OK", 0, 500) &&
				ESP8266_Cmd(cCmd2, "OK", 0, 500));
	}

	else
	{
		sprintf(cCmd1, "AT+CIPSERVER=%d,%s", 0, pPortNum);

		return ESP8266_Cmd(cCmd1, "OK", 0, 500);
	}
}

/*
 * 函数名：ESP8266_Get_LinkStatus
 * 描述  ：获取 WF-ESP8266 的连接状态，较适合单端口时使用
 * 输入  ：无
 * 返回  : 2，获得ip
 *         3，建立连接
 *         3，失去连接
 *         0，获取状态失败
 * 调用  ：被外部调用
 */
uint8_t ESP8266_Get_LinkStatus(void)
{
	if (ESP8266_Cmd("AT+CIPSTATUS", "OK", 0, 500))
	{
		if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "STATUS:2\r\n"))
			return 2;

		else if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "STATUS:3\r\n"))
			return 3;

		else if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "STATUS:4\r\n"))
			return 4;
	}

	return 0;
}

/*
 * 函数名：ESP8266_Get_IdLinkStatus
 * 描述  ：获取 WF-ESP8266 的端口（Id）连接状态，较适合多端口时使用
 * 输入  ：无
 * 返回  : 端口（Id）的连接状态，低5位为有效位，分别对应Id5~0，某位若置1表该Id建立了连接，若被清0表该Id未建立连接
 * 调用  ：被外部调用
 */
uint8_t ESP8266_Get_IdLinkStatus(void)
{
	uint8_t ucIdLinkStatus = 0x00;

	if (ESP8266_Cmd("AT+CIPSTATUS", "OK", 0, 500))
	{
		if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "+CIPSTATUS:0,"))
			ucIdLinkStatus |= 0x01;
		else
			ucIdLinkStatus &= ~0x01;

		if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "+CIPSTATUS:1,"))
			ucIdLinkStatus |= 0x02;
		else
			ucIdLinkStatus &= ~0x02;

		if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "+CIPSTATUS:2,"))
			ucIdLinkStatus |= 0x04;
		else
			ucIdLinkStatus &= ~0x04;

		if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "+CIPSTATUS:3,"))
			ucIdLinkStatus |= 0x08;
		else
			ucIdLinkStatus &= ~0x08;

		if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "+CIPSTATUS:4,"))
			ucIdLinkStatus |= 0x10;
		else
			ucIdLinkStatus &= ~0x10;
	}

	return ucIdLinkStatus;
}

/*
 * 函数名：ESP8266_Inquire_ApIp
 * 描述  ：获取 F-ESP8266 的 AP IP
 * 输入  ：pApIp，存放 AP IP 的数组的首地址
 *         ucArrayLength，存放 AP IP 的数组的长度
 * 返回  : 0，获取失败
 *         1，获取成功
 * 调用  ：被外部调用
 */
uint8_t ESP8266_Inquire_ApIp(char *pApIp, uint8_t ucArrayLength)
{
	char uc;

	char *pCh;

	ESP8266_Cmd("AT+CIFSR", "OK", 0, 500);

	pCh = strstr(strEsp8266_Fram_Record.Data_RX_BUF, "APIP,\"");

	if (pCh)
		pCh += 6;

	else
		return 0;

	for (uc = 0; uc < ucArrayLength; uc++)
	{
		pApIp[uc] = *(pCh + uc);

		if (pApIp[uc] == '\"')
		{
			pApIp[uc] = '\0';
			break;
		}
	}

	return 1;
}

/*
 * 函数名：ESP8266_UnvarnishSend
 * 描述  ：配置WF-ESP8266模块进入透传发送
 * 输入  ：无
 * 返回  : 1，配置成功
 *         0，配置失败
 * 调用  ：被外部调用
 */
bool ESP8266_UnvarnishSend(void)
{
	if (!ESP8266_Cmd("AT+CIPMODE=1", "OK", 0, 500))
		return false;

	return ESP8266_Cmd("AT+CIPSEND", "OK", ">", 500);
}

/*
 * 函数名：ESP8266_ExitUnvarnishSend
 * 描述  ：配置WF-ESP8266模块退出透传模式
 * 输入  ：无
 * 返回  : 无
 * 调用  ：被外部调用
 */
void ESP8266_ExitUnvarnishSend(void)
{
	HAL_Delay(1000);

	macESP8266_Usart("+++");

	HAL_Delay(500);
}

/*
 * 函数名：ESP8266_SendString
 * 描述  ：WF-ESP8266模块发送字符串
 * 输入  ：enumEnUnvarnishTx，声明是否已使能了透传模式
 *       ：pStr，要发送的字符串
 *       ：ulStrLength，要发送的字符串的字节数
 *       ：ucId，哪个ID发送的字符串
 * 返回  : 1，发送成功
 *         0，发送失败
 * 调用  ：被外部调用
 */
bool ESP8266_SendString(FunctionalState enumEnUnvarnishTx, char *pStr, uint32_t ulStrLength, ENUM_ID_NO_TypeDef ucId)
{
	char cStr[20];
	bool bRet = false;

	if (enumEnUnvarnishTx)
	{
		macESP8266_Usart("%s", pStr);

		bRet = true;
	}

	else
	{
		if (ucId < 5)
			sprintf(cStr, "AT+CIPSEND=%d,%d", ucId, ulStrLength + 2);

		else
			sprintf(cStr, "AT+CIPSEND=%d", ulStrLength + 2);

		ESP8266_Cmd(cStr, "> ", 0, 100);

		bRet = ESP8266_Cmd(pStr, "SEND OK", 0, 500);
	}

	return bRet;
}

/*
 * 函数名：ESP8266_ReceiveString
 * 描述  ：WF-ESP8266模块接收字符串
 * 输入  ：enumEnUnvarnishTx，声明是否已使能了透传模式
 * 返回  : 接收到的字符串首地址
 * 调用  ：被外部调用
 */
char *ESP8266_ReceiveString(FunctionalState enumEnUnvarnishTx)
{
	char *pRecStr = 0;

	strEsp8266_Fram_Record.InfBit.FramLength = 0;
	strEsp8266_Fram_Record.InfBit.FramFinishFlag = 0;

	while (!strEsp8266_Fram_Record.InfBit.FramFinishFlag)
		;
	strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength] = '\0';

	if (enumEnUnvarnishTx)
		pRecStr = strEsp8266_Fram_Record.Data_RX_BUF;

	else
	{
		if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "+IPD"))
			pRecStr = strEsp8266_Fram_Record.Data_RX_BUF;
	}

	return pRecStr;
}

/* 下面的是USART1与USART2中断函数，你可以放在 xx_it.c 中，也可以放到这里，记住：二者选其一 */
// void USART1_IRQHandler(void)
// {
// 	uint8_t ucCh;
// 	if ( LL_USART_IsActiveFlag_RXNE(USART1) != RESET )
// 	{
// 		ucCh  = LL_USART_ReceiveData8( USART1 );

// 		if ( strUSART_Fram_Record .InfBit .FramLength < ( RX_BUF_MAX_LEN - 1 ) )                       //预留1个字节写结束符
// 			   strUSART_Fram_Record .Data_RX_BUF [ strUSART_Fram_Record .InfBit .FramLength ++ ]  = ucCh;

// 	}

// 	if ( LL_USART_IsActiveFlag_IDLE( USART1) == SET )     //数据帧接收完毕
// 	{
// 		strUSART_Fram_Record .InfBit .FramFinishFlag = 1;

// 		ucCh = LL_USART_ReceiveData8( USART1 );
//   	}
// }

// /**
//   * @brief This function handles USART2 global interrupt.
//   */
// void USART2_IRQHandler(void)
// {
// 	uint8_t ucCh;
// 	if (LL_USART_IsActiveFlag_RXNE(USART2) != RESET) {
// 	ucCh = LL_USART_ReceiveData8(USART2);

// 	if ( strEsp8266_Fram_Record .InfBit .FramLength < ( RX_BUF_MAX_LEN - 1 ) )                       //预留1个字节写结束符
// 		strEsp8266_Fram_Record .Data_RX_BUF [ strEsp8266_Fram_Record .InfBit .FramLength ++ ]  = ucCh;

// 	}

// 	if (LL_USART_IsActiveFlag_IDLE(USART2) == SET) {
// 		strEsp8266_Fram_Record .InfBit .FramFinishFlag = 1;

// 		ucCh = LL_USART_ReceiveData8(USART2);                 //获取连接状态
// 		ucTcpClosedFlag = strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "CLOSED\r\n" ) ? 1 : 0;
// 	}
// }
