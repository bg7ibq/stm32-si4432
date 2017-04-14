//2015.02.20 433mhz testing program
#include "si4432.h"
#include "spi2.h"
#include "key.h"
#include "usart.h"
#include "delay.h"
// #include "string.h"

#define  TX1_RX0	SPI_RW_Reg(0x0e|0x80, 0x01)		// ����״̬�����߿��ض���
#define  TX0_RX1	SPI_RW_Reg(0x0e|0x80, 0x02)		// ����״̬�����߿��ض���
#define  TX0_RX0	SPI_RW_Reg(0x0e|0x80, 0x00)

FlagType Flag;

u8 RxBuf[RxBuf_Len]={0};
u8 TxBuf[TxBuf_Len] = {0};  //����Ĺ̶����ݵĲ����źţ���10��������ǰ9�����ݵ�У��ͣ��ֱ�Ϊ65��66��67��68��69��70��71��72��73��109
u8 ItStatus;  //���ͽ����ж�
u8 RSSI;      //RSSI

u8 count_50hz;
u8 ItStatus1,ItStatus2;
u8 rf_timeout;

//SI4432��STM32����������Ϣ
//STM32��IO��������Ϣ
void Si4432_IO_Init(void)
{
	RCC->APB2ENR|=1<<3;     //ʹ��PORTBʱ��  
	GPIOB->CRH&=0XFFF000FF;//PA10���ó���������	  
	GPIOB->CRH|=0X00033800;//PA11\PA12���ó����������				   
	GPIOB->ODR|=0x07<<10;	   //PA10����,Ĭ������
	SPI2_Init();			 //��ʼ��SPI2�� 
	SPI2_SetSpeed(SPI2_SPEED_8);	
}


//SPI��дSI4432�ļĴ�������
//������u8 add Ҫ��д�ļĴ����ĵ�ַ
//		u8 data_ Ҫд��Ĵ�����ֵ
//���أ�u8 status ��д�ɹ���״̬
u8 SPI_RW_Reg(u8 addr, u8 data_)
{
	u8 status;
	
	NSS = 0;
	SPI2_ReadWriteByte(addr);
	status = SPI2_ReadWriteByte(data_);
	NSS = 1;
	
	return(status);
}


//��SI4432�ĵ�ַadd����ȡ����Ϊnumber�ֽڵ����ݴ���ָ��*data_��ָ��ĵ�ַ�ռ�
//������add Ҫ��ȡ��SI4432����ʼ��ַ
//      *data_���ݶ�ȡ���ŵĵ�ַ
//		number��ȡ����Ϣ����
//		���ض������ֽ���
void SPI_Read_Buf(u8 addr, u8 *data_, u8 number)
{
	int number_ctr;
	
	NSS = 0;
	SPI2_ReadWriteByte(addr);
	for(number_ctr=0;number_ctr<number;number_ctr++)
	{
		data_[number_ctr] = SPI2_ReadWriteByte(0);
	}	
		NSS = 1;
}


//��SI4432�ĵ�ַadd��д�����*data_���ĳ���Ϊnumber������
//������add Ҫд��ĵ�ַ
//		*data_��д��ֵ��ŵĵ�ַ
//		numberҪд������ݵĳ���
void SPI_Write_Buf(u8 addr, u8 *data_, u8 number)
{
	u8 number_ctr;
	NSS = 0;  
	SPI2_ReadWriteByte(addr);   
	for(number_ctr=0; number_ctr<number; number_ctr++)
	{
// 		send_byte(*data_);
		SPI2_ReadWriteByte(*data_++);
		
	}
	NSS = 1;
}

//SI4432��ʼ������
void Si4432_init(void)
{  

	Si4432_IO_Init();	//SI4432�ӿڳ�ʼ��
	SDN = 1;			//�ر�оƬ
 	delay_ms(10);	// RF ģ���ϵ縴λ
 	
 	SDN = 0;
	delay_ms(500);
	SPI_RW_Reg(READREG + 0x03, 0x00);	 //��RFģ���ж�	
	SPI_RW_Reg(READREG + 0x04, 0x00);
	
	SPI_RW_Reg(WRITEREG + 0x06, 0x80);  //ʹ��ͬ�������
	SPI_RW_Reg(WRITEREG + 0x07, 0x01);  //���� Ready ģʽ
	
//	SPI_RW_Reg(WRITEREG + 0x09, 0x7f);  //���ص���= 12P
	SPI_RW_Reg(WRITEREG + 0x09, 0xD7);  //���ص���= 9P
	SPI_RW_Reg(WRITEREG + 0x69, 0x60);  //AGC
	SPI_RW_Reg(WRITEREG + 0x0a, 0x05);  //�رյ�Ƶ���
	SPI_RW_Reg(WRITEREG + 0x0b, 0xea);  //GPIO 0 ������ͨ�����
	SPI_RW_Reg(WRITEREG + 0x0c, 0xea);  //GPIO 1 ������ͨ�����
//	SPI_RW_Reg(WRITEREG + 0x0b, 0x15);  //GPIO 0 ����RX
//	SPI_RW_Reg(WRITEREG + 0x0c, 0x12);  //GPIO 1 ����TX
	SPI_RW_Reg(WRITEREG + 0x0d, 0xf4);  //GPIO 2 ����յ�������
//	SPI_RW_Reg(WRITEREG + 0x0c, 0xea);  //GPIO 1 ������ͨ�����
//	SPI_RW_Reg(WRITEREG + 0x0d, 0xf4);  //GPIO 2 ����յ�������
	SPI_RW_Reg(WRITEREG + 0x70, 0x2c);  
	SPI_RW_Reg(WRITEREG + 0x1d, 0x40);  //ʹ�� afc
	
	// 1.2K bps setting
//-------rx bw------------------------
	SPI_RW_Reg(WRITEREG + 0x1c, 0x16);  //����16��Nibble��Preamble
	SPI_RW_Reg(WRITEREG + 0x20, 0x83);   
	SPI_RW_Reg(WRITEREG + 0x21, 0xc0);  //
	SPI_RW_Reg(WRITEREG + 0x22, 0x13);  // 
	SPI_RW_Reg(WRITEREG + 0x23, 0xa9);  //
	SPI_RW_Reg(WRITEREG + 0x24, 0x00);  //
//	SPI_RW_Reg(WRITEREG + 0x25, 0x04);  //
	SPI_RW_Reg(WRITEREG + 0x25, 0x03);  //
	SPI_RW_Reg(WRITEREG + 0x2a, 0x14);
//----data rate------------------
	SPI_RW_Reg(WRITEREG + 0x6e, 0x09);
	SPI_RW_Reg(WRITEREG + 0x6f, 0xd5);
	//1.2K bps setting end		
	
	SPI_RW_Reg(WRITEREG + 0x30, 0x8c);  //ʹ��PH+ FIFOģʽ����λ��ǰ�棬ʹ��CRCУ��
	SPI_RW_Reg(WRITEREG + 0x32, 0xff);  //byte0, 1,2,3 ��Ϊͷ��
	SPI_RW_Reg(WRITEREG + 0x33, 0x42);  //byte 0,1,2,3 ��ͷ�룬ͬ����3,2 ��ͬ����
	SPI_RW_Reg(WRITEREG + 0x34, 16);    //����16��Nibble��Preamble
	SPI_RW_Reg(WRITEREG + 0x35, 0x20);  //��Ҫ���4��nibble��Preamble
	SPI_RW_Reg(WRITEREG + 0x36, 0x2d);  //ͬ����Ϊ 0x2dd4
	SPI_RW_Reg(WRITEREG + 0x37, 0xd4);
	SPI_RW_Reg(WRITEREG + 0x38, 0x00);
	SPI_RW_Reg(WRITEREG + 0x39, 0x00);
	SPI_RW_Reg(WRITEREG + 0x3a, 'H');   //�����ͷ��Ϊ�� ��HAHA"
	SPI_RW_Reg(WRITEREG + 0x3b, 'A');
	SPI_RW_Reg(WRITEREG + 0x3c, 'H');
	SPI_RW_Reg(WRITEREG + 0x3d, 'A');
	SPI_RW_Reg(WRITEREG + 0x3e, 64);    //�ܹ�����64���ֽڵ�����
	SPI_RW_Reg(WRITEREG + 0x3f, 'H');   //��ҪУ���ͷ��Ϊ����HAHA"
	SPI_RW_Reg(WRITEREG + 0x40, 'A');
	SPI_RW_Reg(WRITEREG + 0x41, 'H');
	SPI_RW_Reg(WRITEREG + 0x42, 'A');
	SPI_RW_Reg(WRITEREG + 0x43, 0xff);  //ͷ��1,2,3,4 ������λ����ҪУ��
	SPI_RW_Reg(WRITEREG + 0x44, 0xff);  // 
	SPI_RW_Reg(WRITEREG + 0x45, 0xff);  // 
	SPI_RW_Reg(WRITEREG + 0x46, 0xff);  // 
//--------------------------------------------------------------------------
//	SPI_RW_Reg(WRITEREG + 0x6d, 0x06);  //���书������  0x00:+0dBM  0x01:+3dBM  0x02:+6dBM  0x03:+9dBM  0x04:+11dBM  0x05:+14dBM  0x06:+17dBM  0x07:20dBM
    SPI_RW_Reg(WRITEREG + 0x6d, 0x01);
	SPI_RW_Reg(WRITEREG + 0x79, 0x0);   //����Ҫ��Ƶ
	SPI_RW_Reg(WRITEREG + 0x7a, 0x0);   //����Ҫ��Ƶ
//	SPI_RW_Reg(WRITEREG + 0x71, 0x22);  //���䲻��ҪCLK��FiFo��FSKģʽ
	SPI_RW_Reg(WRITEREG + 0x71, 0x2B); 	// ���䲻��ҪCLK��FiFo GFSKģʽ
	SPI_RW_Reg(WRITEREG + 0x72, 0x30);  //ƵƫΪ 30KHz
	SPI_RW_Reg(WRITEREG + 0x73, 0x0);   //û��Ƶ��ƫ��
	SPI_RW_Reg(WRITEREG + 0x74, 0x0);   //û��Ƶ��ƫ��
//----------------------------------------------------------------
	SPI_RW_Reg(WRITEREG + 0x75, 0x53);  //Ƶ������ 434
	SPI_RW_Reg(WRITEREG + 0x76, 0x64);  //
	SPI_RW_Reg(WRITEREG + 0x77, 0x00);
	TX0_RX0;	// ���߿��ز��ڷ��䡢����״̬
}


//��Ƶ�ź�ǿ��ָʾ����
u8 RF4432_RSSI(void)
{	
	if(!(NIRQ))
	{
		ItStatus = SPI_RW_Reg(READREG + 0x04,0x00);  //���жϼĴ���
	 	if((ItStatus&0x80)==0x80)
		{
			RSSI = SPI_RW_Reg(READREG + 0x26, 0x00);
		    return 1;
		}
		else
		{
		    return 0;
		}
	}
	else
	{
	return 0;
	}
}

//SI4432�����źŰ�����
//������*rxBuffer ���յ�������Ҫ�洢�ĵ�ַ
//���أ�!=0 ���ճɹ�
//		0 ����ʧ��
u8 RF4432_RxPacket(u8 *rxBuffer)
{	
	if(!(NIRQ))
	{		
		ItStatus = SPI_RW_Reg(READREG + 0x03,0x00);  //���жϼĴ���
	 	if((ItStatus&0x02)==0x02)
		{
		    SPI_Read_Buf(0x7f, rxBuffer, RxBuf_Len);
		    return 1;
		}
		else
		{
		    return 0;
		}
	}
	else
	{
	return 0;
	}
}

//SI4432�����źŰ�����  �Զ����㷢�ͳ���
//������*dataBuffer �����͵����ݴ�ŵĵ�ַ
void RF4432_TxPacket(u8 *dataBuffer)
{
    u8 TX_Timeout;

	SPI_RW_Reg(WRITEREG + 0x34, 40);   //����16��Nibble ��ǰ����
	SPI_RW_Reg(WRITEREG + 0x3e, TxBuf_Len);  //�ܹ�����Tx_Data_Len���ֽڵ�����
	
	SPI_Write_Buf(WRITEREG + 0x7f, dataBuffer, TxBuf_Len);   //��Ҫ���͵�����д��Ĵ���
    SPI_RW_Reg(WRITEREG + 0x07, 0x09); //���뷢��ģʽ	
	
	TX_Timeout = 0;
	while(NIRQ)		//�ȴ��ж�
	{
	   TX_Timeout++;
		if(TX_Timeout>=150)
		{
		    printf("����ʧ��...TX ERROR\n");
		    TX_Timeout=0;
			delay_ms(200);
			
			Si4432_init();
			
			break;		//��ǿ������
		}
		delay_ms(5);
	}

	if(!(NIRQ))
	{
		ItStatus = SPI_RW_Reg(READREG + 0x03,0x00);  //���жϼĴ���
		if((ItStatus&0x04)==0x04)
		{
		  ItStatus=0  ;//Uart_sentstr("���ͳɹ�...TX OK\r\n");
		}
	}
}

//����SI4432Ϊ����ģʽ
void SetRX_Mode(void)
{	
	SPI_RW_Reg(WRITEREG + 0x07, 0x01);	//���� Ready ģʽ
	delay_ms(5);
	TX0_RX1;		//�������߿���
//    TX1_RX0;
    delay_ms(5);

	SPI_RW_Reg(WRITEREG + 0x08, SPI_RW_Reg(READREG + 0x08, 0x00)|0x02); //����FIFO��0
	SPI_RW_Reg(WRITEREG + 0x08, SPI_RW_Reg(READREG + 0x08, 0x00)&0xFD);
	
	SPI_RW_Reg(READREG + 0x03, 0x00);	 //������е��жϱ�־
	SPI_RW_Reg(READREG + 0x04, 0x00);
	
	SPI_RW_Reg(WRITEREG + 0x05, 0x02);  //RFģ���յ��������ݺ󣬲����ж�
	SPI_RW_Reg(WRITEREG + 0x07, 0x05);  //RF ģ��������ģʽ
}

//����SI4432Ϊ����ģʽ
void SetTX_Mode(void)
{
	SPI_RW_Reg(WRITEREG + 0x07, 0x01);	//rfģ�����Readyģʽ
	delay_ms(5);
	TX1_RX0;		//�������߿��صķ���
//    TX0_RX1;
	delay_ms(5);
	
	SPI_RW_Reg(WRITEREG + 0x08, SPI_RW_Reg(READREG + 0x08, 0x00)|0x01); //����FIFO��0
	SPI_RW_Reg(WRITEREG + 0x08, SPI_RW_Reg(READREG + 0x08, 0x00)&0xFE);
	
	SPI_RW_Reg(READREG + 0x03, 0x00);  //������е��жϱ�־
	
	SPI_RW_Reg(WRITEREG + 0x05, 0x04);	//�������ݷ�����󣬲����ж�
	delay_ms(5);
//	TX0_RX0;
}



//SI4432���Գ���	
void test_al(void)
{
	u8 i, chksum;
// 	uchar RSSI_Buf[4];
//  uchar RSSI_Timeout=0;
// 	u8 re_re=0;

 	Si4432_init();    //��ʼ��SS4432
  
	printf("SI4432 ��ʼ���ɹ�\n");
    //Uart_sentstr("�����ɹ�...................START\r\n");

	SetRX_Mode();
	
	while(1)
	{	
		
	/*	if(RF4432_RSSI())  //���н�������ʱ��RSSI��ֵ
		{
		    RSSI_Timeout=0;
		   // Uart_sentstr("�ź�ǿ��...");
			RSSI_Buf[0]=RSSI/100 + 0x30;
	        RSSI_Buf[1]=RSSI%100/10 + 0x30;
	        RSSI_Buf[2]=RSSI%10 + 0x30;
			RSSI_Buf[3]='\0';
			if(RSSI_Buf[0])
			RSSI_Buf[0]=0;
			//Uart_sentstr(RSSI_Buf);
		    //Uart_sentstr("\r\n");
		}
		else if(RSSI_Timeout>200)
		{
		    RSSI_Timeout=0;
		    //Uart_sentstr("�ź�ǿ��...");
			RSSI = SPI_RW_Reg(READREG + 0x26, 0x00);
			RSSI_Buf[0]=RSSI/100 + 0x30;
	        RSSI_Buf[1]=RSSI%100/10 + 0x30;
	        RSSI_Buf[2]=RSSI%10 + 0x30;
			RSSI_Buf[3]='\0';
			//Uart_sentstr(RSSI_Buf);
		    //Uart_sentstr("\r\n");
		}
		RSSI_Timeout++;		*/
		
	
		if(KEY==0)  //��������	KEY_Scan()
		{
// 		    re_re=0;
			printf("��������\n");
			SetTX_Mode();		// ÿ���һ��ʱ�䣬����һ�����ݣ������� Acknowledge �ź�
			RF4432_TxPacket(TxBuf);
		    SetRX_Mode();
			
		}
			
		if(RF4432_RxPacket(RxBuf)!=0)  //�����ݽ��յ� ����1 ���෵��0
		{
		    printf("�յ�����:%d��\n",RxBuf_Len);
		    for(i=0;i<RxBuf_Len;i++)	
			{
				send_byte(RxBuf[i]);
			}
							
			chksum = 0;
			for(i=0;i<RxBuf_Len-1;i++)  //����Checksum
			{
        	    chksum += RxBuf[i];          	 		 	
        	}
			
			if(( chksum == RxBuf[RxBuf_Len-1] )&&( RxBuf[0] == 0x41 ))  //У������
     		{
// 				LED=0;
// 				re_re=1;
				delay_ms(60);
// 		        LED=1;
			    SetRX_Mode();//Uart_sentstr("У��ɹ�...RX OK\r\n\r\n");
     			;//OPEN_RX_OK;	//����ָ����LED
        	}
			else
			{
			    SetRX_Mode();//Uart_sentstr("У��ʧ��...RX ERROR\r\n\r\n");
			}
		}//end if(RF4432_RxPacket())
    }//end while
}	


