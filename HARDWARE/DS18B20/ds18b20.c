#include "ds18b20.h"
#include "delay.h"	
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//Mini STM32������
//DS18B20 ��������		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2010/6/17 
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
//All rights reserved
//////////////////////////////////////////////////////////////////////////////////	 
u8 RomCode[8]; 
u8 noskip=1;//����ROM��� 1������rom 0����rom

u8 DS18B20_ROM[10][8]={
40,255,110,126,98,20,3,153,//DS18B20�ڵ�1��ROM
40,255,162,112,98,20,3,143,//DS18B20�ڵ�2��ROMֵ
40,255,110,126,98,20,3,153,//DS18B20�ڵ�1��ROM
40,255,162,112,98,20,3,143,//DS18B20�ڵ�2��ROMֵ
40,255,110,126,98,20,3,153,//DS18B20�ڵ�1��ROM
40,255,162,112,98,20,3,143,//DS18B20�ڵ�2��ROMֵ
40,255,110,126,98,20,3,153,//DS18B20�ڵ�1��ROM
40,255,162,112,98,20,3,143,//DS18B20�ڵ�2��ROMֵ
40,255,110,126,98,20,3,153,//DS18B20�ڵ�1��ROM
40,255,162,112,98,20,3,143//DS18B20�ڵ�2��ROMֵ	
};
//��λDS18B20
void DS18B20_Rst(void)	   
{                 
	DS18B20_IO_OUT(); //SET PA0 OUTPUT
    DS18B20_DQ_OUT=0; //����DQ
    delay_us(750);    //����750us
    DS18B20_DQ_OUT=1; //DQ=1 
	delay_us(15);     //15US
}
//�ȴ�DS18B20�Ļ�Ӧ
//����1:δ��⵽DS18B20�Ĵ���
//����0:����
u8 DS18B20_Check(void) 	   
{   
	u8 retry=0;
	DS18B20_IO_IN();//SET PA0 INPUT	 
    while (DS18B20_DQ_IN&&retry<200)
	{
		retry++;
		delay_us(1);
	};	 
	if(retry>=200)return 1;
	else retry=0;
    while (!DS18B20_DQ_IN&&retry<240)
	{
		retry++;
		delay_us(1);
	};
	if(retry>=240)return 1;	    
	return 0;
}
//��DS18B20��ȡһ��λ
//����ֵ��1/0
u8 DS18B20_Read_Bit(void) 			 // read one bit
{
    u8 data;
	DS18B20_IO_OUT();//SET PA0 OUTPUT
    DS18B20_DQ_OUT=0; 
	delay_us(2);
    DS18B20_DQ_OUT=1; 
	DS18B20_IO_IN();//SET PA0 INPUT
	delay_us(12);
	if(DS18B20_DQ_IN)data=1;
    else data=0;	 
    delay_us(50);           
    return data;
}
//��DS18B20��ȡһ���ֽ�
//����ֵ������������
u8 DS18B20_Read_Byte(void)    // read one byte
{        
    u8 i,j,dat;
    dat=0;
	for (i=1;i<=8;i++) 
	{
        j=DS18B20_Read_Bit();
        dat=(j<<7)|(dat>>1);
    }						    
    return dat;
}
//дһ���ֽڵ�DS18B20
//dat��Ҫд����ֽ�
void DS18B20_Write_Byte(u8 dat)     
 {             
    u8 j;
    u8 testb;
	DS18B20_IO_OUT();//SET PA0 OUTPUT;
    for (j=1;j<=8;j++) 
	{
        testb=dat&0x01;
        dat=dat>>1;
        if (testb) 
        {
            DS18B20_DQ_OUT=0;// Write 1
            delay_us(2);                            
            DS18B20_DQ_OUT=1;
            delay_us(60);             
        }
        else 
        {
            DS18B20_DQ_OUT=0;// Write 0
            delay_us(60);             
            DS18B20_DQ_OUT=1;
            delay_us(2);                          
        }
    }
}
//��ʼ�¶�ת��
void DS18B20_Start(void)// ds1820 start convert
{   	
// 	u8 i;
    DS18B20_Rst();	   
	DS18B20_Check();
	
    DS18B20_Write_Byte(0xcc);// skip rom
    DS18B20_Write_Byte(0x44);// convert
} 
//��ʼ��DS18B20��IO�� DQ ͬʱ���DS�Ĵ���
//����1:������
//����0:����    	 
u8 DS18B20_Init(void)
{
	RCC->APB2ENR|=1<<2;    //ʹ��PORTA��ʱ�� 
	GPIOA->CRL&=0XFFFFFFF0;//PORTA.0 �������
	GPIOA->CRL|=0X00000003;
	GPIOA->ODR|=1<<0;      //���1
	DS18B20_Rst();
	return DS18B20_Check();
}  
//��ds18b20�õ��¶�ֵ
//���ȣ�0.1C
//����ֵ���¶�ֵ ��-550~1250�� 
short DS18B20_Get_Temp(u8 Channel)
{
    u8 temp ,i;
    u8 TL,TH;
	short tem;
//     DS18B20_Start ();                    // ds1820 start convert
    DS18B20_Rst();
    DS18B20_Check();
	if(noskip==1)//������
	{
		DS18B20_Write_Byte(0x55);
		for(i=0;i<8;i++)DS18B20_Write_Byte(DS18B20_ROM[Channel][i]); //??64?ROM?
	}
	else
        DS18B20_Write_Byte(0xcc);// skip rom
    DS18B20_Write_Byte(0xbe);// convert	    
    TL=DS18B20_Read_Byte(); // LSB   
    TH=DS18B20_Read_Byte(); // MSB  
	    	  
    if(TH>7)
    {
        TH=~TH;
        TL=~TL; 
        temp=0;//�¶�Ϊ��  
    }else temp=1;//�¶�Ϊ��	  	  
    tem=TH; //��ø߰�λ
    tem<<=8;    
    tem+=TL;//��õװ�λ
    tem=(float)tem*0.625;//ת��     
	if(temp)return tem; //�����¶�ֵ
	else return -tem;    
} 
 
void Read_RomCord(void)//��ȡROM��
{
	u8 j;
	DS18B20_Init();
	DS18B20_Write_Byte(0x33);//��ȡ���������
	for(j=0;j<8;j++)
	{
		RomCode[j] = DS18B20_Read_Byte() ;
	}	
}

u8 crc;
u8 CRC8(void)//CRCУ��
{
	u8 i,x;
	u8 crcbuff;
	crc=0;
	for(x=0;x<8;x++)
	{
		crcbuff=RomCode[x];
		for(i=0;i<8;i++)
		{
			if(((crc^crcbuff)&0x01)==0)
			crc >>= 1; 	
			else { 
              crc ^= 0x18;   //CRC=X8+X5+X4+1
              crc >>= 1; 
              crc |= 0x80; 
            }
			crcbuff >>= 1; 			
		}
	}
	return crc;
	
}

