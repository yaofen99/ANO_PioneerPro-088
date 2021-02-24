/*
 * @Author: your name
 * @Date: 2021-01-28 02:42:06
 * @LastEditTime: 2021-02-23 21:55:10
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: \ANO_PioneerPro-088\SRC\AnoImu\Ano_Imu_Calibration.h
 */
#ifndef _ANO_IMU_CALIBRATION_H_
#define _ANO_IMU_CALIBRATION_H_


//==引用
#include "include.h"


//==定义
typedef struct
{
	//
	u8 acc_stable;
	u8 gyr_stable;	
	//
	u8 acc_cali_on;
	u8 gyr_cali_on;
	//
}_ano_imu_cali_st;
//==数据声明

enum GRY_CALI_MODE{
	CALI_UNABLE,
	CALI_ENABLE,
	CALI_RESET,
};


extern _ano_imu_cali_st st_imu_cali;

//==函数声明

//static

//public
void AccGyrStableCheck_Services(float dT_s,float acc_cmss_in[3],float gyr_dps_in[3]);
u8 GetGyrAvValue(u8 en,vec3_f gyr_raw_in,vec3_f gyr_av_out);
u8 GetAccAvValue(u8 en,vec3_f acc_raw_in,vec3_f acc_av_out);

	
#endif


