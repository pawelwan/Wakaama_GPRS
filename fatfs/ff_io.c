#include "common.h"
#include "diskio.h"
#include "sd/sd.h"
#include "board.h"

#define SD_CARD_PRESENT		1
#define USB_STICK_PRESENT	0

//#include "usb_host/usb_conf.h"
//#include "usb_host/usbh_usr.h"
//#include "usb_host/usbh_msc_core.h"

#if USB_STICK_PRESENT
	extern USBH_Usr_Result_t USBH_Usr_INT_Result;
	extern USB_OTG_CORE_HANDLE     USB_OTG_Core;
	extern USBH_HOST               USB_Host;
#endif

#define SD_CARD		0
#define USB_STICK	1

#define ledSd(a)	led3(a)


DSTATUS status[2] = {STA_NOINIT,STA_NOINIT};

DWORD get_fattime (void)
{
	return(0);
}

DSTATUS disk_initialize (BYTE lun)
{
	xprintf("disk_initialize, lun=%d\n",lun);
	
	switch(lun)
	{
		case SD_CARD:
		{
			ledSd(LED_ON);
			xprintf("Initializing SD Card...\n");
			
			for(int i=0;i<5;i++)
			{
				xprintf("--- initializing SD card, try %d\n",i+1);
				if(sdInit()==0)
				{
					xprintf("SD Card init OK\n");
					status[lun] = 0;
					ledSd(LED_OFF);
					return(0);
				}
				vTaskDelay(200);
			}
			xprintf("SD Card init failed\n");
			status[lun] = STA_NOINIT;
			ledSd(LED_OFF);
			return(STA_NOINIT);
		}
		
		#if USB_STICK_PRESENT
		case USB_STICK:
		{
			ledUsb(LED_ON);
			xprintf("initializing FATFS for USB...\n");
			if(HCD_IsDeviceConnected(&USB_OTG_Core))
			{
				xprintf("FATfs for USB init OK\n");
				status[lun] = 0;
				ledUsb(LED_OFF);
				return 0;
			}
			else
			{
				xprintf("FATfs for USB init FAILED\n");
				status[lun] = STA_NOINIT;
				ledUsb(LED_OFF);
				return(STA_NOINIT);
			}
			
		}
		#endif
		
		default:
			xprintf("disk_initialize wrong lun: %d\n",lun);
			break;
	}
	
	return(STA_NOINIT);
}

DSTATUS disk_status (BYTE lun)
{
	//xprintf("disk_status rq, lun=%d, returning %d\n",lun,status[lun]);
	return(status[lun]);
}

DRESULT disk_read (BYTE lun, BYTE* buffer, DWORD lba, BYTE count)
{
	//if (status[lun] == STA_NOINIT) return RES_NOTRDY;
	
	switch(lun)
	{
		case SD_CARD:
		{
			ledSd(LED_ON);
			if(
				sdReadBlocks(lba,count,buffer) != 0
				)
				{
					xprintf("sdReadBlocks error, lba=%X, count=%X\n",(unsigned int)lba,(unsigned int)count);
					ledSd(LED_OFF);
					return(RES_ERROR);
				}
			ledSd(LED_OFF);
			return(RES_OK);
		}
		
		#if USB_STICK_PRESENT
		case USB_STICK:
		{
			uint8_t usbsta = USBH_MSC_OK;
			ledUsb(LED_ON);
			do{
			usbsta = USBH_MSC_Read10(&USB_OTG_Core, buffer, lba, 512 * count);
			USBH_MSC_HandleBOTXfer(&USB_OTG_Core, &USB_Host);
			if (!HCD_IsDeviceConnected(&USB_OTG_Core))
				{
					xprintf("disk read, returning RES_ERR\n");
					ledUsb(LED_OFF);
					return RES_ERROR;
				}
			}while (usbsta == USBH_MSC_BUSY);
			ledUsb(LED_OFF);
			return(RES_OK);
		}
		#endif
		
		default:
			xprintf("disk_read wrong lun: %d\n",lun);
			break;
	}
	
	xprintf("disk read, returning RES_NOTRDY\n");
	return RES_NOTRDY;
}

#if	_READONLY == 0
DRESULT disk_write (BYTE lun, const BYTE* buffer, DWORD lba, BYTE count)
{
	//if (status[lun] == STA_NOINIT) return RES_NOTRDY;
	
	switch(lun)
	{
		case SD_CARD:
		{
			ledSd(LED_ON);
			if(
				sdWriteBlocks(lba,count,(uint8_t*)buffer) != 0
				)
				{
					xprintf("sdReadBlocks error, lba=%X, count=%X\n",(unsigned int)lba,(unsigned int)count);
					ledSd(LED_OFF);
					return(RES_ERROR);
				}
			ledSd(LED_OFF);
			return(RES_OK);
		}
		
		#if USB_STICK_PRESENT
		case USB_STICK:
		{
			ledUsb(LED_ON);
			uint8_t usbsta = USBH_MSC_OK;
			do{
			usbsta = USBH_MSC_Write10(&USB_OTG_Core, buffer, lba, 512 * count);
			USBH_MSC_HandleBOTXfer(&USB_OTG_Core, &USB_Host);
			if (!HCD_IsDeviceConnected(&USB_OTG_Core))
				{
					xprintf("disk write, returning RES_ERR\n");
					ledUsb(LED_OFF);
					return RES_ERROR;
				}
			}while (usbsta == USBH_MSC_BUSY);
			ledUsb(LED_OFF);
			return(RES_OK);
		}
		#endif
		
		default:
			xprintf("disk_write wrong lun: %d\n",lun);
			break;
	}
	xprintf("disk write, returning RES_NOTRDY\n");
	return RES_NOTRDY;
}

#endif	//#if	_READONLY == 0



DRESULT disk_ioctl (BYTE lun, BYTE ctrl, void * response)
{
	switch(lun)
	{
		case SD_CARD:
		{
			switch(ctrl)
			{
				case CTRL_SYNC:
				{
					return(RES_OK);
				}
				case GET_SECTOR_COUNT:
				{
					uint32_t max_lba;
					uint16_t block_size;
					sdGetSizeInfo(&max_lba,&block_size);
					*(DWORD*)response = max_lba;
					xprintf("fatFS, GET_SECTOR_COUNT, returning value %X\n",(unsigned int)max_lba);
					return(RES_OK);
				}
				case GET_BLOCK_SIZE:
				{
					xprintf("fatFS, GET_SECTOR_SIZE, returning RES_OK\n");
					*(DWORD*)response = 1;
					return(RES_OK);
				}
				case GET_SECTOR_SIZE:
				{
					uint32_t max_lba;
					uint16_t block_size;
					sdGetSizeInfo(&max_lba,&block_size);
					*(DWORD*)response = block_size;
					xprintf("fatFS, GET_SECTOR_SIZE, returning value %d \n", block_size);
					return(RES_OK);
				}
				case CTRL_ERASE_SECTOR:
				{
					xprintf("CTRL_ERASE_SECTOR, returning OK\n");
					return(RES_OK);
				}
			}
		}
		
		#if USB_STICK_PRESENT
		case USB_STICK:
		{
			switch(ctrl)
			{
				case CTRL_SYNC:
				{
					return(RES_OK);
				}
				case GET_SECTOR_COUNT:
				{
					*(DWORD*)response = USBH_MSC_Param.MSCapacity;
					xprintf("fatFS, USB GET_SECTOR_COUNT, returning value %d\n",USBH_MSC_Param.MSCapacity);
					return(RES_OK);
				}
				case GET_BLOCK_SIZE:
				{
					debug_msg("fatFS, USB GET_BLOCK_SIZE, returning value 512");
					*(DWORD*)response = 512;
					return(RES_OK);
				}
				case GET_SECTOR_SIZE:
				{
					debug_msg("fatFS, USB GET_SECTOR_SIZE, returning value 512");
					*(DWORD*)response = 512;
					return(RES_OK);
				}
				case CTRL_ERASE_SECTOR:
				{
					debug_msg("CTRL_ERASE_SECTOR, returning OK");
					return(RES_OK);
				}
			}
		}
		#endif
		
	}
	return(RES_PARERR);
}



WCHAR ff_convert (WCHAR wch, UINT dir)
{
	if (wch < 0x80) {
		/* ASCII Char */
		return wch;
	}

	/* I don't support unicode it is too big! */
	return 0;
}

WCHAR ff_wtoupper (WCHAR wch)
{
	if (wch < 0x80) {
		/* ASCII Char */
		if (wch >= 'a' && wch <= 'z') {
			wch &= ~0x20;
		}
		return wch;
	}

	/* I don't support unicode it is too big! */
	return 0;
}

