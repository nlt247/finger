#ifndef __FP_API_H__
#define __FP_API_H__

#include "fingerprint_exports.h"

// 返回代码定义
#define FP_SUCCESS              0
#define FP_ERR_GENERAL         -1
#define FP_ERR_CONNECTION      -2
#define FP_ERR_USB_OPEN_FAIL   -3
#define FP_ERR_NO_DEVICE       -4
#define FP_ERR_TIMEOUT         -5

// 指纹操作常量
#define FP_BUFFER_1             1
#define FP_BUFFER_2             2
#define FP_BUFFER_3             3

#endif // __FP_API_H__