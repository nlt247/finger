#ifndef __linux__

//#include "stdafx.h"
//#include <afxmt.h>
#include <windows.h>
//#include "stdio.h"

#include "device.h"
#include<devguid.h>
#include <dbt.h>
extern "C"{
#include <setupapi.h>   // from MS Platform SDK
}

//#include "direct.h"
//#include <time.h>
#include <winioctl.h>
#include <windows.h>
#include <Basetsd.h>
#include <usbioctl.h>
#include <devioctl.h>
//#include <ntdddisk.h>
#include <ntddscsi.h>
//#include <stdio.h>
#include <stddef.h>
//#include <stdlib.h>
#include "spti.h"
#include "define.h"
#include "usbcommand.h"
#include "command.h"
//#include "Protocol.h"

#include <QtGlobal>
#include <string>

HANDLE hUsbHandle = INVALID_HANDLE_VALUE;

BOOL USBSCSIInit(QString dev_name)
{
    Q_UNUSED(dev_name)

    BOOL    w_bRet;
    HANDLE  hUDisk= INVALID_HANDLE_VALUE;
    CHAR    strDriver[25]; 
    int     Driver;
    
    std::wstring strPath = L"A:";
    
    for ( Driver='C'; Driver<='Z'; Driver++ )
    {
//        strPath = strPath.Format( _T("%c:"),Driver );
        strPath[0] = Driver;
        int type = GetDriveType( strPath.data() );
        if( type==DRIVE_REMOVABLE || type==DRIVE_CDROM )
        {
            sprintf_s(strDriver,"\\\\.\\%c:",Driver);   
            hUDisk = CreateFileA(strDriver,
                                GENERIC_WRITE | GENERIC_READ,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                OPEN_EXISTING,
                                0,
                                NULL);  
            
            if(hUDisk==INVALID_HANDLE_VALUE) continue;
            hUsbHandle=hUDisk;

            InitCmdPacket(CMD_TEST_CONNECTION, 0, 0, NULL, 0);
            
            w_bRet = USB_SendPacket(hUDisk, CMD_TEST_CONNECTION, 0, 0);
            
            if ( !w_bRet )
                continue;
            
            if(RESPONSE_RET != ERR_SUCCESS)
                continue;
                    
            //if(g_pRcmPacket->m_abyData[0] == p_nDeviceID)
            {
                hUsbHandle=hUDisk;
                return TRUE;
            }           
        }
    }

    hUsbHandle = INVALID_HANDLE_VALUE;
    return FALSE;
}

void USBSCSIDeInit(void)
{
}

BOOL USBSCSIRead(HANDLE hHandle,BYTE* pCDB,DWORD nCDBLen,BYTE*pData,DWORD *nLength,DWORD nTimeOut)
{
    Q_UNUSED(hHandle)

    BOOL status = 0;

    SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
    DWORD length = 0;
    DWORD TransLen;

    if (hUsbHandle == INVALID_HANDLE_VALUE)
        return 0;

    ZeroMemory(&sptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));

    TransLen=*nLength;

    sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
    sptdwb.sptd.PathId = 0;
    sptdwb.sptd.TargetId = 1;
    sptdwb.sptd.Lun = 0;
    sptdwb.sptd.CdbLength = CDB6GENERIC_LENGTH;
    sptdwb.sptd.SenseInfoLength = 0;
    sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_IN;
    sptdwb.sptd.DataTransferLength =TransLen;

    sptdwb.sptd.TimeOutValue = nTimeOut;
    sptdwb.sptd.DataBuffer = pData;
    sptdwb.sptd.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
    memcpy(sptdwb.sptd.Cdb,pCDB,nCDBLen);

    SetLastError( 0 );

    length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
    status = DeviceIoControl(hUsbHandle,
                             IOCTL_SCSI_PASS_THROUGH_DIRECT,
                             &sptdwb,
                             length,
                             &sptdwb,
                             length,
                             nLength,
                             FALSE);
    *nLength=sptdwb.sptd.DataTransferLength;

    if ( status == FALSE  || sptdwb.sptd.ScsiStatus)
    {
        return FALSE;
    }

    return status;
}

BOOL USBSCSIWrite(HANDLE hHandle,BYTE* pCDB,DWORD nCDBLen,BYTE* pData,DWORD nLength,DWORD nTimeOut)
{
    Q_UNUSED(hHandle)
    BOOL status = 0;

    SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
    DWORD length = 0,
        returned = 0;
    if (hUsbHandle == INVALID_HANDLE_VALUE)
        return 0;
    DWORD TransLen=nLength;

    ZeroMemory(&sptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));

    sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
    sptdwb.sptd.PathId = 0;
    sptdwb.sptd.TargetId = 1;
    sptdwb.sptd.Lun = 0;
    sptdwb.sptd.CdbLength = CDB6GENERIC_LENGTH;
    sptdwb.sptd.SenseInfoLength = 0;
    sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_OUT;
    sptdwb.sptd.DataTransferLength = TransLen;

    sptdwb.sptd.TimeOutValue = nTimeOut;
    sptdwb.sptd.DataBuffer = pData;
    sptdwb.sptd.SenseInfoOffset =
        offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
    memcpy(sptdwb.sptd.Cdb,pCDB,nCDBLen);
    length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
    SetLastError( 0 );
    status = DeviceIoControl(hUsbHandle,
                             IOCTL_SCSI_PASS_THROUGH_DIRECT,
                             &sptdwb,
                             length,
                             &sptdwb,
                             length,
                             &returned,
                             FALSE);

    if ( status == FALSE || sptdwb.sptd.ScsiStatus )
    {
        return FALSE;
    }

    return status;

}

#else

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <scsi/sg.h>

#include <QtGlobal>
#include <QDebug>

#include "define.h"
#include "device.h"

int sg_fd = -1;

BOOL USBSCSIInit(QString dev_name)
{
    sg_fd = open(dev_name.toLatin1(), O_RDWR | O_NONBLOCK);
    if (sg_fd < 0) {
        qInfo() << "[FINGERPRINT]Error opening file:" << dev_name;
        return FALSE;
    }
    return TRUE;
}

void USBSCSIDeInit(void)
{
    if (sg_fd >= 0)
        close(sg_fd);
}

BOOL USBSCSIRead(HANDLE hHandle, BYTE *pCDB, DWORD nCDBLen, BYTE *pData, DWORD *nLength, DWORD nTimeout)
{
    Q_UNUSED(hHandle)

    sg_io_hdr_t io_hdr;
    int status;

    if (sg_fd < 0)
        return false;

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
        qInfo() << "[FINGERPRINT]SCSI Read Error" << status;
        return false;
    }
//    qInfo() << "[FINGERPRINT] Read Data:" << QByteArray((const char *) pData, *nLength).toHex(' ');

    return true;
}

BOOL USBSCSIWrite(HANDLE hHandle, BYTE *pCDB, DWORD nCDBLen, BYTE *pData, DWORD nLength, DWORD nTimeout)
{
    Q_UNUSED(hHandle)

    sg_io_hdr_t io_hdr;
    int status;

    if (sg_fd < 0)
        return false;

//    qInfo() << "[FINGERPRINT]  CDB Data:" << QByteArray((const char *) pCDB, nCDBLen).toHex(' ');
//    qInfo() << "[FINGERPRINT]Write Data:" << QByteArray((const char *) pData, nLength).toHex(' ');

    memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
    io_hdr.interface_id = 'S';
    io_hdr.cmd_len = nCDBLen;
    io_hdr.cmdp = pCDB;
    io_hdr.mx_sb_len = 0;
    io_hdr.sbp = NULL;
    io_hdr.timeout = nTimeout;
    io_hdr.dxfer_direction = SG_DXFER_TO_DEV;
    io_hdr.dxfer_len = nLength;
//    io_hdr.dxfer_len = nLength;
    io_hdr.dxferp = pData;
    status = ioctl(sg_fd, SG_IO, &io_hdr);
    if (status < 0) {
        qInfo() << "[FINGERPRINT]SCSI Write Error" << errno << strerror(errno);
        return false;
    }

    return true;
}

#endif
