#ifndef __FP_COMMAND__
#define __FP_COMMAND__

#include "define.h"

//////////////////////////////	STRUCT	////////////////////////////////////////////
#pragma pack(1)

#define		COMM_TIMEOUT					5000

typedef struct _ST_CMD_PACKET_
{
	WORD	m_wPrefix;
	BYTE	m_bySrcDeviceID;
	BYTE	m_byDstDeviceID;
	WORD	m_wCMDCode;
	WORD	m_wDataLen;
	BYTE	m_abyData[16];
	WORD	m_wCheckSum;
} ST_CMD_PACKET, *PST_CMD_PACKET;

typedef struct _ST_RCM_PACKET_
{
	WORD	m_wPrefix;
	BYTE	m_bySrcDeviceID;
	BYTE	m_byDstDeviceID;
	WORD	m_wCMDCode;
	WORD	m_wDataLen;
	WORD	m_wRetCode;
	BYTE	m_abyData[14];
	WORD	m_wCheckSum;
} ST_RCM_PACKET, *PST_RCM_PACKET;

typedef struct _ST_COMMAND_
{
	char	szCommandName[64];
	WORD	wCode;
} ST_COMMAND, *PST_COMMAND;

#pragma pack()

#define	RESPONSE_RET				(g_pRcmPacket->m_wRetCode)
#define	CMD_PACKET_LEN				(sizeof(ST_CMD_PACKET))
#define	DATA_PACKET_LEN				(sizeof(ST_RCM_PACKET))

// packet encrypted data position
#define	PKT_POS_PREFIX				0
#define	PKT_POS_SRC_DEV_ID			2
#define	PKT_POS_DST_DEV_ID			3
#define	PKT_POS_CMD_CODE			4
#define	PKT_POS_DATA_LEN			6
#define	PKT_POS_ENC_DATA			8
#define	PKT_POS_CHKSUM				(8 + PKT_VAL_DATA_LEN)

// packet decrypt data position
#define	PKT_POS_DEC_DATA			4

// packet encrypted data values
#define	PKT_VAL_PREFIX				(*((unsigned short*)&g_Packet[PKT_POS_PREFIX]))
#define	PKT_VAL_SRC_DEV_ID			(*((unsigned char*)&g_Packet[PKT_POS_SRC_DEV_ID]))
#define	PKT_VAL_DST_DEV_ID			(*((unsigned char*)&g_Packet[PKT_POS_DST_DEV_ID]))
#define	PKT_VAL_CMD_CODE			(*((unsigned short*)&g_Packet[PKT_POS_CMD_CODE]))
#define	PKT_VAL_DATA_LEN			(*((unsigned short*)&g_Packet[PKT_POS_DATA_LEN]))
#define	PKT_VAL_ENC_DATA			((unsigned char*)&g_Packet[PKT_POS_ENC_DATA])
#define	PKT_VAL_CHKSUM				(*((unsigned short*)&g_Packet[PKT_POS_CHKSUM]))


#define	MAX_DATA_LEN				512
/////////////////////////////	Value	/////////////////////////////////////////////
extern BYTE				g_Packet[1024 * 64];
extern DWORD			g_dwPacketSize;
extern PST_CMD_PACKET	g_pCmdPacket;
extern PST_RCM_PACKET	g_pRcmPacket;
extern ST_COMMAND		g_Commands[];

/////////////////////////////	Function	/////////////////////////////////////////////
#define SEND_COMMAND(cmd, ret, nSrcDeviceID, nDstDeviceID)									\
    ret = USB_SendPacket(m_hUsbHandle, cmd, nSrcDeviceID, nDstDeviceID);

#define SEND_DATAPACKET(cmd, ret, nSrcDeviceID, nDstDeviceID)								\
    ret = USB_SendDataPacket(m_hUsbHandle, cmd, nSrcDeviceID, nDstDeviceID);

#define RECEIVE_DATAPACKET(cmd, ret, nSrcDeviceID, nDstDeviceID)						\
    ret = USB_ReceiveDataPacket(m_hUsbHandle, cmd, nSrcDeviceID, nDstDeviceID);

WORD	GetCheckSum(BOOL bCmdData);
BOOL	CheckReceive(BYTE* p_pbyPacket, DWORD p_dwPacketLen, WORD p_wPrefix, WORD p_wCMDCode);
void	InitCmdPacket(WORD p_wCMDCode, BYTE p_bySrcDeviceID, BYTE p_byDstDeviceID, BYTE* p_pbyData, WORD p_wDataLen);
void	InitCmdDataPacket(WORD p_wCMDCode, BYTE p_bySrcDeviceID, BYTE p_byDstDeviceID, BYTE* p_pbyData, WORD p_wDataLen);
BOOL	ReceiveAck(WORD p_wCMDCode, BYTE p_bySrcDeviceID, BYTE p_byDstDeviceID);

BOOL	SendDataPacket(WORD p_wCMDCode, BYTE p_bySrcDeviceID, BYTE p_byDstDeviceID);
BOOL	ReceiveDataAck(WORD p_wCMDCode, BYTE p_bySrcDeviceID, BYTE p_byDstDeviceID);
BOOL	ReceiveDataPacket(WORD p_wCMDCode, BYTE p_byDataLen, BYTE p_bySrcDeviceID, BYTE p_byDstDeviceID);
BOOL	ReadDataN(BYTE* p_pData, int p_nLen, DWORD p_dwTimeOut);

char*   GetErrorMsg(DWORD p_dwErrorCode);

int EncryptCommandPacket(void);
int DecryptCommandPacket(void);

#endif
