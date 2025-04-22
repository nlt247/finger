#include "define.h"

#include "device.h"
#include "usbcommand.h"
#include "command.h"
#include "crypt/crypt_user.h"

#include "communication.h"

CCommunication::CCommunication(QString devname)
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

    if (USBSCSIInit(m_devname) == FALSE)
	{
		CloseConnection();
		return ERR_USB_OPEN_FAIL;
	}

	return CONNECTION_SUCCESS;
}

int	CCommunication::Run_GetImage(void)
{
	BOOL	w_bRet;
	
	w_bRet = Run_Command_NP(CMD_GET_IMAGE);
	
	if(!w_bRet)
		return ERR_CONNECTION;
	
	return RESPONSE_RET;
}

int	CCommunication::Run_FingerDetect(int* p_pnDetectResult)
{
	BOOL	w_bRet;
	
	w_bRet = Run_Command_NP(CMD_FINGER_DETECT);
	
	if (!w_bRet)
		return ERR_CONNECTION;
	
	*p_pnDetectResult = g_pRcmPacket->m_abyData[0];
	
	return ERR_SUCCESS;	
}

int	CCommunication::Run_StoreChar(int p_nTmplNo, int p_nRamBufferID, int* p_pnDupTmplNo)
{
	BOOL	w_bRet;
	BYTE	w_abyData[4];
	
	w_abyData[0] = LOBYTE(p_nTmplNo);
	w_abyData[1] = HIBYTE(p_nTmplNo);
	w_abyData[2] = LOBYTE(p_nRamBufferID);
	w_abyData[3] = HIBYTE(p_nRamBufferID);
	
	InitCmdPacket(CMD_STORE_CHAR, m_bySrcDeviceID, m_byDstDeviceID, w_abyData, 4);
	
	//. Send command packet to target
	SEND_COMMAND(CMD_STORE_CHAR, w_bRet, m_bySrcDeviceID, m_byDstDeviceID);
	
	if (!w_bRet)
		return ERR_CONNECTION;
	
	if (RESPONSE_RET != ERR_SUCCESS)
	{
		if (RESPONSE_RET == ERR_DUPLICATION_ID)
			*p_pnDupTmplNo = MAKEWORD(g_pRcmPacket->m_abyData[0], g_pRcmPacket->m_abyData[1]);

		return RESPONSE_RET;
	}
	
	return RESPONSE_RET;
}

int CCommunication::Run_DelChar(int p_nSTmplNo, int p_nETmplNo)
{
	BOOL	w_bRet;
	BYTE	w_abyData[4];
	
	w_abyData[0] = LOBYTE(p_nSTmplNo);
	w_abyData[1] = HIBYTE(p_nSTmplNo);
	w_abyData[2] = LOBYTE(p_nETmplNo);
	w_abyData[3] = HIBYTE(p_nETmplNo);
	
	//. Assemble command packet
	InitCmdPacket(CMD_DEL_CHAR, m_bySrcDeviceID, m_byDstDeviceID, w_abyData, 4);
	
	SEND_COMMAND(CMD_DEL_CHAR, w_bRet, m_bySrcDeviceID, m_byDstDeviceID);
	
	if(!w_bRet)
		return ERR_CONNECTION;
	
	return RESPONSE_RET;	
}

int	CCommunication::Run_GetEmptyID(int p_nSTmplNo, int p_nETmplNo, int* p_pnEmptyID)
{
	BOOL	w_bRet;
	BYTE	w_abyData[4];

	w_abyData[0] = LOBYTE(p_nSTmplNo);
	w_abyData[1] = HIBYTE(p_nSTmplNo);
	w_abyData[2] = LOBYTE(p_nETmplNo);
	w_abyData[3] = HIBYTE(p_nETmplNo);

	//. Assemble command packet
	InitCmdPacket(CMD_GET_EMPTY_ID, m_bySrcDeviceID, m_byDstDeviceID, w_abyData, 4);
	
	SEND_COMMAND(CMD_GET_EMPTY_ID, w_bRet, m_bySrcDeviceID, m_byDstDeviceID);
	
	if(!w_bRet)
		return ERR_CONNECTION;
	
	if ( RESPONSE_RET != ERR_SUCCESS )	
		return RESPONSE_RET;
	
	*p_pnEmptyID = MAKEWORD(g_pRcmPacket->m_abyData[0], g_pRcmPacket->m_abyData[1]);
	
	return ERR_SUCCESS;	
}

int	CCommunication::Run_GetStatus(int p_nTmplNo, int* p_pnStatus)
{
	BOOL	w_bRet;
	BYTE	w_abyData[2];
	
	w_abyData[0] = LOBYTE(p_nTmplNo);
	w_abyData[1] = HIBYTE(p_nTmplNo);
	
	InitCmdPacket(CMD_GET_STATUS, m_bySrcDeviceID, m_byDstDeviceID, w_abyData, 2);
	
	SEND_COMMAND(CMD_GET_STATUS, w_bRet, m_bySrcDeviceID, m_byDstDeviceID);
	
	if(!w_bRet)
		return ERR_CONNECTION;
	
	if ( RESPONSE_RET != ERR_SUCCESS )	
		return RESPONSE_RET;
	
	*p_pnStatus = g_pRcmPacket->m_abyData[0];
	
	return ERR_SUCCESS;	
}

int	CCommunication::Run_Generate(int p_nRamBufferID)
{
	BOOL	w_bRet;
	BYTE	w_abyData[2];
	
	w_abyData[0] = LOBYTE(p_nRamBufferID);
	w_abyData[1] = HIBYTE(p_nRamBufferID);
	
	InitCmdPacket(CMD_GENERATE, m_bySrcDeviceID, m_byDstDeviceID, w_abyData, 2);
	
	//. Send command packet to target
	SEND_COMMAND(CMD_GENERATE, w_bRet, m_bySrcDeviceID, m_byDstDeviceID);
	
	if(!w_bRet) 
		return ERR_CONNECTION;
	
	return RESPONSE_RET;		
}

int	CCommunication::Run_Merge(int p_nRamBufferID, int p_nMergeCount)
{
	BOOL	w_bRet;
	BYTE	w_abyData[3];
	
	w_abyData[0] = LOBYTE(p_nRamBufferID);
	w_abyData[1] = HIBYTE(p_nRamBufferID);
	w_abyData[2] = p_nMergeCount;
	
	InitCmdPacket(CMD_MERGE, m_bySrcDeviceID, m_byDstDeviceID, w_abyData, 3);
	
	SEND_COMMAND(CMD_MERGE, w_bRet, m_bySrcDeviceID, m_byDstDeviceID);
	
	if (!w_bRet) 
		return ERR_CONNECTION;
	
	return RESPONSE_RET;	
}

int	CCommunication::Run_Search(int p_nRamBufferID, int p_nStartID, int p_nSearchCount, int* p_pnTmplNo, int* p_pnLearnResult)
{
	BOOL	w_bRet;
	BYTE	w_abyData[6];
	
	w_abyData[0] = LOBYTE(p_nRamBufferID);
	w_abyData[1] = HIBYTE(p_nRamBufferID);
	w_abyData[2] = LOBYTE(p_nStartID);
	w_abyData[3] = HIBYTE(p_nStartID);
	w_abyData[4] = LOBYTE(p_nSearchCount);
	w_abyData[5] = HIBYTE(p_nSearchCount);
	
	InitCmdPacket(CMD_SEARCH, m_bySrcDeviceID, m_byDstDeviceID, w_abyData, 6);
	
	//. Send command packet to target
	SEND_COMMAND(CMD_SEARCH, w_bRet, m_bySrcDeviceID, m_byDstDeviceID);
	
	if (!w_bRet)
		return ERR_CONNECTION;
	
	if(RESPONSE_RET != ERR_SUCCESS)
		return RESPONSE_RET;
	
	*p_pnTmplNo			= MAKEWORD(g_pRcmPacket->m_abyData[0], g_pRcmPacket->m_abyData[1]);
	*p_pnLearnResult	= g_pRcmPacket->m_abyData[2];
	
	return RESPONSE_RET;
}

BOOL CCommunication::Run_Command_NP( WORD p_wCMD )
{
	BOOL w_bRet;

	//. Assemble command packet
    InitCmdPacket(p_wCMD, m_bySrcDeviceID, m_byDstDeviceID, nullptr, 0);

 	SEND_COMMAND(p_wCMD, w_bRet, m_bySrcDeviceID, m_byDstDeviceID);

	return w_bRet;
}

void CCommunication::CloseConnection() 
{
	USBSCSIDeInit();
}
