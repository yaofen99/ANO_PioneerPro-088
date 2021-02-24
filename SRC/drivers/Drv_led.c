/******************** (C) COPYRIGHT 2017 ANO Tech ********************************
 * 作�?    ：匿名科�?
 * 官网    ：www.anotc.com
 * 淘宝    ：anotc.taobao.com
 * 技术Q�? �?190169595
 * 描述    ：LED驱动
**********************************************************************************/
#include "Drv_led.h"
#include "Ano_Math.h"
#include "Drv_time.h"


void Drv_LED_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	
	RCC_AHB1PeriphClockCmd(ANO_RCC_LED,ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = ANO_Pin_LED2| ANO_Pin_LED3| ANO_Pin_LED4;
	GPIO_Init(ANO_GPIO_LED, &GPIO_InitStructure);
	
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = ANO_Pin_LED1;
	GPIO_Init(ANO_GPIO_LED, &GPIO_InitStructure);
	
	
	GPIO_ResetBits(ANO_GPIO_LED, ANO_Pin_LED1);		
	GPIO_SetBits(ANO_GPIO_LED, ANO_Pin_LED2);		
	GPIO_ResetBits(ANO_GPIO_LED, ANO_Pin_LED3);		
	GPIO_ResetBits(ANO_GPIO_LED, ANO_Pin_LED4);
}




u16 led_accuracy = 20;//该时间应与LED_Duty()调用周期相同
float LED_Brightness[4] = {0,20,0,0}; //TO 20 //XBRG


//LED�?1ms驱动，在1ms定时中断里调用�?
void LED_1ms_DRV( ) //0~20
{
	static u16 led_cnt[4];
	u8 i;
	
	for(i=0;i<4;i++)
	{
			
		if( led_cnt[i] < LED_Brightness[i] )
		{
			switch(i)
			{
				case 0:	
					LED1_ON;
				break;
				case 1:	
					LED2_ON;
				break;
				case 2:	
					LED3_ON;
				break;
				case 3:	
					LED4_ON;
				break;
			}
		}
		else
		{
			switch(i)
			{
				case 0:	
					LED1_OFF;
				break;
				case 1:	
					LED2_OFF;
				break;
				case 2:	
					LED3_OFF;
				break;
				case 3:	
					LED4_OFF;
				break;
			}
		}
		
		if(++led_cnt[i]>=led_accuracy)
		{
			led_cnt[i] = 0;
		}
	}
	

}

static void ledOnOff(u8 led)
{
	for(u8 i=0; i<LED_NUM; i++)
	{
		if(led & (1<<i))
			LED_Brightness[i] = 20;
		else
			LED_Brightness[i] = 0;
	}
}
static void ledBreath(u8 dT_ms,u8 led,u16 T)
{
	static u8 dir[LED_NUM];
	
	for(u8 i=0; i<LED_NUM; i++)
	{
		if(led & (1<<i))
		{
			switch(dir[i])
			{
				case 0:
					LED_Brightness[i] += safe_div(led_accuracy,((float)T/(dT_ms)),0);
					if(LED_Brightness[i]>20)
					{
						dir[i] = 1;
					}
				
				break;
				case 1:
					LED_Brightness[i] -= safe_div(led_accuracy,((float)T/(dT_ms)),0);
					if(LED_Brightness[i]<0)
					{
						dir[i] = 0;
					}
					
				break;
					
				default:
					dir[i] = 0;
				break;
			}
		}
		else
			LED_Brightness[i] = 0;
	}
}
//					调用周期   LED     亮时�?    灭时�?
static void ledFlash(u8 dT_ms,u8 led, u16 on_ms,u16 off_ms)
{
	static u16 tim_tmp;
	
	if(tim_tmp < on_ms)
		ledOnOff(led);
	else
		ledOnOff(0);
	
	tim_tmp += dT_ms;
	if(tim_tmp >= (on_ms + off_ms))
		tim_tmp = 0;
}

_led_sta LED_STA;
#include "Ano_FcData.h"
#include "Ano_OPMV_CBTracking_Ctrl.h"
#include "Ano_OPMV_LineTracking_Ctrl.h"
void LED_Task2(u8 dT_ms)
{
	static u16 timtmp = 0;

	if(LED_STA.errOneTime)
	{
		ledOnOff(BIT_RLED);
		timtmp += dT_ms;
		if(timtmp > 3000)
		{
			timtmp = 0;
			LED_STA.errOneTime = 0;
		}
	}
	else if(LED_STA.errMpu>0 || LED_STA.errMag>0 || LED_STA.errBaro>0)
	{
		u8 flashtims;
		static u8 cnttmp = 0;
		
		if(LED_STA.errMpu>0)
			flashtims = 2;
		else if(LED_STA.errMag>0)
			flashtims = 3;
		else 
			flashtims = 4;
		
		if(cnttmp < flashtims)
		{
			if(timtmp < 60)
				ledOnOff(BIT_RLED);
			else
				ledOnOff(0);
			timtmp += dT_ms;
			
			if(timtmp > 200)
			{
				timtmp = 0;
				cnttmp++;
			}
		}
		else
		{
			timtmp += dT_ms;
			if(timtmp > 1000)
			{
				timtmp = 0;
				cnttmp = 0;
			}
		}
	}

	else if(LED_STA.saving)
	{
		LED_Brightness[G_led] = 20;
		LED_Brightness[R_led] = 0;
		LED_Brightness[B_led] = 0;
	}
	else if(LED_STA.calAcc || LED_STA.calGyr || LED_STA.rst_imu)
	{
		ledFlash(dT_ms,BIT_WLED,40,40);
	}
	else if(LED_STA.calMag)
	{
		if(LED_STA.calMag == 1)
			ledBreath(dT_ms,BIT_GLED,300);
		else if(LED_STA.calMag == 2)
			ledFlash(dT_ms,BIT_PLED,40,40);
		else if(LED_STA.calMag == 100)
			ledFlash(dT_ms,BIT_RLED,40,40);
		else
			ledBreath(dT_ms,BIT_BLED,300);
			
	}
	else if(LED_STA.noRc)
	{
		ledBreath(dT_ms,BIT_RLED,600);
	}
	else if(LED_STA.lowVt)
	{
		ledFlash(dT_ms,BIT_RLED,100,100);
	}
	else	//无其他提示，正常显示模式档位及外置光流、Gps等状�?
	{
		static u8  statmp = 0;
		static u8  modtmp = 0;
		
		if(statmp == 0)			//显示飞行模式1�?2�?3，白色未解锁，绿色解锁，模式几就闪几�?
		{
			if(modtmp <= flag.flight_mode)
			{
				if(timtmp < 60)
				{
					if(flag.unlock)
						ledOnOff(BIT_GLED);
					else
						ledOnOff(BIT_WLED);
				}
				else
					ledOnOff(0);
				timtmp += dT_ms;
				
				if(timtmp > 200)
				{
					timtmp = 0;
					modtmp++;
				}
			}
			else
			{
				modtmp = 0;
				statmp = 1;
			}
		}
		else if(statmp == 1)	//显示光流、GPS等状�?
		{
			if(modtmp == 0)		//判断GPS是否正常
			{
				if(switchs.gps_on)
				{
					if(timtmp < 60)
						ledOnOff(BIT_BLED);
					else
						ledOnOff(0);
					timtmp += dT_ms;
					if(timtmp > 200)
					{
						timtmp = 0;
						modtmp++;
					}
				}
				else
					modtmp = 1;
			}
			else if(modtmp == 1)	//判断光流是否正常
			{
				if(switchs.of_flow_on>0 && switchs.of_tof_on>0)
				{
					if(timtmp < 60)
						ledOnOff(BIT_PLED);
					else
						ledOnOff(0);
					timtmp += dT_ms;
					if(timtmp > 200)
					{
						timtmp = 0;
						modtmp++;
					}
				}
				else
					modtmp = 2;
			}
			else if(modtmp == 2)		//判断飞控外接ANO_OPMV是否正常
			{
				if(switchs.opmv_on && (ano_opmv_cbt_ctrl.target_loss==0 || ano_opmv_lt_ctrl.target_loss==0) )
				{
					if(timtmp < 60)
						ledOnOff(BIT_YLED);
					else
						ledOnOff(0);
					timtmp += dT_ms;
					if(timtmp > 200)
					{
						timtmp = 0;
						modtmp++;
					}
				}
				else
					modtmp = 3;
			}
			
			if(modtmp == 3)
			{
				statmp = 2;
				modtmp = 0;
			}
		}
		else		//每一轮循环后，加一个灭灯长延时
		{
			ledOnOff(0);
			timtmp += dT_ms;
			if(timtmp > 1000)
			{
				timtmp = 0;
				statmp = 0;
				modtmp = 0;
			}
		}
	}
}

/******************* (C) COPYRIGHT 2016 ANO TECH *****END OF FILE************/

