#ifndef __SI4432_H
#define __SI4432_H
#include "sys.h"

#define  DestID  0XFF
#define  MyID	 0XAA

#define READREG        0x00  	//���Ĵ���ָ��
#define WRITEREG       0x80 	//д�Ĵ���ָ��

// #define Max_Tx_Data_Len 64//���ķ������ݳ���
// #define Max_Rx_Data_Len 64//���Ľ������ݳ���
#define TxBuf_Len 64 
#define RxBuf_Len 64     //����RF4432���ݰ�����
extern u8 RxBuf[RxBuf_Len];
extern u8 TxBuf[TxBuf_Len];

//�˿ڶ���
#define NSS PBout(12)//NSEL  SPIƬѡ�ź�
#define NIRQ PBin(10)
#define SDN PBout(11)//�ڲ���Դ����

#define SI4432_PWRSTATE_READY		0x01		// ģ�� Ready ״̬����
#define SI4432_PWRSTATE_TX		0x09		// ģ�� ����״̬����
#define SI4432_PWRSTATE_RX		0x05		// ģ�� ����״̬����
#define SI4432_PACKET_SENT_INTERRUPT	0x04		// ģ�� ��������ж�
#define SI4432_Rx_packet_received_interrupt   0x02      // ģ�� �յ����ݰ��ж�

 
// extern u8 RxBuf[RxBuf_Len];
// extern u8 TxBuf[TxBuf_Len];  //ÿ�뷢��Ĺ̶����ݵĲ����źţ���10��������ǰ9�����ݵ�У��ͣ��ֱ�Ϊ65��66��67��68��69��70��71��72��73��109
extern u8 ItStatus;  //���ͽ����ж�
extern u8 RSSI;      //RSSI
extern u8 count_50hz;
extern u8 ItStatus1,ItStatus2;
extern u8 rf_timeout;

typedef struct 
{
	
	u8 reach_1s				: 1;
	u8 rf_reach_timeout			: 1;
	u8 is_tx				: 1;
	
	
}	FlagType;



void Si4432_IO_Init(void);
void Si4432_init(void);
u8 SPI_RW_Reg(u8 addr, u8 data_);
void SPI_Read_Buf(u8 addr, u8 *data_, u8 number);
void SPI_Write_Buf(u8 addr, u8 *data_, u8 number);
u8 RF4432_RSSI(void);
u8 RF4432_RxPacket(u8 *rxBuffer);
void RF4432_TxPacket(u8 *dataBuffer);
void SetRX_Mode(void);
void SetTX_Mode(void);
extern void test_al(void);
#endif

