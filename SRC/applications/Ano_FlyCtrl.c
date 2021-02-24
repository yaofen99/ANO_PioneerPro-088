#include "Ano_FlyCtrl.h"
#include "Ano_DT.h"
#include "include.h"

_fly_ct_st program_ctrl;
//设计成单一线程执行命令�?
static u16 val, spd;

void FlyCtrlDataAnl(u8 *data)
{


	val = ((*(data+3))<<8) + (*(data+4));
	spd = ((*(data+5))<<8) + (*(data+6));	
	program_ctrl.cmd_state[0] = *(data+2);
}

#include "Ano_FcData.h"
#include "Ano_FlightCtrl.h"
#include "ANO_IMU.h"

static u8 cmd_take_off_f;
void FlyCtrl_Task(u8 dT_ms)
{
	if(program_ctrl.cmd_state[0] != program_ctrl.cmd_state[1])
	{
		//指令更新，复位数�?,停止之前操作
		FlyCtrlReset();
		//
		if(rc_in.no_signal == 0 && flag.flight_mode == LOC_HOLD &&(switchs.of_flow_on || switchs.gps_on  ))
		{
			program_ctrl.state_ok = 1;
		}
		else
		{
			program_ctrl.state_ok = 0;
				//发送字符串
				AnoDTSendStr(USE_HID|USE_U2,SWJ_ADDR,LOG_COLOR_GREEN,"FC State Error!");
				//复位指令状�?
				program_ctrl.cmd_state[0] = 0;
		}

	}
	
	switch(program_ctrl.cmd_state[0])
	{
		case (0x01)://起飞
		{
			
			if(program_ctrl.state_ok != 0)
			{
				//起飞状态为初始状态，且遥控有信号,且光流或者GPS有效
				if(flag.auto_take_off_land == AUTO_TAKE_OFF_NULL)
				{	
					if(cmd_take_off_f == 0)
					{
						//
						cmd_take_off_f = 1;
						//发送字符串
						AnoDTSendStr(USE_HID|USE_U2,SWJ_ADDR,LOG_COLOR_GREEN,"Take off!");
						//一键起�?
						one_key_take_off();
					}

				}
				else if(flag.auto_take_off_land == AUTO_TAKE_OFF_FINISH)
				{
					//发送字符串
					AnoDTSendStr(USE_HID|USE_U2,SWJ_ADDR,LOG_COLOR_GREEN,"Take off OK!");
					//复位指令状�?
					program_ctrl.cmd_state[0] = 0;
				}
				else if(flag.auto_take_off_land > AUTO_TAKE_OFF_FINISH)
				{
					//发送字符串
					AnoDTSendStr(USE_HID|USE_U2,SWJ_ADDR,LOG_COLOR_GREEN,"CMD Error!");
					//复位指令状�?
					program_ctrl.cmd_state[0] = 0;				
				}
			}

		}
		break;
		case (0x02):	//降落
		{
			if(flag.auto_take_off_land == AUTO_TAKE_OFF_FINISH)
			{
				//复位指令状�?
				AnoDTSendStr(USE_HID|USE_U2,SWJ_ADDR,LOG_COLOR_GREEN,"Landing!");
				//一键降�?
				one_key_land();
			}
			else if(flag.auto_take_off_land == AUTO_TAKE_OFF_NULL)
			{
				//发送字符串
				AnoDTSendStr(USE_HID|USE_U2,SWJ_ADDR,LOG_COLOR_GREEN,"Landing OK!");
				//
				program_ctrl.cmd_state[0] = 0;
			}
		}
		break;
		case (0xA0):	//紧急停�?
		{
			if(flag.unlock)
			{
				//
				AnoDTSendStr(USE_HID|USE_U2,SWJ_ADDR,LOG_COLOR_GREEN,"Emergency stop OK!");
				//上锁
				flag.unlock_cmd = 0;
				//
				program_ctrl.cmd_state[0] = 0;
			}
			
		}
		break;
		case (0x03):	//上升
		{	
			//目标速度赋�?
			program_ctrl.vel_cmps_ref[Z] = spd;
			//目标时间赋�?
			if(spd != 0)
			{
				program_ctrl.exp_process_t_ms[Z] = val*1000/LIMIT(spd,0,fc_stv.vel_limit_z_p);
			}
			else
			{
				program_ctrl.exp_process_t_ms[Z] = 0;
			}

			//判断开始和完成
			if(program_ctrl.fb_process_t_ms[Z] == 0)
			{
				//
				AnoDTSendStr(USE_HID|USE_U2,SWJ_ADDR,LOG_COLOR_GREEN,"Go up!");			
			}
			else if(program_ctrl.exp_process_t_ms[Z] < program_ctrl.fb_process_t_ms[Z])
			{
				//
				AnoDTSendStr(USE_HID|USE_U2,SWJ_ADDR,LOG_COLOR_GREEN,"Go up OK!");
				//
				program_ctrl.cmd_state[0] = 0;
			}

			//计时反馈
			program_ctrl.fb_process_t_ms[Z] += dT_ms;
		}
		break;
		case (0x04):	//下降
		{
			//目标速度赋�?
			program_ctrl.vel_cmps_ref[Z] = -spd;
			//目标时间赋�?
			if(spd != 0)
			{
				program_ctrl.exp_process_t_ms[Z] = val*1000/LIMIT(spd,0,-fc_stv.vel_limit_z_n);
			}
			else
			{
				program_ctrl.exp_process_t_ms[Z] = 0;
			}

			//判断开始和完成
			if(program_ctrl.fb_process_t_ms[Z] == 0)
			{
				//
				AnoDTSendStr(USE_HID|USE_U2,SWJ_ADDR,LOG_COLOR_GREEN,"Go down!");				
			}
			else if(program_ctrl.exp_process_t_ms[Z] < program_ctrl.fb_process_t_ms[Z])
			{
				//
				AnoDTSendStr(USE_HID|USE_U2,SWJ_ADDR,LOG_COLOR_GREEN,"Go down OK!");
				//
				program_ctrl.cmd_state[0] = 0;
			}	

			//计时反馈
			program_ctrl.fb_process_t_ms[Z] += dT_ms;
		}
		break;
		case (0x05):	//前进
		{
			//目标速度赋�?
			program_ctrl.vel_cmps_ref[X] = spd;
			//目标时间赋�?
			if(spd != 0)
			{
				program_ctrl.exp_process_t_ms[X] = val*1000/LIMIT(spd,0,fc_stv.vel_limit_xy);
			}
			else
			{
				program_ctrl.exp_process_t_ms[X] = 0;
			}

			//判断开始和完成
			if(program_ctrl.fb_process_t_ms[X] == 0)
			{
				//
				AnoDTSendStr(USE_HID|USE_U2,SWJ_ADDR,LOG_COLOR_GREEN,"Go ahead!");				
			}
			else if(program_ctrl.exp_process_t_ms[X] < program_ctrl.fb_process_t_ms[X])
			{
				//
				AnoDTSendStr(USE_HID|USE_U2,SWJ_ADDR,LOG_COLOR_GREEN,"Go ahead OK!");
				//
				program_ctrl.cmd_state[0] = 0;
			}

			//计时反馈
			program_ctrl.fb_process_t_ms[X] += dT_ms;
		}
		break;
		case (0x06):	//后退
		{
			//目标速度赋�?
			program_ctrl.vel_cmps_ref[X] = -spd;
			//目标时间赋�?
			if(spd != 0)
			{
				program_ctrl.exp_process_t_ms[X] = val*1000/LIMIT(spd,0,fc_stv.vel_limit_xy);
			}
			else
			{
				program_ctrl.exp_process_t_ms[X] = 0;
			}

			//判断开始和完成
			if(program_ctrl.fb_process_t_ms[X] == 0)
			{
				//
				AnoDTSendStr(USE_HID|USE_U2,SWJ_ADDR,LOG_COLOR_GREEN,"Go back!");				
			}
			else if(program_ctrl.exp_process_t_ms[X] < program_ctrl.fb_process_t_ms[X])
			{
				//
				AnoDTSendStr(USE_HID|USE_U2,SWJ_ADDR,LOG_COLOR_GREEN,"Go back OK!");
				//
				program_ctrl.cmd_state[0] = 0;
			}

			//计时反馈
			program_ctrl.fb_process_t_ms[X] += dT_ms;
		}
		break;
		case (0x07):	//向左
		{
			//目标速度赋�?
			program_ctrl.vel_cmps_ref[Y] = spd;
			//目标时间赋�?
			if(spd != 0)
			{
				program_ctrl.exp_process_t_ms[Y] = val*1000/LIMIT(spd,0,fc_stv.vel_limit_xy);
			}
			else
			{
				program_ctrl.exp_process_t_ms[Y] = 0;
			}

			//判断开始和完成
			if(program_ctrl.fb_process_t_ms[Y] == 0)
			{
				//
				AnoDTSendStr(USE_HID|USE_U2,SWJ_ADDR,LOG_COLOR_GREEN,"Go left!");				
			}
			else if(program_ctrl.exp_process_t_ms[Y] < program_ctrl.fb_process_t_ms[Y])
			{
				//
				AnoDTSendStr(USE_HID|USE_U2,SWJ_ADDR,LOG_COLOR_GREEN,"Go left OK!");
				//
				program_ctrl.cmd_state[0] = 0;
			}

			//计时反馈
			program_ctrl.fb_process_t_ms[Y] += dT_ms;
		}
		break;
		case (0x08):	//向右
		{
			//目标速度赋�?
			program_ctrl.vel_cmps_ref[Y] = -spd;
			//目标时间赋�?
			if(spd != 0)
			{
				program_ctrl.exp_process_t_ms[Y] = val*1000/LIMIT(spd,0,fc_stv.vel_limit_xy);
			}
			else
			{
				program_ctrl.exp_process_t_ms[Y] = 0;
			}

			//判断开始和完成
			if(program_ctrl.fb_process_t_ms[Y] == 0)
			{
				//
				AnoDTSendStr(USE_HID|USE_U2,SWJ_ADDR,LOG_COLOR_GREEN,"Go right!");				
			}
			else if(program_ctrl.exp_process_t_ms[Y] < program_ctrl.fb_process_t_ms[Y])
			{
				//
				AnoDTSendStr(USE_HID|USE_U2,SWJ_ADDR,LOG_COLOR_GREEN,"Go right OK!");
				//
				program_ctrl.cmd_state[0] = 0;
			}

			//计时反馈
			program_ctrl.fb_process_t_ms[Y] += dT_ms;
		}
		break;
		case (0x09):	//左转
		{
			//目标速度赋�?
			program_ctrl.yaw_pal_dps = spd;
			//目标时间赋�?
			if(spd != 0)
			{
				program_ctrl.exp_process_t_ms[3] = val*1000/LIMIT(spd,0,fc_stv.yaw_pal_limit);
			}
			else
			{
				program_ctrl.exp_process_t_ms[3] = 0;
			}

			//判断开始和完成
			if(program_ctrl.fb_process_t_ms[3] == 0)
			{
				//
				AnoDTSendStr(USE_HID|USE_U2,SWJ_ADDR,LOG_COLOR_GREEN,"Turn left!");				
			}
			else if(program_ctrl.exp_process_t_ms[3] < program_ctrl.fb_process_t_ms[3])
			{
				//
				AnoDTSendStr(USE_HID|USE_U2,SWJ_ADDR,LOG_COLOR_GREEN,"Turn left OK!");
				//
				program_ctrl.cmd_state[0] = 0;
			}

			//计时反馈
			program_ctrl.fb_process_t_ms[3] += dT_ms;
		}
		break;
		case (0x0A):	//右转
		{
			//目标速度赋�?
			program_ctrl.yaw_pal_dps = -spd;
			//目标时间赋�?
			if(spd != 0)
			{
				program_ctrl.exp_process_t_ms[3] = val*1000/LIMIT(spd,0,fc_stv.yaw_pal_limit);
			}
			else
			{
				program_ctrl.exp_process_t_ms[3] = 0;
			}

			//判断开始和完成
			if(program_ctrl.fb_process_t_ms[3] == 0)
			{
				//
				AnoDTSendStr(USE_HID|USE_U2,SWJ_ADDR,LOG_COLOR_GREEN,"Turn right!");				
			}
			else if(program_ctrl.exp_process_t_ms[3] < program_ctrl.fb_process_t_ms[3])
			{
				//
				AnoDTSendStr(USE_HID|USE_U2,SWJ_ADDR,LOG_COLOR_GREEN,"Turn right OK!");
				//
				program_ctrl.cmd_state[0] = 0;
			}
			
			//计时反馈
			program_ctrl.fb_process_t_ms[3] += dT_ms;
		}
		break;
		default:
		{
		
		}
		break;
	}
	
	//复位操作
	if(program_ctrl.cmd_state[0] == 0)
	{
		FlyCtrlReset();
	}
	//记录历史�?
	program_ctrl.cmd_state[1] = program_ctrl.cmd_state[0];
	
	//数据处理坐标转换�?,以解锁时候机头指向为参�?
	if(flag.unlock !=0)
	{
		//参考方向转世界坐标（本飞控为地理坐标）
		h2w_2d_trans(program_ctrl.vel_cmps_ref,program_ctrl.ref_dir,program_ctrl.vel_cmps_w);
		//世界坐标（本飞控为地理坐标）转水平航向坐标�?
		w2h_2d_trans(program_ctrl.vel_cmps_w,imu_data.hx_vec,program_ctrl.vel_cmps_h);
		//水平方向变化，Z不变
		program_ctrl.vel_cmps_h[Z] = program_ctrl.vel_cmps_w[Z] = program_ctrl.vel_cmps_ref[Z];
	}
	else
	{
		//记录机头指向为参考方�?
		program_ctrl.ref_dir[X] = imu_data.hx_vec[X];
		program_ctrl.ref_dir[Y] = imu_data.hx_vec[Y];
		//

	}
}

void FlyCtrlReset()
{
	//
	cmd_take_off_f = 0;
	//
	for(u8 i = 0;i<4;i++)
	{
		if(i<3)
		{
			//
			program_ctrl.vel_cmps_ref[i] = 0;
			program_ctrl.vel_cmps_w[i] = 0;
			program_ctrl.vel_cmps_h[i] = 0;
		}
		else
		{
			program_ctrl.yaw_pal_dps = 0;
		}
		
		program_ctrl.exp_process_t_ms[i] = 0;
		program_ctrl.fb_process_t_ms[i] = 0;
	}
	
}
