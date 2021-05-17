/*
 * @Author: your name
 * @Date: 2021-03-03 22:46:35
 * @LastEditTime: 2021-03-03 23:40:37
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: \ANO_PioneerPro-088\SRC\drivers\Drv_ak8975.h
 */
#ifndef _AK8975_H_
#define	_AK8975_H_

#include "stm32f4xx.h"

#define AK8975_WIA_REG          0X00 
#define AK8975_INFO_REG         0X01 
#define AK8975_ST1_REG          0X02 
#define AK8975_HXL_REG          0X03 
#define AK8975_HXH_REG          0X04
#define AK8975_HYL_REG          0X05
#define AK8975_HYH_REG          0X06
#define AK8975_HZL_REG          0X07
#define AK8975_HZH_REG          0X08
#define AK8975_ST2_REG          0X09 
#define AK8975_CNTL_REG         0X0A 
#define AK8975_RSV_REG          0X0B
#define AK8975_ASTC_REG         0X0C 
#define AK8975_TS1_REG          0X0D
#define AK8975_TS2_REG          0X0E
#define AK8975_I2CDIS_REG       0X0F 
#define AK8975_ASAX_REG         0X10 
#define AK8975_ASAY_REG         0X11
#define AK8975_ASAZ_REG         0X12

void Drv_AK8975CSPin_Init(void);
/**
读取电子罗盘磁力计数据
 * @param {*}
 * @return {*}
 */
void Drv_AK8975_Read(void);
void Mag_Get(s16 mag_val[3]);
#endif

