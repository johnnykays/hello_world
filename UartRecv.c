/**
  ******************************************************************************
  * @file    UartRecv.c 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This file create a UART recv thread.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, MXCHIP Inc. SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2014 MXCHIP Inc.</center></h2>
  ******************************************************************************
  **/

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


void uartRecv_thread(void *inContext)
{
  uart_recv_log_trace();
  mico_Context_t *Context = inContext;
  int recvlen;
  uint8_t *inDataBuffer;
  unsigned short crc16;
  
  inDataBuffer = malloc(UART_ONE_PACKAGE_LENGTH);
  require(inDataBuffer, exit);
  
  while(1) {
    recvlen = _uart_get_one_packet(inDataBuffer, UART_ONE_PACKAGE_LENGTH);
    if (recvlen <= 0)
      continue;
    
    crc16 = Modbus_Data_Process(inDataBuffer,recvlen);
    if(crc16 == get_CRC16((char*)inDataBuffer,recvlen-2))
    {
      sppUartCommandProcess(inDataBuffer, recvlen, Context);
    }
    
    MicoUartSend(UART_FOR_SERVERACK,"11111111111111111111", 20);///////
    MicoUartSend(UART_FOR_SERVERACK,inDataBuffer, recvlen);///////
  }
  
exit:
  if(inDataBuffer) free(inDataBuffer);
  mico_rtos_delete_thread(NULL);
}

/* Packet format: BB 00 CMD(2B) Status(2B) datalen(2B) data(x) checksum(2B)
* copy to buf, return len = datalen+10
*/
size_t _uart_get_one_packet(uint8_t* inBuf, int inBufLen)
{
  uart_recv_log_trace();

  int datalen;
  
  while(1) {
    if( MicoUartRecv( UART_FOR_APP, inBuf, inBufLen, UART_RECV_TIMEOUT) == kNoErr){
      return inBufLen;
    }
   else{
     datalen = MicoUartGetLengthInBuffer( UART_FOR_APP );
     if(datalen){
       MicoUartRecv(UART_FOR_APP, inBuf, datalen, UART_RECV_TIMEOUT);
       return datalen;
     }
   }
  }
}

unsigned short Modbus_Data_Process(uint8_t* recvbuf,int len)
{
  unsigned short temp;    
  if(len <= 3)
  {
    return 0;
  }
  temp = recvbuf[len-1] << 8;
  temp = temp | recvbuf[len-2];  
  temp = (temp >> 8) | ((temp&0x00ff)) << 8;  
  return temp;
}
