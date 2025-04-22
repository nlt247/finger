#ifndef FINGERPRINT_EXPORTS_H
#define FINGERPRINT_EXPORTS_H

#include "fingerprint_api.h"
#include "fingerprint.h"
#include "communication.h"

// 声明所有您想导出的函数
extern "C" {
    // FingerPrint_Worker类的导出工厂函数
    FINGERPRINT_API FingerPrint_Worker* CreateFingerPrintWorker(const char* device_path);
    FINGERPRINT_API void DestroyFingerPrintWorker(FingerPrint_Worker* worker);
    
    // FingerPrint_Worker类的方法包装
    FINGERPRINT_API bool FP_Worker_Init(FingerPrint_Worker* worker);
    FINGERPRINT_API void FP_Worker_Start(FingerPrint_Worker* worker);
    FINGERPRINT_API void FP_Worker_Stop(FingerPrint_Worker* worker);
    FINGERPRINT_API void FP_Worker_Detect_Start(FingerPrint_Worker* worker);
    FINGERPRINT_API void FP_Worker_Detect_Stop(FingerPrint_Worker* worker);
    FINGERPRINT_API void FP_Worker_Enroll_Start(FingerPrint_Worker* worker);
    FINGERPRINT_API void FP_Worker_Save(FingerPrint_Worker* worker, int finger_id);
    FINGERPRINT_API void FP_Worker_Delete(FingerPrint_Worker* worker, int finger_id);
    FINGERPRINT_API bool FP_Worker_Online(FingerPrint_Worker* worker);
    FINGERPRINT_API int FP_Worker_Avail_ID(FingerPrint_Worker* worker);
    FINGERPRINT_API bool FP_Worker_New_Template_Avail(FingerPrint_Worker* worker);
}

#endif // FINGERPRINT_EXPORTS_H