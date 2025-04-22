#include "define.h"

#include "device.h"
#include "usbcommand.h"
#include "command.h"
#include "crypt/crypt_user.h"

#include "communication.h"

CCommunication::CCommunication(const std::string& devname)
{
    m_nConnectionMode = USB_CON_MODE;
    m_hUsbHandle = nullptr;
    m_devname = devname;
}

CCommunication::~CCommunication()
{
    USBSCSIDeInit();
}

int CCommunication::Run_InitConnection()
{
    m_bySrcDeviceID = 0;
    m_byDstDeviceID = 0;

    if (USBSCSIInit(m_devname.c_str()) == FALSE)
    {
        CloseConnection();
        return ERR_USB_OPEN_FAIL;
    }

    return CONNECTION_SUCCESS;
}

int CCommunication::Run_GetImage(void)
{
    InitCmdPacket(CMD_GET_IMAGE, m_bySrcDeviceID, m_byDstDeviceID, NULL, 0);
    
    if (!USB_SendPacket(m_hUsbHandle, CMD_GET_IMAGE, m_bySrcDeviceID, m_byDstDeviceID))
        return ERR_CONNECTION;
    
    return RESPONSE_RET;
}

int CCommunication::Run_FingerDetect(int* p_pnDetectResult)
{
    InitCmdPacket(CMD_FINGER_DETECT, m_bySrcDeviceID, m_byDstDeviceID, NULL, 0);
    
    if (!USB_SendPacket(m_hUsbHandle, CMD_FINGER_DETECT, m_bySrcDeviceID, m_byDstDeviceID))
        return ERR_CONNECTION;
    
    if (RESPONSE_RET != ERR_SUCCESS)
        return RESPONSE_RET;
    
    *p_pnDetectResult = g_pRcmPacket->m_abyData[0];
    return ERR_SUCCESS;
}

int CCommunication::Run_StoreChar(int p_nTmplNo, int p_nRamBufferID, int* p_pnDupTmplNo)
{
    BYTE w_abyData[4];
    
    w_abyData[0] = LOBYTE(p_nTmplNo);
    w_abyData[1] = HIBYTE(p_nTmplNo);
    w_abyData[2] = p_nRamBufferID;
    
    InitCmdPacket(CMD_STORE_CHAR, m_bySrcDeviceID, m_byDstDeviceID, w_abyData, 3);
    
    if (!USB_SendPacket(m_hUsbHandle, CMD_STORE_CHAR, m_bySrcDeviceID, m_byDstDeviceID))
        return ERR_CONNECTION;
    
    if (RESPONSE_RET != ERR_SUCCESS)
        return RESPONSE_RET;
    
    *p_pnDupTmplNo = MAKEWORD(g_pRcmPacket->m_abyData[0], g_pRcmPacket->m_abyData[1]);
    return ERR_SUCCESS;
}

int CCommunication::Run_DelChar(int p_nSTmplNo, int p_nETmplNo)
{
    BYTE w_abyData[4];
    
    w_abyData[0] = LOBYTE(p_nSTmplNo);
    w_abyData[1] = HIBYTE(p_nSTmplNo);
    w_abyData[2] = LOBYTE(p_nETmplNo);
    w_abyData[3] = HIBYTE(p_nETmplNo);
    
    InitCmdPacket(CMD_DEL_CHAR, m_bySrcDeviceID, m_byDstDeviceID, w_abyData, 4);
    
    if (!USB_SendPacket(m_hUsbHandle, CMD_DEL_CHAR, m_bySrcDeviceID, m_byDstDeviceID))
        return ERR_CONNECTION;
    
    return RESPONSE_RET;
}

int CCommunication::Run_GetEmptyID(int p_nSTmplNo, int p_nETmplNo, int* p_pnEmptyID)
{
    BYTE w_abyData[4];
    
    w_abyData[0] = LOBYTE(p_nSTmplNo);
    w_abyData[1] = HIBYTE(p_nSTmplNo);
    w_abyData[2] = LOBYTE(p_nETmplNo);
    w_abyData[3] = HIBYTE(p_nETmplNo);
    
    InitCmdPacket(CMD_GET_EMPTY_ID, m_bySrcDeviceID, m_byDstDeviceID, w_abyData, 4);
    
    if (!USB_SendPacket(m_hUsbHandle, CMD_GET_EMPTY_ID, m_bySrcDeviceID, m_byDstDeviceID))
        return ERR_CONNECTION;
    
    if (RESPONSE_RET != ERR_SUCCESS)
        return RESPONSE_RET;
    
    *p_pnEmptyID = MAKEWORD(g_pRcmPacket->m_abyData[0], g_pRcmPacket->m_abyData[1]);
    return ERR_SUCCESS;
}

int CCommunication::Run_Generate(int p_nRamBufferID)
{
    BYTE w_abyData[1];
    
    w_abyData[0] = p_nRamBufferID;
    
    InitCmdPacket(CMD_GENERATE, m_bySrcDeviceID, m_byDstDeviceID, w_abyData, 1);
    
    if (!USB_SendPacket(m_hUsbHandle, CMD_GENERATE, m_bySrcDeviceID, m_byDstDeviceID))
        return ERR_CONNECTION;
    
    return RESPONSE_RET;
}

int CCommunication::Run_Merge(int p_nRamBufferID, int p_nMergeCount)
{
    BYTE w_abyData[2];
    
    w_abyData[0] = p_nRamBufferID;
    w_abyData[1] = p_nMergeCount;
    
    InitCmdPacket(CMD_MERGE, m_bySrcDeviceID, m_byDstDeviceID, w_abyData, 2);
    
    if (!USB_SendPacket(m_hUsbHandle, CMD_MERGE, m_bySrcDeviceID, m_byDstDeviceID))
        return ERR_CONNECTION;
    
    return RESPONSE_RET;
}

int CCommunication::Run_Search(int p_nRamBufferID, int p_nStartID, int p_nSearchCount, int* p_pnTmplNo, int* p_pnLearnResult)
{
    BYTE w_abyData[5];
    
    w_abyData[0] = p_nRamBufferID;
    w_abyData[1] = LOBYTE(p_nStartID);
    w_abyData[2] = HIBYTE(p_nStartID);
    w_abyData[3] = LOBYTE(p_nSearchCount);
    w_abyData[4] = HIBYTE(p_nSearchCount);
    
    InitCmdPacket(CMD_SEARCH, m_bySrcDeviceID, m_byDstDeviceID, w_abyData, 5);
    
    if (!USB_SendPacket(m_hUsbHandle, CMD_SEARCH, m_bySrcDeviceID, m_byDstDeviceID))
        return ERR_CONNECTION;
    
    if (RESPONSE_RET != ERR_SUCCESS)
        return RESPONSE_RET;
    
    *p_pnTmplNo = MAKEWORD(g_pRcmPacket->m_abyData[0], g_pRcmPacket->m_abyData[1]);
    *p_pnLearnResult = g_pRcmPacket->m_abyData[2];
    
    return ERR_SUCCESS;
}

void CCommunication::CloseConnection()
{
    USBSCSIDeInit();
}