/*
**********************************************************************************************************************
*											        eGon
*						           the Embedded GO-ON Bootloader System
*									       eGON arm boot sub-system
*
*						  Copyright(C), 2006-2010, SoftWinners Microelectronic Co., Ltd.
*                                           All Rights Reserved
*
* File    :
*
* By      : Jerry
*
* Version : V2.00
*
* Date	  :
*
* Descript:
**********************************************************************************************************************
*/
#include "egon2.h"
#include "types.h"
#include "osal_de.h"

void OSAL_InterruptDisable(__u32 IrqNo)
{
    wBoot_DisableInt(IrqNo);
    return;
}

void OSAL_InterruptEnable(__u32 IrqNo)
{
    wBoot_EnableInt(IrqNo);
    return;
}

int OSAL_RegISR(__u32 IrqNo,
				__u32 Flags,
				ISRCallback Handler,
				void *pArg,
				__u32 DataSize,
				__u32 Prio)
{
    return wBoot_InsINT_Func(IrqNo, (int *)Handler, pArg);
}


void OSAL_UnRegISR(__u32 IrqNo, ISRCallback Handler, void *pArg)
{
    wBoot_UnsInt_Func(IrqNo);
    return;
}

void *OSAL_PAtoVA(void * pa)
{
	return (void *)pa;
}

void *OSAL_VAtoPA(void * va)
{
	if((unsigned int)(va) > 0x40000000)
        return (void*)((unsigned int)(va) - 0x40000000);

    return (void *)(va);
}

void * OSAL_malloc(__u32 Size)
{
	return wBoot_malloc(Size);
}

void OSAL_free(void * pAddr)
{
	wBoot_free(pAddr);
}


void * OSAL_PhyAlloc(__u32 Size)
{
	return wBoot_malloc(Size);
}

void OSAL_PhyFree(void *pAddr, __u32 Size)
{
    wBoot_free(pAddr);
}


/*
*******************************************************************************
*                     OSAL_IrqLock
*
* Description:
*               ��IRQ�ж����֡������ص�ǰcpsr��״̬��
*
* Parameters:
*    cpu_sr  : ȡ�õ�ǰcpsr��״̬��
*
* Return value:
*           �ޡ�
*
* note:
*    void
*
*******************************************************************************
*/
void OSAL_DE_IrqLock(__u32 *cpu_sr)
{

}

/*
*******************************************************************************
*                     OSAL_IrqUnLock
*
* Description:
*              �ָ�cpsr��״̬����һ����irqλ��0��
*
* Parameters:
*    cpu_sr  : ֮ǰ��cpsr״̬��
*
* Return value:
*           �ޡ�
*
* note:
*    void
*
*******************************************************************************
*/
void OSAL_DE_IrqUnLock(__u32 cpu_sr)
{

}

/*
*******************************************************************************
*                     OSAL_CacheRangeFlush
*
* Description:
*    Cache����
*
* Parameters:
*    Address    :  Ҫ��ˢ�µ�������ʼ��ַ
*    Length     :  ��ˢ�µĴ�С
*    Flags      :  ˢ�±��λ
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void OSAL_CacheRangeFlush(void *Address, __u32 Length, __u32 Flags)
{
    if(Address == NULL)
    {
        //__wrn("Address is NULL!\n");
        return;
    }
    if(Length == 0)
    {
        //__wrn("Length is ZERO!\n");
        return;
    }

    switch(Flags)
    {
    case CACHE_FLUSH_I_CACHE_REGION:
        wlibc_FlushICacheRegion(Address, Length);
        break;

    case CACHE_FLUSH_D_CACHE_REGION:
        wlibc_FlushDCacheRegion(Address, Length);
        break;

    case CACHE_FLUSH_CACHE_REGION:
        wlibc_FlushCacheRegion(Address, Length);
        break;

    case CACHE_CLEAN_D_CACHE_REGION:
        wlibc_CleanDCacheRegion(Address, Length);
        break;

    case CACHE_CLEAN_FLUSH_D_CACHE_REGION:
        wlibc_CleanFlushDCacheRegion(Address, Length);
        break;

    case CACHE_CLEAN_FLUSH_CACHE_REGION:
        wlibc_CleanFlushCacheRegion(Address, Length);
        break;

    default:
        __wrn("Flags is illegal!");
        break;
    }
    return;
}


int OSAL_sw_get_ic_ver(void)
{
	__u32 reg_val;

	reg_val = *((volatile __u32 *)(0x1c20c00 + 0x13C));
	reg_val = (reg_val >> 6) & 0x03;
	if(reg_val == 0)
	{
		return 0xA;
	}
	else
	{
		return 0xB;
	}
}

