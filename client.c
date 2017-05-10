

#include "MICOAppDefine.h"
#include "MICODefine.h"
#include "SppProtocol.h"
#include "MicoPlatform.h"
#include "MICONotificationCenter.h"

#define uart_recv_log(M, ...) custom_log("UART RECV", M, ##__VA_ARGS__)
#define uart_recv_log_trace() custom_log_trace("UART RECV")

int length;
char slavedata[64]={0};

static size_t _uart_get_one_packet(uint8_t* buf, int maxlen);

unsigned short Modbus_Data_Process(uint8_t* recvbuf,int len);
extern unsigned short get_CRC16(char *ptr,int len);
void Function3_RecvData(uint8_t* data,int count);

