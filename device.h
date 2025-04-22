#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "define.h"
#include <string>

typedef unsigned long  ULONG_PTR_1;

#define DEVICE_CDROM  1
#define DEVICE_DRIVER 2

#ifdef __linux__
extern int sg_fd;

// Linux SCSI 兼容定义
typedef struct {
    int dummy;  // 仅用作占位符
} SCSI_PASS_THROUGH_DIRECT;

// 定义常量
#define SCSI_IOCTL_DATA_IN   1
#define SCSI_IOCTL_DATA_OUT  0

#endif

#ifdef __cplusplus
extern "C" {
#endif

    BOOL USBSCSIInit(const char* dev_name);
    void USBSCSIDeInit(void);

    BOOL USBSCSIRead(HANDLE hHandle, BYTE *pCDB, DWORD nCDBLen, BYTE *pData, DWORD *nLength, DWORD nTimeout);
    BOOL USBSCSIWrite(HANDLE hHandle, BYTE *pCDB, DWORD nCDBLen, BYTE *pData, DWORD nLength, DWORD nTimeout);

#ifdef __cplusplus
}
#endif

#endif