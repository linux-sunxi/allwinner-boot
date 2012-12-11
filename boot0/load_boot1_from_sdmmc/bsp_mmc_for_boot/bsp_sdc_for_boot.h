/*
********************************************************************************

*           				eBase 
*                 the Abstract of Hardware 
*                     
* 
*              (c) Copyright 2012-2016, ALL WINNER TECH. 
*           								All Rights Reserved 

*
* File    : bsp_sdc_for_boot.h
* By      : Aaron <leafy.myeh@allwinnertech.com>
* Version : V1.00
* Date    : 2012/2/13 18:50 
* Description :  
* Update   :  date      		author         version     notes    
*			 2012/2/13 			Sam.Wu   		V1.0	 create
*			
********************************************************************************
*/
#ifndef _BSP_SDC_FOR_BOOT_H_
#define _BSP_SDC_FOR_BOOT_H_

/*
************************************************************************************************************************
*                       SPMMC_PhyInit
*
*Description: SDMMC������������ʼ��
*
*Arguments  :   none
*
*Return     :   = 0      ok;
*               = -1     fail.
************************************************************************************************************************
*/
__s32 SDMMC_PhyInit(__u32 card_no, __u32 bus_width);
/*
************************************************************************************************************************
*                       SDMMC_PhyExit
*
*Description: SDMMC�����������˳�
*
*Arguments  :   none
*
*Return     :   = 0      ok;
*               = -1     fail.
************************************************************************************************************************
*/
__s32 SDMMC_PhyExit(__u32 card_no);
/*
************************************************************************************************************************
*                       SDMMC_PhyRead
*
*Description: SDMMC������������
*
*Arguments  :   ��׼���豸����
*
*Return     :   = 0      ok;
*               = -1     fail.
************************************************************************************************************************
*/
__s32 SDMMC_PhyRead(__u32 start_sector, __u32 nsector, void *buf, __u32 card_no);
/*
************************************************************************************************************************
*                       SDMMC_PhyWrite
*
*Description: SDMMC������д����
*
*Arguments  :   ��׼���豸����
*
*Return     :   = 0      ok;
*               = -1     fail.
************************************************************************************************************************
*/
__s32 SDMMC_PhyWrite(__u32 start_sector, __u32 nsector, void *buf, __u32 card_no);

__s32 SDMMC_PhyDiskSize(__u32 card_no);

__s32 SDMMC_PhyErase(__u32 block, __u32 nblock, __u32 card_no);



/*
************************************************************************************************************************
*                       SDMMC_PhyInit
*
*Description: SDMMC���߼�������ʼ��
*
*Arguments  :   none
*
*Return     :   = 0      ok;
*               = -1     fail.
************************************************************************************************************************
*/
__s32 SDMMC_LogicalInit(__u32 card_no, __u32 card_offset, __u32 bus_width);
/*
************************************************************************************************************************
*                       SDMMC_LogicalExit
*
*Description: SDMMC���߼�������ʼ��
*
*Arguments  :   none
*
*Return     :   = 0      ok;
*               = -1     fail.
************************************************************************************************************************
*/
__s32 SDMMC_LogicalExit(__u32 card_no);
/*
************************************************************************************************************************
*                       SDMMC_LogicalRead
*
*Description: SDMMC���߼�������
*
*Arguments  :   ��׼���豸����
*
*Return     :   = 0      ok;
*               = -1     fail.
************************************************************************************************************************
*/__s32 SDMMC_LogicalRead(__u32 start_sector, __u32 nsector, void *buf, __u32 card_no);

/*
************************************************************************************************************************
*                       SDMMC_LogicalWrite
*
*Description: SDMMC���߼�д����
*
*Arguments  :   ��׼���豸����
*
*Return     :   = 0      ok;
*               = -1     fail.
************************************************************************************************************************
*/
__s32 SDMMC_LogicalWrite(__u32 start_sector, __u32 nsector, void *buf, __u32 card_no);


__s32 SDMMC_LogicalDiskSize(__u32 card_no);

__s32 SDMMC_LogicaErase(__u32 block, __u32 nblock, __u32 card_no);


#endif //#ifndef _BSP_SDC_FOR_BOOT_H_