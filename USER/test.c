#include <stm32f10x_lib.h>
#include "sys.h"
#include "usart.h"		
#include "delay.h"	
#include "led.h" 
#include "key.h"
#include "24cxx.h"
#include "timer.h"
#include "oled.h"
#include "si4432.h"
#include "usmart.h"
#include "string.h"
// #include "exti.h"
// #include "wdg.h"
// #include "rtc.h"
// #include "wkup.h"
// #include "adc.h"
// #include "dma.h"
// #include "flash.h"
// #include "touch.h"
// #include "24l01.h"
// #include "mmc_sd.h"
// #include "remote.h"
// #include "ds18b20.h"

//STM32F103C8T6-SI4432_SMT���Գ���

//ʱ�� 2015�� 2��20��
//�����ߣ�л�� �Ϻ������������޹�˾
//SI4432�ͺ�:XL_4432-SMT�����ص������ߣ�
//�װ��·�������Ϻ������������޹�˾ QS_TS1630_CTRL_V1.2_MB��·��

//���������0.96��OLED
//OLED�ͺţ�0.96�� ����SPI�ӿڣ�ģ��SPI��  �߽�����

//ʵ�ֹ��ܣ��״�ʵ��SI4432������ͨ�Ź��ܣ��ϵ��ģ���Ϊ����״̬����PA8
//���������͵�ƽ��ģ�����ⷢ�����ݣ�ֻ�ǻ���ͨ�Ź��ܵ�ʵ�֣������ڵ�ַ��
//��ͨ��Э�顣

 //����Ŀ�أ�
 //1.���ô���1���ⲿ����ͨ�ţ��ɴ���������ⳤ������;
 //2.�����պ���������������͸�ָ����ַ��si4422ģ��;
 //3.����ģ���յ����ݺ��������Ľ���Ч����ͨ������1��DMA������
 //��������SI4432�ĵ�Ե㴮��͸��ģ��
 
//���ô���1��PCͨ��
//2��27�� ��ͨ����1���������ݳ��Ƚ���
//2��27�� ��ͨ����1��DMA����
//2��27�� �״���ɴ���͸����û�е�ַ��
/*SI4432����������Ϣ
GND------------>GND
SDN------------>PB11
NIRQ----------->PB10
NSEL----------->PB12
SCLK----------->PB13(SPI2)
SDI------------>PB15(SPI2)
SDO------------>PB14(SPI2)
VCC------------>VCC3.3(1.8~3.6v)

*/

/*OLED������Ϣ
GND------------>GND
VCC------------>VCC3.3
D0(SCLK) ------>PB0
D1(SDIN) ------>PB1
RST------------>PB14
DC------------->PC8
CS------------->PC9
*/




int main(void)
{		
	u8 FLAG=0;
  	Stm32_Clock_Init(9);//ϵͳʱ������
	delay_init(72);		//��ʱ��ʼ��
	USART1_Init(72,115200); //����1��ʼ�� 
	KEY_Init();
	LED_Init();         //LED��ʼ��	
	Si4432_init();
	SetRX_Mode();
	
	while(1)
	{

		if((USART_RX_STA&0x80)==0x80)//�����������յ� ==�����ȼ�����&�����ȼ�
		{
			FLAG=USART_RX_STA;
// 			printf("�Ѿ����յ�����%d��\n",(u8)(USART_RX_STA&0X3F));
// 			Usart1_Printf((char *)USART_RX_BUF);//���յ������ݷ��ظ�����
			memcpy(&TxBuf[3],USART_RX_BUF ,USART_RX_STA&0X3F);//�������յ������ݷ���SI4432���ͻ�������
			memset(USART_RX_BUF,0,USART_RX_STA&0X3F);
			USART_RX_STA=0;			//���������ݶ����� ���ڱ�־λ����
			
			
// 			printf("ͨ��SI4432��������\n");
			TxBuf[0]=DestID;//Ŀ�ĵ�ַ
			TxBuf[1]=MyID;//ת��Դ��ַ
			TxBuf[2]=FLAG&0X3F;///װ��Ҫ���͵����ݳ���
			SetTX_Mode();		// 
			RF4432_TxPacket(TxBuf);//��SI4432���ͻ������е����ݷ��ͳ�ȥ
			memset(TxBuf,0x00,TxBuf_Len);
		    SetRX_Mode();
			
		}//end if((USART_RX_STA&0x80)==0x80)
		
		if(RF4432_RxPacket(RxBuf)!=0)  //�����ݽ��յ� ����1 ���෵��0
		{
// 		    printf("�յ�����:%d��\n",RxBuf[2]);
		    Usart1_Printf((char *)&RxBuf[3]);//���յ�����Ч���ݷ��͵�����
			memset(RxBuf,0x00,RxBuf_Len);
			SetRX_Mode();				
		}//end if(RF4432_RxPacket())
	}	
}


				 
