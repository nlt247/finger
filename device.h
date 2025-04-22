#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "define.h"
#include <QObject>

typedef unsigned long  ULONG_PTR_1;

#define DEVICE_CDROM  1
#define DEVICE_DRIVER 2

#ifdef __cplusplus
extern "C" {
#endif

	BOOL USBSCSIInit(QString dev_name);
	void USBSCSIDeInit(void);

	BOOL USBSCSIRead(HANDLE hHandle, BYTE *pCDB, DWORD nCDBLen, BYTE *pData, DWORD *nLength, DWORD nTimeout);
	BOOL USBSCSIWrite(HANDLE hHandle, BYTE *pCDB, DWORD nCDBLen, BYTE *pData, DWORD nLength, DWORD nTimeout);

#ifdef __cplusplus
}
#endif

#endif
