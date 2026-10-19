#ifndef __APP_BLUETOOTH_H_
#define __APP_BLUETOOTH_H_

#include "main.h"

void Init_PID(void);
void ResetPID(void);
void ProtocolGetPID(void);

void deal_bluetooth(uint8_t rxbuf);
void ProtocolCpyData(void);
void Protocol(void);

int StringFind(const char *pSrc, const char *pDst) ;
void SendAutoUp(void);//自动上报 放在time6里面 Automatically report and put it in time6
#endif

