#ifndef _ONENET_H_
#define _ONENET_H_

_Bool OneNET_DevLink(void);

void OneNET_SendData(void);
void OneNET_SendCmd(void);
void OneNET_RevPro(unsigned char *cmd);
unsigned char *OneNET_GetIPD(uint16_t timeOut);

#endif
