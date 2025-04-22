#ifndef __FP_COMMUNICATION__
#define __FP_COMMUNICATION__

#include "define.h"
#include <string>

class CCommunication  
{
private:
    int     m_nConnectionMode;
    BYTE    m_bySrcDeviceID;
    BYTE    m_byDstDeviceID;
    HANDLE  m_hUsbHandle;

public: 
    CCommunication(const std::string& devname);
    virtual ~CCommunication();

    int Run_InitConnection();

    int Run_GetImage(void);
    int Run_FingerDetect(int* p_pnDetectResult);

    int Run_StoreChar(int p_nTmplNo, int p_nRamBufferID, int* p_pnDupTmplNo);
    int Run_DelChar(int p_nSTmplNo, int p_nETmplNo);

    int Run_GetEmptyID(int p_nSTmplNo, int p_nETmplNo, int* p_pnEmptyID);
    int Run_GetStatus(int p_nTmplNo, int* p_pnStatus);

    int Run_Generate(int p_nRamBufferID);
    int Run_Merge(int p_nRamBufferID, int p_nMergeCount);

    int Run_Search(int p_nRamBufferID, int p_nStartID, int p_nSearchCount, int* p_pnTmplNo, int* p_pnLearnResult);

    BOOL Run_Command_NP(WORD p_wCMD);

    void CloseConnection();

private:
    std::string m_devname;
};

#endif