/*-----------------------------------------------------------------------*/
/* Low level disk I/O module for FatFs                                   */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "sdcard_spi.h" /* SD Card - SPI support */

/* Definitions of physical drive number for each media */
#define SD_SPI_DISK		0

// Private variables
SD_CardInfo SDCardInfo;

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber (0..) */
)
{
	DSTATUS stat;
	SD_Error status;

	switch (pdrv) {
	case SD_SPI_DISK :
		status = SD_Init();

		if (status == SD_RESPONSE_NO_ERROR)
		{
			/* Read CSD/CID MSD registers */
			status = SD_GetCardInfo(&SDCardInfo);
		}

		if (status == SD_RESPONSE_NO_ERROR)
			stat = 0x00;
		else
			stat = STA_NOINIT;

		return stat;
	}

	return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber (0..) */
)
{
	DSTATUS stat;
	SD_Error status;

	switch (pdrv) {
	case SD_SPI_DISK :
		status = SD_GetCardInfo(&SDCardInfo);

		if (status == SD_RESPONSE_NO_ERROR)
			stat = 0x00;
		else
			stat = STA_NOINIT;

		return stat;
	}

	return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	UINT count		/* Number of sectors to read (1..128) */
)
{
	DRESULT res;
	SD_Error status;

	switch (pdrv) {
	case SD_SPI_DISK :
		if (count == 1)
			status = SD_ReadBlock((uint8_t*)buff, sector << 9, 512);
		else
			status = SD_ReadMultiBlocks((uint8_t*)buff, sector << 9, 512, count);

		if(status == SD_RESPONSE_NO_ERROR)
			res = RES_OK;
		else
			res = RES_ERROR;

		return res;
	}

	return RES_PARERR;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	UINT count			/* Number of sectors to write (1..128) */
)
{
	DRESULT res;
	SD_Error status;

	switch (pdrv) {
	case SD_SPI_DISK :
		if (count == 1)
			status = SD_WriteBlock((uint8_t*)buff, sector << 9, 512);
		else
			status = SD_WriteMultiBlocks((uint8_t*)buff, sector << 9, 512, count);

		if(status == SD_RESPONSE_NO_ERROR)
			res = RES_OK;
		else
			res = RES_ERROR;

		return res;
	}

	return RES_PARERR;
}
#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;

	switch (pdrv) {
	case SD_SPI_DISK :
		switch (cmd) {
		case CTRL_SYNC :            /* Make sure that no pending write process */
			// no synchronization to do
			res = RES_OK;
			break;

		case GET_SECTOR_SIZE :      /* Get R/W sector size (WORD) */
			*(WORD*)buff = 512;
			res = RES_OK;
			break;

		case GET_SECTOR_COUNT :     /* Get number of sectors on the disk (DWORD) */
			*(DWORD*)buff = SDCardInfo.CardCapacity / 512;
			res = RES_OK;
			break;

		case GET_BLOCK_SIZE :       /* Get erase block size in unit of sector (DWORD) */
			*(DWORD*)buff = 512;
			res = RES_OK;
			break;
		}

		return res;
	}

	return RES_PARERR;
}
#endif
