/*
*********************************************************************************************************
*											        ePDK
*						            the Easy Portable/Player Develop Kits
*									         Decompression For Boot
*
*						        (c) Copyright 2009-2010, Sunny China
*											All	Rights Reserved
*
* File    : DFB_File.c
* By      : sunny
* Version : V2.0
* Date	  : 2009-11-4 10:34:26
*********************************************************************************************************
*/
#include  "file_op.h"

CSzFile       file_t;

static void SET_BUFFER_CLEAR(DecBuffer_t *buffer)
{
    if (buffer)
    {
        buffer->State = DEC_BUFFER_CLEAR;
        buffer->Pos = 0;
    }
}

static void SET_BUFFER_DIRTY(DecBuffer_t *buffer)
{
    if (buffer)
    {
        buffer->State = DEC_BUFFER_DIRTY;
        buffer->Pos = 0;
    }
}

//static int  BUFFER_IS_CLEAR(DecBuffer_t *buffer)
//{
//    return buffer->State == DEC_BUFFER_CLEAR;
//}
//
//static int  BUFFER_IS_DIRTY(DecBuffer_t *buffer)
//{
//    return buffer->State == DEC_BUFFER_DIRTY;
//}

__s32 DFB_LogicRead(UInt32 nSectNum, UInt32 nSectorCnt, void *pBuf)
{
	//return NFB_LogicRead(nSectNum, nSectorCnt, pBuf);
    //return SDMMC_PhyRead(nSectNum, nSectorCnt, pBuf, card_no);
	return wBoot_block_read(nSectNum, nSectorCnt, pBuf);
}

__s32 DFB_LogicWrite(UInt32 nSectNum, UInt32 nSectorCnt, void *pBuf)
{
	//return NFB_LogicWrite(nSectNum, nSectorCnt, pBuf);
    //return SDMMC_PhyWrite(nSectNum, nSectorCnt, pBuf, card_no);
	return wBoot_block_write(nSectNum, nSectorCnt, pBuf);
}

WRes DFB_Part_Read(void *data, __u32 size, CSzFile *p)
{
    UInt64 Offset;
    UInt32 Left;
    UInt32 ReadOut;
    UInt32 len;
    char *pBuffer = NULL;

    /* 先判断是否已达到文件结尾 */
    if (p->CurPos >= p->Size)
    {
        sprite_wrn("Read to file end\n");
        return 0;
    }

    /* 先读取前面未对齐部分 */
    Left = size;
    ReadOut = 0;
    len = 0;
    Offset = p->CurPos & (DEC_SECTOR_SIZE - 1);
    if (Offset)
    {
        /* 直接从buffer里面读取 */
        pBuffer = (char *)(p->Buffer_t.Buffer) + p->Buffer_t.Pos;
        len = size < (DEC_SECTOR_SIZE - Offset) ? size : (DEC_SECTOR_SIZE - Offset);
        if (len > (p->Size - p->CurPos))
        {
            len = p->Size - p->Buffer_t.Pos;
			Left = 0;
        }
        else
		{
			Left -= len;
		}
        memcpy(data, pBuffer, len);
        p->Buffer_t.Pos += len;
        p->CurPos += len;
        ReadOut += len;
    }
    if (Left>>DEC_SECTOR_BITS)
    {
        UInt32 SectorNum = 0;
        UInt32 SectorCnt = 0;

        pBuffer = (char *)data + ReadOut;
        SectorNum = p->CurSec;
        SectorCnt = Left>>DEC_SECTOR_BITS;

        if (SUCCESS != DFB_LogicRead(SectorNum, SectorCnt, pBuffer))
        {
            sprite_wrn("Read %d Logic sector: %d failed\n",
                   SectorCnt, SectorNum);
            return 0;
        }
        p->CurSec += SectorCnt;
        len = Left < DEC_SECTOR_SIZE ? Left : SectorCnt << DEC_SECTOR_BITS;
        if (len >= (p->Size - p->CurPos))
        {
            len = p->Size - p->CurPos;
			Left = 0;
        }
		else
		{
			Left -= len;
		}
        p->CurPos += len;
        ReadOut += len;
    }
    if (Left)
    {
        if (SUCCESS != DFB_LogicRead(p->CurSec, 1, p->Buffer_t.Buffer))
        {
            sprite_wrn("Read %d Logic sector: %d failed\n", 1, p->CurSec);
            return 0;
        }
        len = Left;
        if (len > (p->Size - p->Buffer_t.Pos))
        {
            len = p->Size - p->Buffer_t.Pos;
			Left = 0;
        }
		else
		{
			Left -= len;
		}
		SET_BUFFER_DIRTY(&p->Buffer_t);

        pBuffer = (char *)data + ReadOut;
        memcpy(pBuffer, p->Buffer_t.Buffer, len);
        p->CurPos += len;
        ReadOut += len;
        p->CurSec += 1;
        p->Buffer_t.Pos += len;
    }
    return ReadOut;
}

WRes DFB_Part_Write(const void *data, __u32 size, CSzFile *p)
{
    UInt32 Offset;
    UInt32 Left;
    UInt32 WriteOut;
    UInt32 len;
    char *pBuffer = NULL;

    /* 先写入前面未对齐部分 */
    Left = size;
    WriteOut = 0;
    len = 0;
    Offset = p->CurPos & (DEC_SECTOR_SIZE - 1);
    if (Offset)
    {
        pBuffer = (char *)(p->Buffer_t.Buffer) + p->Buffer_t.Pos;
        len = size < (DEC_SECTOR_SIZE - Offset) ? size : (DEC_SECTOR_SIZE - Offset);

        memcpy(pBuffer, data, len);
        p->Buffer_t.Pos += len;

        /* Buffer中的数据被是否全部填满 */
        if (DEC_SECTOR_SIZE == p->Buffer_t.Pos)
        {
            if (SUCCESS != DFB_LogicWrite(p->CurSec, 1, p->Buffer_t.Buffer))
            {
                sprite_wrn("Write %d Logic sector: %d failed\n", 1, p->CurSec);
                return 0;
            }
            SET_BUFFER_CLEAR(&p->Buffer_t);
            p->CurSec += 1;
        }
        Left -= len;
        p->CurPos += len;
        WriteOut += len;
    }
    if (Left>>DEC_SECTOR_BITS)
    {
        UInt32 SectorNum = 0;
        UInt32 SectorCnt = 0;

        pBuffer = (char *)data + WriteOut;
        SectorNum = p->CurSec;
        SectorCnt = Left>>DEC_SECTOR_BITS;

        if (SUCCESS != DFB_LogicWrite(SectorNum, SectorCnt, pBuffer))
        {
            sprite_wrn("Write %d Logic sector: %d failed\n",
                    SectorCnt, SectorNum);
            return 0;
        }
        p->CurSec += SectorCnt;
        len = Left < DEC_SECTOR_SIZE ? Left : SectorCnt << DEC_SECTOR_BITS;
        Left -= len;
        p->CurPos += len;
        WriteOut += len;
    }
    if (Left)
    {
        len = Left;
        pBuffer = (char *)data + WriteOut;
        memcpy((char *)(p->Buffer_t.Buffer) + p->Buffer_t.Pos, pBuffer, len);
		SET_BUFFER_DIRTY(&p->Buffer_t);
        p->Buffer_t.Pos += len;

        /* Buffer中的数据被是否全部填满 */
        if (DEC_SECTOR_SIZE == p->Buffer_t.Pos)
        {
            if (SUCCESS != DFB_LogicWrite(p->CurSec, 1, p->Buffer_t.Buffer))
            {
                sprite_wrn("Write %d Logic sector: %d failed\n", 1, p->CurSec);
                return 0;
            }
            SET_BUFFER_CLEAR(&p->Buffer_t);
            p->CurSec += 1;
        }
        Left -= len;
        p->CurPos += len;
        WriteOut += len;
    }
    return WriteOut;
}

void File_Construct(CSzFile *p)
{
    p->SectorNr 	     = 0;
    p->CurSec   	     = 0;
    p->CurPos   	     = 0;
    p->Size     	     = 0;
	p->Buffer_t.Buffer   = NULL;
	SET_BUFFER_CLEAR(&p->Buffer_t);
}

CSzFile *File_Open(Sector_t sector, __u32 size_low, __u32 size_high, __u32 writeMode)
{
    CSzFile *p = &file_t;

    p->SectorNr 	     = sector;
    p->CurSec   	     = sector;
    p->CurPos   	     = 0;
    p->Size     	     = size_low | (((__s64)size_high) << 32);
    p->writeMode         = writeMode;
    p->Buffer_t.Buffer   = (__u8 *)sprite_malloc(512 * 1024);
//    p->Buffer_t.Buffer   = //DFB_Malloc(DEC_SECTOR_SIZE);
	SET_BUFFER_CLEAR(&p->Buffer_t);

	return p;
}


WRes File_Close(CSzFile *p)
{
    /* 写模式打开，如果缓冲区为Dirty，写入缓冲区中数据 */
//    if (p->writeMode)
//    {
//        if (BUFFER_IS_DIRTY(&p->Buffer_t))
//        {
//            if (SUCCESS != DFB_LogicWrite(p->CurSec, 1, p->Buffer_t.Buffer))
//            {
//                sprite_wrn("Write %d Logic sector: %d failed\n", 1, p->CurSec);
//                return 0;
//            }
//        }
//    }
	p->SectorNr 	     = 0;
    p->CurSec   	     = 0;
    p->CurPos   	     = 0;
    p->Size     	     = 0;
	wBoot_free(p->Buffer_t.Buffer);
	p->Buffer_t.Buffer = NULL;
	p->Buffer_t.Pos = 0;
    return 0;
}

WRes File_Read(void *data, __u32 size, __u32 size_cnt, CSzFile *p)
{
    __u32 originalSize = size;
    __u32 tmpsize;

    if (originalSize == 0)
        return 0;

    tmpsize = DFB_Part_Read(data, originalSize, p);
    if (tmpsize == originalSize)
    {
        return 0;
    }
    else
    {
        if (p->CurPos == p->Size)
        {
            sprite_wrn("Read to file end....\n");
            return 0;
        }
        else
        {
        	sprite_wrn("Read encount Error....\n");
        	return 1;
        }
    }
}

WRes File_Write(const void *data, __u32 size, __u32 size_cnt, CSzFile *p)
{
    __u32 originalSize = size;
    __u32 tmpsize;

    if (originalSize == 0)
        return 0;

    tmpsize = DFB_Part_Write(data, originalSize, p);
    if (tmpsize == originalSize)
    {
        return 0;
    }
    else
    {
        sprite_wrn("Write encount Error....\n");
        return 1;
    }
}


WRes File_Seek(CSzFile *p, __s64 pos, ESzSeek origin)
{
    Int64 Offset;
    Int64 SeekPos;
    switch (origin)
    {
      case SZ_SEEK_SET: SeekPos = 0; 			break;
      case SZ_SEEK_CUR: SeekPos = p->CurPos; 	break;
      case SZ_SEEK_END: SeekPos = p->Size; 	    break;
      default: return 1;
    }
    /* 只支持在打开文件大小范围内seek */
    if ((p->Size <(SeekPos + pos)) || (0 > (SeekPos + pos)))
    {
        sprite_wrn("Seek overflow\n");
        return 1;
    }
    p->CurPos = SeekPos + pos;
    p->CurSec = p->SectorNr + (Sector_t)(p->CurPos>>DEC_SECTOR_BITS);

    /* 如果Seek后的位置未以一个Sector对齐，则预读一个Sector到Buffer中 */
    Offset = p->CurPos & (DEC_SECTOR_SIZE - 1);
    if (Offset)
    {
        if (SUCCESS != DFB_LogicRead(p->CurSec, 1, p->Buffer_t.Buffer))
        {
            sprite_wrn("Read %d Logic sector: %d failed\n", 1, p->CurSec);
            return 1;
        }
		SET_BUFFER_DIRTY(&p->Buffer_t);
        p->Buffer_t.Pos = Offset;
    }
	else
	{
		SET_BUFFER_CLEAR(&p->Buffer_t);
	}
    return 0;
}
