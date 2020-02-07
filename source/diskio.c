/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */

/* PS3 I/O support */
typedef struct {
    int device;
    void *dirStruct;
} DIR_ITER;

#include "types.h"
#include "iosupport.h"
#include "storage.h"
#include <malloc.h>
#include <sys/file.h>
#include <lv2/mutex.h> 
#include <sys/errno.h>

static u64 ff_ps3id[8] = {
	0x010300000000000AULL, 0x010300000000000BULL, 0x010300000000000CULL, 0x010300000000000DULL,
	0x010300000000000EULL, 0x010300000000000FULL, 0x010300000000001FULL, 0x0103000000000020ULL 
	};
static int dev_fd[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
static int dev_sectsize[8] = {512, 512, 512, 512, 512, 512, 512, 512};
# if 0
DWORD get_fattime (void)
{
	return ((DWORD)(FF_NORTC_YEAR - 1980) << 25 | (DWORD)FF_NORTC_MON << 21 | (DWORD)FF_NORTC_MDAY << 16);
}
#endif
static DSTATUS ps3fatfs_init(int fd)
{
    int rr;
	static device_info_t disc_info;
	disc_info.unknown03 = 0x12345678; // hack for Iris Manager Disc Less
	disc_info.sector_size = 0;
	rr=sys_storage_get_device_info(ff_ps3id[fd], &disc_info);
	if(rr != 0)  
	{
		dev_sectsize[fd] = 512; 
		//return STA_NOINIT;
	}

	dev_sectsize[fd]  = disc_info.sector_size;

	if(dev_fd[fd] >= 0)
		return RES_OK;

	if(sys_storage_open(ff_ps3id[fd], &dev_fd[fd])<0) 
	{
		dev_fd[fd] = -1; 
		return STA_NOINIT;
	}

	dev_sectsize[fd] = disc_info.sector_size;
	return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	if(dev_fd[pdrv] >= 0)
		return 0;
	//
	return STA_NOINIT;
}


/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	return ps3fatfs_init (pdrv);
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res = RES_PARERR;
	int fd = pdrv;
    int flag = ((int) (s64) buff) & 31;

    if(dev_fd[fd] < 0 || !buff) 
		return RES_PARERR;

    void *my_buff;
    
    if(flag) 
		my_buff = memalign(16, dev_sectsize[fd] * count); 
	else 
		my_buff = buff;

    if(!my_buff) 
		return RES_ERROR;

    int r;
    uint32_t sectors_read;

	res = RES_OK;
	r = sys_storage_read(dev_fd[fd], (uint32_t) sector, (uint32_t) count, 
		(uint8_t *) my_buff, &sectors_read); 

	if(r == 0x80010002) 
	{
		return RES_NOTRDY;//PS3_NTFS_Shutdown(fd);
	}

	usleep(62500);

    if(flag) 
	{
		if(r>=0)
			memcpy(buff, my_buff, dev_sectsize[fd] * count);
        free(my_buff);
    }

    if(r < 0) 
		return RES_ERROR;

    if(sectors_read != count) 
		return RES_ERROR;
	return res;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res = RES_PARERR;

	return res;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res = RES_PARERR;

	return res;
}

//#endif
