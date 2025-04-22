#ifndef _CDMODE_H_
#define _CDMODE_H_

#define SCSI_TIMEOUT 5

#define SYI_STATUS unsigned long
#define SYAPI    WINAPI


//---------------------return code---------------------------- 
// #define RT_OK            0x00 //?? 
// #define RT_FAIL          0x01 //?? 
#define RT_OK            0x00 //³É¹¦
#define RT_COMMAND_ERR   0x02 //ÃüÁî´íÎó 
#define RT_PARAM_ERR     0x03 //²ÎÊý´íÎó
#define RT_OVERTIME      0x04 //³¬Ê± 
#define RT_ECC_ERR       0x05 //Ð£Ñé´íÎó
#define RT_WRITE_ERR     0x06 //Ð´Êý¾Ý´íÎó
#define RT_READ_ERR      0x07 //¶ÁÊý¾Ý´íÎó 
#define RT_PACKAGE_ERR   0x08 //°üÊÕ·¢´íÎó

//---------------command--------------------------------------
#define CMD_DOWNCODE   	     0x01  //ÏÂÔØ´úÂëµ½RAM
#define CMD_UPDATE    		 0x02  //ÔÚÏß¸üÐÂ
#define CMD_DOWNALG          0x03  //ÏÂÔØËã·¨µ½Flash
#define CMD_GETDEVICEINFO    0x04  //»ñÈ¡Éè±¸ÐòÁÐºÅºÍROMÐ­Òé°æ±¾ 
#define CMD_MODIFYVIDPID     0x05  //ÐÞ¸ÄPID£¬VID
#define CMD_CHECKOTP         0x10  //
//-------------------------------------------------------------


//device info
typedef struct _T_DevInfo{
	unsigned char Ver[16];//device version
	unsigned char SN[16];//device sn      
}TDevInfo;

//command struct
typedef struct _T_PCCmd
{
	BYTE cHead[2];
	BYTE cCmd;
	BYTE cParam;
	union{
		BYTE cLen[2];
		WORD nLen;
	};
}TPCCmd;

//respond
typedef struct _T_Respond
{
	BYTE cCmd;
	BYTE cSW;
	union{
		BYTE cLen[2];
		WORD nLen;
	};
}TRespond;

typedef struct _tDevHandle
{
	int    nDeviceType;//Éè±¸ÀàÐÍ£¬CDROM·½Ê½£¬»¹ÊÇUSB Driver·½Ê½
    HANDLE hDevice;
	HANDLE hUSBWrite;
	HANDLE hUSBRead; 
	
}tDevHandle,*PtDevHandle;

bool OpenUSB(HANDLE* pHandle, int p_nDeviceID);
bool CloseUsb(HANDLE *pHandle);
bool USB_SendPacket(HANDLE hHandle, WORD p_wCMD, BYTE p_bySrcDeviceID, BYTE p_byDstDeviceID);
bool USB_ReceiveAck(HANDLE hHandle, WORD p_wCMD);
bool USB_ReceiveDataAck(HANDLE hHandle, WORD p_wCMD);
bool USB_SendRawData(HANDLE hHandle, PBYTE p_pBuffer, UINT p_dwDataLen);
bool USB_ReceiveRawData(HANDLE hHandle, PBYTE p_pBuffer, UINT p_dwDataLen);
bool USB_ReceiveImage(HANDLE hHandle, PBYTE p_pBuffer, UINT p_dwDataLen, BYTE p_bType);
int USB_DownImage(HANDLE hHandle, BYTE* pBuf, DWORD nBufLen);
int USB_DownFirmware(HANDLE hHandle, BYTE* pBuf, DWORD nBufLen);

bool USB_SendData(HANDLE hHandle, WORD	p_wCMD, int p_nDataLen, PBYTE p_pData);
bool USB_SendDataPacket(HANDLE hHandle, WORD p_wCMD, BYTE p_bySrcDeviceID, BYTE p_byDstDeviceID);
bool USB_ReceiveData(HANDLE hHandle, WORD	p_wCMD, int p_nDataLen, PBYTE p_pData);
bool USB_ReceiveDataPacket(HANDLE hHandle, WORD	p_wCMD, BYTE p_bySrcDeviceID, BYTE p_byDstDeviceID);

DWORD GetReadWaitTime(int p_nCmdCode);

#endif