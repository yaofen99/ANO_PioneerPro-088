/*
 * @Author: your name
 * @Date: 2021-01-28 02:42:06
 * @LastEditTime: 2021-02-23 21:54:34
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \ANO_PioneerPro-088\SRC\AnoImu\Ano_Imu_Task.c
 */
#include "Ano_Imu_Data.h"
#include "Ano_Imu_Calibration.h"
#include "Ano_Imu_Task.h"
#include "Drv_Bmi088.h"
#include "Drv_spl06.h"
#include "Ano_Parameter.h"

/*IMU循环任务1毫秒*/
void ImuServices_1ms_c()
{
	/*读取陀螺仪加速度计数据*/
	DrvBmi088AccelerationRead();
	//Drv_Icm20602_Read();
	DrvBmi088ReadServices();
	//传入数据
	ImuDataGet((s16 *)st_bmi_data.un_ins.st_data.s16_gyr,(s16 *)st_bmi_data.un_ins.st_data.s16_acc);
	//数据计算
	ImuDataCalcu(Ano_Parame.set.acc_calibrated,(float *)Ano_Parame.set.gyr_zero_offset,(float *)Ano_Parame.set.acc_zero_offset,(float (*)[])Ano_Parame.set.IEM);
	//
	if(flag.unlock==0 )
	{
		//稳定性检测
		AccGyrStableCheck_Services(0.001f,st_imuData.f_acc_cmpss,st_imuData.f_gyr_dps);
		//
		if(st_imu_cali.gyr_cali_on)
		{
			//
			if(GetGyrAvValue(st_imu_cali.gyr_stable,st_imuData.f_gyrRaw,(float *)Ano_Parame.set.gyr_zero_offset))
			{
				if(st_imu_cali.gyr_cali_on==CALI_RESET)
				{
					st_imu_cali.gyr_cali_on = CALI_UNABLE;
				}
				else if(st_imu_cali.gyr_cali_on==CALI_ENABLE)
				{
					st_imu_cali.gyr_cali_on = CALI_UNABLE;
					//存储参数
					data_save();
				}
			}
		}
	}
}
/*IMU循环任务20毫秒*/
void ImuServices_20ms_c()
{
	//获取IMU温度
	ImuTemperatureGet(spl_data.temperature);
	//
}
