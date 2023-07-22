#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "timer.h"
#include "math.h"
#include "bsp_adc.h"
#include "usart3.h"	  
#include "AD9959.H"

/**
*@ADC    PC1-PC4
*@usart3 PB10-PB11
*@AD9959 CS------PA6           SDIO0----PA5
*		  	 SCLK----PB1           SDIO1----PA4
*		 		 UPDATE--PB0           SDIO2----PA3
*				 PS0-----PA7           SDIO3----PA8
*				 PS1-----PA2		  		 PDC------PA9
*				 PS2-----PB9		  		 RST------PA10
*				 PS3-----PC0
*@�̵���  
*					PA0  �ӱ����·����
*					PA1  �ӱ����·���
*         PC13  AD637 �������/���
*��Чֵ*2*����2=���ֵ
*/

//���ڽ�������
unsigned char  buf[64];
//��������������ģʽ
extern	u8 Mode_MeasureFlag;
extern  u8 Mode_CorrectFlag; 
extern	u8 Mode_MeasureFlag;
extern	u8 Mode_StartCorrectFlag;
extern  u8 MeasureRi_Flag; 
extern  u8 MeasureRo_Flag; 
extern  u8 MeasureAv_Flag; 
extern  u8 Mode_AmplitudeFlag;
// ADC1ת���ĵ�ѹֵͨ��MDA��ʽ����SRAM
extern __IO uint16_t ADC_ConvertedValue[NOFCHANEL];

// �ֲ����������ڱ���ת�������ĵ�ѹֵ 	 
float ADC_ConvertedValueLocal[NOFCHANEL];        
float ADC_Temp1,ADC_Temp2;
float Ri,Ro,Av;
float Vo=0;
float Vout=0,Vs=0;
extern u8 count[356];
void Judge_Change(void);
// �����ʱ
void Delay(__IO uint32_t nCount)
{
  for(; nCount != 0; nCount--);
} 
u16 averageFilter(float ADC);
u8 a=0;

int main(void)
{		
 
	delay_init();	    																						//��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);								//4����ռ,4����Ӧ��
	uart_init(115200);	 																				  //���ڳ�ʼ��Ϊ115200
	LED_Init();			    														  						//LED�˿ڳ�ʼ��
	JDQ_Init();																										//�̵���
	uart3_init(9600);    																					//���ڳ�ʼ��Ϊ9600
	ADCx_Init();																									//ADC��ʼ��
	Init_AD9959();																								//AD9959��ʼ��
	//TIM3_Int_Init(4999,7199);																	//10Khz�ļ���Ƶ�ʣ�������5000Ϊ500ms  
	setfrequency(1000);
  while(1)
	{     
		if(Mode_MeasureFlag==1)
		{ 
			/**********************************************����Ri**********************************************/	
			if(MeasureRi_Flag==1)
			{
				Key3=0;	
				Key1=0;
				delay_ms(1000);			
				ADC_Temp1 =(float) ADC_ConvertedValue[1]/4096*3.3;
				//ADC_Temp1=averageFilter(ADC_Temp1);
				printf("\r\n ADC_Temp1  = %f  \r\n",ADC_Temp1);	
				Key1=1; 	
				delay_ms(5000);	
				//delay_ms(7000);	
								
				ADC_Temp2 =(float) ADC_ConvertedValue[1]/4096*3.3;
				//ADC_Temp2=averageFilter(ADC_Temp2);
				printf("\r\n ADC_Temp2  = %f  \r\n",ADC_Temp2);

				Ri=(ADC_Temp2*1000)/(ADC_Temp1-ADC_Temp2);
				if(Ri<0)Ri=0;

				printf("\r\n Ri  = %f  \r\n",Ri);
				sprintf((char *)buf,"page1.t2.txt=\"%.1f\"",Ri); //ǿ������ת����ת��Ϊ�ַ���
				HMISends((char *)buf); //����Ri�����ݸ�page0ҳ���t3�ı��ؼ�
				HMISendb(0xff);//������
				
				Key1=0;
				MeasureRi_Flag=0;
			}
			/*********************����Ro*************************/	
			if(MeasureRo_Flag==1)
			{
				Key1=1;
				Key3=1;	
				delay_ms(5000);	
				Key2=0;
					
				ADC_Temp1 =(float) ADC_ConvertedValue[1]/4096*3.3;
				printf("\r\n ADC_Temp1  = %f  \r\n",ADC_Temp1);	
				Key2=1;  
				delay_ms(5000);	
								
				ADC_Temp2 =(float) ADC_ConvertedValue[1]/4096*3.3;
				printf("\r\n ADC_Temp2  = %f  \r\n",ADC_Temp2);

				Ro=((ADC_Temp1-ADC_Temp2)*5100)/ADC_Temp2;
				if(Ro<0)Ro=0;
				printf("\r\n Ro  = %f  \r\n",Ro);
				sprintf((char *)buf,"page1.t4.txt=\"%.1f\"",Ro);
				HMISends((char *)buf);
				HMISendb(0xff);

				Key2=0;
				Key3=0;	
				MeasureRo_Flag=0;
			}

			/*********************����Av*************************/	
			if(MeasureAv_Flag==1)
			{
				Key1=1;
				Key3=0;	
				delay_ms(5000);	
				ADC_Temp1 =(float) ADC_ConvertedValue[1]/4096*3.3;
				printf("\r\n ADC_Temp1  = %f  \r\n",ADC_Temp1);	
				Key3=1;  
				delay_ms(5000);	
				//delay_ms(7000);	
							
				ADC_Temp2 =(float) ADC_ConvertedValue[1]/4096*3.3;
				printf("\r\n ADC_Temp2  = %f  \r\n",ADC_Temp2);
				
				Av=(ADC_Temp2/(ADC_Temp1/105.8));
				if(Av<0)Av=0;	
				printf("\r\n Ro  = %f  \r\n",Av);
				sprintf((char *)buf,"page1.t6.txt=\"%.1f\"",Av);
				HMISends((char *)buf);
				HMISendb(0xff);
			 
				Key2=0;
				Key3=0;	
				MeasureAv_Flag=0;
			} 
									 
		}
		/**********************************************��Ƶ����***********************************************/	
		if(Mode_AmplitudeFlag==1){
				u32 frequency,i;
				u8 temp;
				float Temp;
				Key1=1;
				Key2=0;
				Key3=1;	
				Vo=(float) ADC_ConvertedValue[1]/4096*3.3;
				for(i=0;i<=360;i++){
						if(Mode_AmplitudeFlag==0){
								frequency=1000;
								setfrequency(frequency);
								break;
						}
						if(i<=10){
								frequency=i*100;
								setfrequency(frequency);
						}else if(i>10&&i<100){
								frequency=i*1000;
								setfrequency(i*1000);
						}else{
								frequency=100000+100*(i-100);
								setfrequency(frequency);
						}
						delay_ms(10);	
						Temp =(float) ADC_ConvertedValue[1]/4096*3.3;//��ͼ
						sprintf((char *)buf,"page3.t4.txt=\"%.3f\"",Temp);
						HMISends((char *)buf);
						HMISendb(0xff);
						//value = Temp/Vo;//Av	���޽�ֹƵ��
						if(Temp<=0.71*Vo&&Temp>=0.7*Vo){
								sprintf((char *)buf,"page3.t1.txt=\"%d\"",frequency);
								HMISends((char *)buf);
								HMISendb(0xff);
						}
					  //����Temp2ֵ
						//
						Temp=100*Temp;
						temp=Temp;
						sprintf((char *)buf,"add 2,0,%d",temp);//25-175    360
						HMISends((char *)buf);
						HMISendb(0xff);
				}
				Key1=0;
				Key2=0;
				Key3=0;	
				Mode_AmplitudeFlag=0;
				setfrequency(1000);
		}
		/**********************************************���ϼ��***********************************************/	
		if(Mode_CorrectFlag==1){
				if(Mode_StartCorrectFlag==1){
						/*************************�Զ����*****************************/
						/*************************Ri**************************/
						Key3=0;	
						Key1=0;
						delay_ms(1000);			
						ADC_Temp1 =(float) ADC_ConvertedValue[1]/4096*3.3;
						printf("\r\n ADC_Temp1  = %f  \r\n",ADC_Temp1);	
						Key1=1; 	
						delay_ms(5000);						
						ADC_Temp2 =(float) ADC_ConvertedValue[1]/4096*3.3;
						Ri=(ADC_Temp2*1000)/(ADC_Temp1-ADC_Temp2);
						if(Ri<0)Ri=0;
						if(Ri>80000)Ri=80000;
						Key1=0;
						delay_ms(1000);
						/*************************��̬��ѹ*Vout*************************/
						Key1=0;
						Key2=0;
						Key3=0;
						delay_ms(1000);			
						ADC_Temp1 = (float) ADC_ConvertedValue[2]/4096*3.3;
						Vout = ADC_Temp1*4;
						printf("\r\n ADC_Temp1  = %f  \r\n",ADC_Temp1);	
						delay_ms(1000);
						/************************��̬��ѹ*Vs*************************/
						Key1=1;
						Key3=1;
						delay_ms(1000);
						ADC_Temp1 =(float) ADC_ConvertedValue[1]/4096*3.3;
						Vs=ADC_Temp1;
						delay_ms(1000);
						Key1=0;
						Key2=0;
						
						/************** �Զ��������� ��������ԭ�� *********************/
						Judge_Change();
						Mode_StartCorrectFlag=0;   //��������
				}
		}
	}	 
}	 

/************************����Ri �� Vout(DC) �� Vs(AC)*���ϼ��**********************/
void Judge_Change(void)
{
   if((Ri>1700&Ri<5000)&&(Vout>7.5&Vout<8)&&(Vs>0.7)){											//����				
    sprintf((char *)buf,"page2.t2.txt=\"Normal\"");
		HMISends((char *)buf); 
		HMISendb(0xff);
   }else if((Ri>10000&Ri<100000)&&(Vout>9&Vout<12)&&(Vs<0.2)){					//R1open
		sprintf((char *)buf,"page2.t2.txt=\"R1open\"");
		HMISends((char *)buf); 
		HMISendb(0xff);
	 }else if((Ri>2&Ri<500)&&(Vout>4&Vout<5)&&(Vs<0.2)){  									//R2open
		sprintf((char *)buf,"page2.t2.txt=\"R2open\"");
		HMISends((char *)buf); 
		HMISendb(0xff);
	 }else if((Ri>2&Ri<500)&&(Vout>0.1&Vout<0.4)&&(Vs<0.2)){								//R3open
		sprintf((char *)buf,"page2.t2.txt=\"R3open\"");
		HMISends((char *)buf); 
		HMISendb(0xff);
	 }else if((Ri>8000&Ri<15000)&&(Vout>12)&&(Vs<0.2)){											//R4open
		sprintf((char *)buf,"page2.t2.txt=\"R4open\"");
		HMISends((char *)buf); 
		HMISendb(0xff);
	 }else if((Ri<500)&&(Vout>11&Vout<11.5)&&(Vs<0.2)){											//R1short
		sprintf((char *)buf,"page2.t2.txt=\"R1short\"");
		HMISends((char *)buf); 
		HMISendb(0xff);
	 }else if((Ri<200)&&(Vout>11.8)&&(Vs<0.2)){															//R2short
		sprintf((char *)buf,"page2.t2.txt=\"R2short\"");
		HMISends((char *)buf); 
		HMISendb(0xff);
	 }else if((Ri>1800&Ri<5000)&&(Vout>11)&&(Vs<0.2)){											//R3short
		sprintf((char *)buf,"page2.t2.txt=\"R3short\"");
		HMISends((char *)buf); 
		HMISendb(0xff);
	 }else if((Ri>0&Ri<200)&&(Vout>0&Vout<0.1)&&(Vs<0.2)){									//R4short
		sprintf((char *)buf,"page2.t2.txt=\"R4short\"");
		HMISends((char *)buf); 
		HMISendb(0xff);
	 }else if((Ri<10||Ri>50000)&&(Vout>7&Vout<8)&&(Vs<0.2)){								//C1open
		sprintf((char *)buf,"page2.t2.txt=\"C1open\"");
		HMISends((char *)buf); 
		HMISendb(0xff);
	 }else if((Ri>8000&Ri<20000)&&(Vout>6&Vout<9)&&(Vs<0.2)){								//C2open
		sprintf((char *)buf,"page2.t2.txt=\"C2open\"");
		HMISends((char *)buf); 
		HMISendb(0xff);
	 }else if((Ri>1700&Ri<5000)&&(Vout>7&Vout<7.5)&&(Vs>1)){								//C3open
		sprintf((char *)buf,"page2.t2.txt=\"C3open\"");
		HMISends((char *)buf); 
		HMISendb(0xff);
	 }
}
