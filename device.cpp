#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __linux__
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <scsi/sg.h>
#else
#include <windows.h>
#include <devguid.h>
#include <setupapi.h>
#include <winioctl.h>
#include <Basetsd.h>
#include <usbioctl.h>
#include <devioctl.h>
#include <ntddscsi.h>
#include <stddef.h>
#endif

#include "define.h"
#include "device.h"
#include "command.h"
#include "usbcommand.h"

#ifdef __linux__
int sg_fd = -1;

BOOL USBSCSIInit(const char* dev_name)
{
    sg_fd = open(dev_name, O_RDWR | O_NONBLOCK);
    if (sg_fd < 0) {
        printf("[FINGERPRINT] Error opening file: %s\n", dev_name);
        return FALSE;
    }
    return TRUE;
}

void USBSCSIDeInit(void)
{
    if (sg_fd >= 0)
        close(sg_fd);
    sg_fd = -1;
}

BOOL USBSCSIRead(HANDLE hHandle, BYTE *pCDB, DWORD nCDBLen, BYTE *pData, DWORD *nLength, DWORD nTimeout)
{
    (void)hHandle; // 未使用参数

    sg_io_hdr_t io_hdr;
    int status;

    if (sg_fd < 0)
        return FALSE;

    memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
    io_hdr.interface_id = 'S';
    io_hdr.cmd_len = nCDBLen;
    io_hdr.cmdp = pCDB;
    io_hdr.mx_sb_len = 0;
    io_hdr.sbp = NULL;
    io_hdr.timeout = nTimeout;
    io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;
    io_hdr.dxfer_len = *nLength;
    io_hdr.dxferp = pData;
    status = ioctl(sg_fd, SG_IO, &io_hdr);
    if (status < 0) {
        printf("[FINGERPRINT] SCSI Read Error %d\n", status);
        return FALSE;
    }

    return TRUE;
}

BOOL USBSCSIWrite(HANDLE hHandle, BYTE *pCDB, DWORD nCDBLen, BYTE *pData, DWORD nLength, DWORD nTimeout)
{
    (void)hHandle; // 未使用参数
    
    sg_io_hdr_t io_hdr;
    int status;

    if (sg_fd < 0)
        return FALSE;

    memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
    io_hdr.interface_id = 'S';
    io_hdr.cmd_len = nCDBLen;
    io_hdr.cmdp = pCDB;
    io_hdr.mx_sb_len = 0;
    io_hdr.sbp = NULL;
    io_hdr.timeout = nTimeout;
    io_hdr.dxfer_direction = SG_DXFER_TO_DEV;
    io_hdr.dxfer_len = nLength;
    io_hdr.dxferp = pData;
    status = ioctl(sg_fd, SG_IO, &io_hdr);
    if (status < 0) {
        printf("[FINGERPRINT] SCSI Write Error %d: %s\n", errno, strerror(errno));
        return FALSE;
    }

    return TRUE;
}

#else
// Windows实现部分
HANDLE hUsbHandle = INVALID_HANDLE_VALUE;

BOOL USBSCSIInit(const char* dev_name)
{
    (void)dev_name; // Windows版本未使用设备名参数

    BOOL    w_bRet;
    HANDLE  hUDisk= INVALID_HANDLE_VALUE;
    CHAR    strDriver[25]; 
    int     Driver;
    
    WCHAR strPath[] = L"A:";
    
    for (Driver='C'; Driver<='Z'; Driver++)
    {
        strPath[0] = Driver;
        int type = GetDriveTypeW(strPath);
        if (type==DRIVE_REMOVABLE || type==DRIVE_CDROM)
        {
            sprintf_s(strDriver, "\\\\.\\%c:", Driver);   
            hUDisk = CreateFileA(strDriver,
                                GENERIC_WRITE | GENERIC_READ,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                OPEN_EXISTING,
                                0,
                                NULL);  
            
            if (hUDisk==INVALID_HANDLE_VALUE) continue;
            hUsbHandle = hUDisk;

            InitCmdPacket(CMD_TEST_CONNECTION, 0, 0, NULL, 0);
            
            w_bRet = USB_SendPacket(hUDisk, CMD_TEST_CONNECTION, 0, 0);
            
            if (!w_bRet)
                continue;
            
            if (RESPONSE_RET != ERR_SUCCESS)
                continue;
                    
            hUsbHandle = hUDisk;
            return TRUE;
        }
    }

    hUsbHandle = INVALID_HANDLE_VALUE;
    return FALSE;
}

void USBSCSIDeInit(void)
{
    if (hUsbHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(hUsbHandle);
        hUsbHandle = INVALID_HANDLE_VALUE;
    }
}

// Windows SCSI读写实现（保持不变）...
#endif