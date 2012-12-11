/*
 * (C) Copyright 2012
 *     wangflord@allwinnerstech.com
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 */
#include "include.h"
#include "axp.h"
#include "axp221.h"

int power_step_level = BATTERY_RATIO_DEFAULT;
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int axp_probe(void)
{
	u8    pmu_type;

	if(axp_i2c_read(AXP20_ADDR, BOOT_POWER20_VERSION, &pmu_type))
	{
		eGon2_printf("axp read error\n");

		return -1;
	}
	pmu_type &= 0x0f;
	if(pmu_type & 0x06)
	{
		/* pmu type AXP221 */
		eGon2_printf("PMU: AXP221\n");

		return 0;
	}

	eGon2_printf("PMU: NULL\n");

	return -1;
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int power_init(int set_vol)
{
	int ret = -1;
	int dcdc3_vol;

	ret = eGon2_script_parser_fetch("target", "dcdc3_vol", &dcdc3_vol, 1);
	if(ret)
	{
		dcdc3_vol = 1200;
	}
#ifdef DEBUG
	eGon2_printf("set dcdc3 to %d\n", dcdc3_vol);
#endif
	if(!axp_probe())
	{
		if(!axp_probe_power_supply_condition())
		{
			if(!axp_set_dcdc3(dcdc3_vol))
			{
				eGon2_printf("axp_set_dcdc3 ok\n");
				ret = 0;
			}
			else
			{
				eGon2_printf("axp_set_dcdc3 fail\n");
			}
		}
		else
		{
			eGon2_printf("axp_probe_power_supply_condition error\n");
		}
	}
	else
	{
		eGon2_printf("axp_probe error\n");
	}

	return ret;
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int axp_probe_power_supply_condition(void)
{
	int   dcin_exist, bat_vol;
	//int   buffer_value;
	//char  bat_value, bat_cou;
	int   ratio;

	//����ѹ�������Ƿ񿪻�
    dcin_exist = axp221_probe_dcin_exist();
#ifdef DEBUG
    eGon2_printf("dcin_exist = %x\n", dcin_exist);
#endif
    //���ж�����������ϴιػ���¼�ĵ����ٷֱ�<=5%,ͬʱ���ؼ�ֵС��5mAh����ػ�����������ж�
    if(!dcin_exist)
    {
    	bat_vol = axp221_probe_battery_vol();
		eGon2_printf("bat vol = %d\n", bat_vol);
		if(bat_vol < 3400)
		{
			eGon2_printf("bat vol is lower than 3400 and dcin is not exist\n");
			eGon2_printf("we have to close it\n");
		    axp_set_hardware_poweroff_vol();
			axp221_set_power_off();
			for(;;);
		}
    }

	ratio = axp221_probe_battery_ratio();
	eGon2_printf("bat ratio = %d\n", ratio);
	if(ratio < 0)
	{
		return -1;
	}
	else if(ratio <= 1)
	{
		if(dcin_exist)
		{
			//�ⲿ��Դ���ڣ���ص�������
			power_step_level = BATTERY_RATIO_TOO_LOW_WITH_DCIN;
		}
		else
		{
			//�ⲿ��Դ�����ڣ���ص�������
			power_step_level = BATTERY_RATIO_TOO_LOW_WITHOUT_DCIN;
		}
	}
	else
	{
		power_step_level = BATTERY_RATIO_ENOUGH;
	}

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int axp_get_power_vol_level(void)
{
	return power_step_level;
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int axp_probe_startup_cause(void)
{
	int buffer_value;
	int poweron_reason, next_action;
	int ret;

	buffer_value = axp221_probe_last_poweron_status();
	eGon2_printf("axp buffer %x\n", buffer_value);
	if(buffer_value < 0)
	{
		return -1;
	}
    if(buffer_value == 0x0e)		//��ʾǰһ������ϵͳ״̬����һ��Ӧ��Ҳ����ϵͳ
    {
    	eGon2_printf("pre sys mode\n");
    	return -1;
    }
    else if(buffer_value == 0x0f)      //��ʾǰһ������boot standby״̬����һ��ҲӦ�ý���boot standby
	{
		eGon2_printf("pre boot mode\n");
		return 0;
	}
	//��ȡ ����ԭ���ǰ������������߲����ѹ����
	poweron_reason = axp221_probe_poweron_cause();
	if(poweron_reason == AXP_POWER_ON_BY_POWER_KEY)
	{
		eGon2_printf("key trigger\n");
		next_action = 0x0e;
		ret = 1;
	}
	else if(poweron_reason == AXP_POWER_ON_BY_POWER_TRIGGER)
	{
		eGon2_printf("power trigger\n");
		next_action = 0x0f;
    	ret = 0;
	}
	//�ѿ���ԭ��д��Ĵ���
	axp221_set_next_poweron_status(next_action);

    return ret;
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int axp_set_hardware_poweron_vol(void) //���ÿ���֮��PMUӲ���ػ���ѹΪ2.9V
{
	int vol_value;

	if(eGon2_script_parser_fetch("pmu_para", "pmu_pwron_vol", &vol_value, 1))
	{
		eGon2_printf("boot power:unable to find power off vol set\n");
		vol_value = 2900;
	}
	return axp221_set_poweroff_vol(vol_value);
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int axp_set_hardware_poweroff_vol(void) //���ùػ�֮��PMUӲ���´ο�����ѹΪ3.3V
{
	int vol_value;

	if(eGon2_script_parser_fetch("pmu_para", "pmu_pwroff_vol", &vol_value, 1))
	{
		eGon2_printf("boot power:unable to find power off vol set\n");
		vol_value = 3300;
	}
	return axp221_set_poweroff_vol(vol_value);
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int  axp_set_power_off(void)
{
	return axp221_set_power_off();
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int axp_set_next_poweron_status(int value)
{
	return axp221_set_next_poweron_status(value);
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int  axp_power_get_dcin_battery_exist(int *dcin_exist, int *battery_exist)
{
	*dcin_exist = axp221_probe_dcin_exist();
	*battery_exist = axp221_probe_battery_exist();

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int  axp_probe_battery_vol(void)
{
	return axp221_probe_battery_vol();
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int  axp_probe_rest_battery_capacity(void)
{
	return axp221_probe_battery_ratio();
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int  axp_probe_key(void)
{
	return axp221_probe_key();
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int axp_set_power_supply_output(void)
{
	int vol_value;

	//set dcdc1
	if(!eGon2_script_parser_fetch("target", "dcdc1_vol", &vol_value, 1))
	{
		if(!axp_set_dcdc1(vol_value))
		{
			eGon2_printf("boot power:set dcdc1 to %d ok\n", vol_value);
		}
	}
	//set dcdc2
	if(!eGon2_script_parser_fetch("target", "dcdc2_vol", &vol_value, 1))
	{
		if(!axp_set_dcdc2(vol_value))
		{
			eGon2_printf("boot power:set dcdc2 to %d ok\n", vol_value);
		}
	}
#ifdef DEBUG
	else
	{
		eGon2_printf("boot power:unable to find dcdc2 set\n");
	}
#endif
	//set dcdc4
	if(!eGon2_script_parser_fetch("target", "dcdc4_vol", &vol_value, 1))
	{
		if(!axp_set_dcdc4(vol_value))
		{
			eGon2_printf("boot power:set dcdc4 to %d ok\n", vol_value);
		}
	}
#ifdef DEBUG
	else
	{
		eGon2_printf("boot power:unable to find dcdc4 set\n");
	}
#endif
    //set dcdc5
	if(!eGon2_script_parser_fetch("target", "dcdc5_vol", &vol_value, 1))
	{
		if(!axp_set_dcdc5(vol_value))
		{
			eGon2_printf("boot power:set dcdc5 to %d ok\n", vol_value);
		}
	}
#ifdef DEBUG
	else
	{
		eGon2_printf("boot power:unable to find dcdc5 set\n");
	}
#endif
	//set ldo2
	if(!eGon2_script_parser_fetch("target", "ldo2_vol", &vol_value, 1))
	{
		if(!axp_set_ldo2(vol_value))
		{
			eGon2_printf("boot power:set ldo2 to %d ok\n", vol_value);
		}
	}
#ifdef DEBUG
	else
	{
		eGon2_printf("boot power:unable to find ldo2 set\n");
	}
#endif
	//set ldo3
	if(!eGon2_script_parser_fetch("target", "ldo3_vol", &vol_value, 1))
	{
		if(!axp_set_ldo3(vol_value))
		{
			eGon2_printf("boot power:set ldo2 to %d ok\n", vol_value);
		}
	}
#ifdef DEBUG
	else
	{
		eGon2_printf("boot power:unable to find ldo3 set\n");
	}
#endif
	//set ldo4
//	if(!eGon2_script_parser_fetch("target", "ldo4_vol", &vol_value, 1))
//	{
//		axp_set_ldo4(vol_value);
//	}
//	else
//	{
//		eGon2_printf("boot power:unable to find ldo4 set\n");
//	}
	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int  axp_set_dc1sw(int on_off)
{
	return axp221_set_dc1sw(on_off);
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int  axp_set_dcdc1(int set_vol)
{
	return axp221_set_dcdc1(set_vol);
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int  axp_set_dcdc2(int set_vol)
{
	return axp221_set_dcdc2(set_vol);
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int  axp_set_dcdc3(int set_vol)
{
	return axp221_set_dcdc3(set_vol);
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int  axp_set_dcdc4(int set_vol)
{
	return axp221_set_dcdc4(set_vol);
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int  axp_set_dcdc5(int set_vol)
{
	return axp221_set_dcdc5(set_vol);
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/

int  axp_set_ldo2(int set_vol)
{
	return axp221_set_ldo2(set_vol);
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int  axp_set_ldo3(int set_vol)
{
	return axp221_set_ldo3(set_vol);
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int  axp_set_ldo4(int set_vol)
{
	return axp221_set_ldo4(set_vol);
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int  axp_probe_charge_current(void)
{
	return axp221_probe_charge_current();
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int  axp_set_charge_current(int current)
{
	return axp221_set_charge_current(current);
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int  axp_set_charge_control(void)
{
	return axp221_set_charge_control();
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int  axp_set_vbus_cur_limit(int current)
{
	return axp221_set_vbus_cur_limit(current);
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int  axp_set_vbus_vol_limit(int vol)
{
	return axp221_set_vbus_vol_limit(vol);
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int axp_set_all_limit(void)
{
	int usbvol_limit = 0;
	int usbcur_limit = 0;
	int limit_vol = 0, limit_cur = 0;

	eGon2_script_parser_fetch("pmu_para", "pmu_usbvol_limit", &usbvol_limit, 1);
	eGon2_script_parser_fetch("pmu_para", "pmu_usbcur_limit", &usbcur_limit, 1);
	eGon2_script_parser_fetch("pmu_para", "pmu_usbvol", &limit_vol, 1);
	eGon2_script_parser_fetch("pmu_para", "pmu_usbcur", &limit_cur, 1);
#ifdef DEBUG
	eGon2_printf("usbvol_limit = %d, limit_vol = %d\n", usbvol_limit, limit_vol);
	eGon2_printf("usbcur_limit = %d, limit_cur = %d\n", usbcur_limit, limit_cur);
#endif
	if(!usbvol_limit)
	{
		limit_vol = 0;

	}
	if(!usbcur_limit)
	{
		limit_cur = 0;

	}

	axp_set_vbus_vol_limit(limit_vol);
	axp_set_vbus_cur_limit(limit_cur);

	return 0;
}
static  __u8  power_int_value[4];
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
int axp_int_store(void)
{
    __u8  reg_addr;
    __u8  int_enable[4];
    int	  i;

    axp221_read_int_enable_status(power_int_value);

	//int_enable[0] = 0x2C;	//������VBUS�Ƴ���ACIN�Ƴ�
	//int_enable[1] = 0;		//������������
	//int_enable[2] = 0x3;	//��������Դ�����̰�������
	int_enable[0] = 0;	//������VBUS�Ƴ���ACIN�Ƴ�
	int_enable[1] = 0;	//������������
	int_enable[2] = 0;	//��������Դ�����̰�������

	axp221_write_int_enable_status(int_enable);

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
__s32 axp_int_restore(void)
{
	axp221_write_int_enable_status(power_int_value);

	return 0;
}

/*
************************************************************************************************************
*
*                                             function
*
*    �������ƣ�
*
*    �����б���
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
__s32 axp_int_query(__u8 *int_status)
{
    axp221_int_query(int_status);

	return 0;
}

