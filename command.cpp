#include <stdio.h>
#include <string.h>

#include "define.h"
#include "crypt/crypt_user.h"
#include "command.h"

DWORD			g_dwPacketSize = 0;
BYTE			g_Packet[1024*64];
PST_CMD_PACKET	g_pCmdPacket = (PST_CMD_PACKET)g_Packet;
PST_RCM_PACKET	g_pRcmPacket = (PST_RCM_PACKET)g_Packet;

BOOL CheckReceive( BYTE* p_pbyPacket, DWORD p_dwPacketLen, WORD p_wPrefix, WORD p_wCMDCode )
{
	int				i;
	WORD			w_wCalcCheckSum, w_wCheckSum;
	ST_RCM_PACKET*	w_pstRcmPacket;

	w_pstRcmPacket = (ST_RCM_PACKET*)p_pbyPacket;

	//. Check prefix code
 	if (p_wPrefix != w_pstRcmPacket->m_wPrefix)
 		return FALSE;
 	
	//. Check checksum
	w_wCheckSum = MAKEWORD(p_pbyPacket[p_dwPacketLen-2], p_pbyPacket[p_dwPacketLen-1]);
	w_wCalcCheckSum = 0;
	for (i=0; i<(int)p_dwPacketLen-2; i++)
	{
		w_wCalcCheckSum = w_wCalcCheckSum + p_pbyPacket[i];
	}
	
	if (w_wCheckSum != w_wCalcCheckSum)
		return FALSE;
	
	if (p_wCMDCode != w_pstRcmPacket->m_wCMDCode)
	{
		return FALSE;
	}

	return TRUE;
}

void InitCmdPacket(WORD p_wCMDCode, BYTE p_bySrcDeviceID, BYTE p_byDstDeviceID, BYTE* p_pbyData, WORD p_wDataLen)
{
    unsigned int		i;
	WORD	w_wCheckSum;

	memset( g_Packet, 0, sizeof( g_Packet ));
	g_pCmdPacket->m_wPrefix = CMD_PREFIX_CODE;
	g_pCmdPacket->m_bySrcDeviceID = p_bySrcDeviceID;
	g_pCmdPacket->m_byDstDeviceID = p_byDstDeviceID;
	g_pCmdPacket->m_wCMDCode = p_wCMDCode;
	g_pCmdPacket->m_wDataLen = p_wDataLen;

	if (p_wDataLen)
		memcpy(g_pCmdPacket->m_abyData, p_pbyData, p_wDataLen);

	w_wCheckSum = 0;

	for (i=0; i<sizeof(ST_CMD_PACKET)-2; i++)
	{
		w_wCheckSum = w_wCheckSum + g_Packet[i];
	}

	g_pCmdPacket->m_wCheckSum = w_wCheckSum;
	
	g_dwPacketSize = sizeof(ST_CMD_PACKET);
}

void	InitCmdDataPacket(WORD p_wCMDCode, BYTE p_bySrcDeviceID, BYTE p_byDstDeviceID, BYTE* p_pbyData, WORD p_wDataLen)
{
	int		i;
	WORD	w_wCheckSum;

	g_pCmdPacket->m_wPrefix = CMD_DATA_PREFIX_CODE;
	g_pCmdPacket->m_bySrcDeviceID = p_bySrcDeviceID;
	g_pCmdPacket->m_byDstDeviceID = p_byDstDeviceID;
	g_pCmdPacket->m_wCMDCode = p_wCMDCode;
	g_pCmdPacket->m_wDataLen = p_wDataLen;

	memcpy(&g_pCmdPacket->m_abyData[0], p_pbyData, p_wDataLen);

	//. Set checksum
	w_wCheckSum = 0;
	
	for (i=0; i<(p_wDataLen + 8); i++)
	{
		w_wCheckSum = w_wCheckSum + g_Packet[i];
	}

	g_Packet[p_wDataLen+8] = LOBYTE(w_wCheckSum);
	g_Packet[p_wDataLen+9] = HIBYTE(w_wCheckSum);

	g_dwPacketSize = p_wDataLen + 10;
}

int EncryptCommandPacket(void)
{
	unsigned short w_nCmdDataSize;
	unsigned int i;
	unsigned int w_nEncSize;
	unsigned int w_nTotalEncSize;
	int w_nRet;
	unsigned short w_nCalcSum;
	unsigned int w_nTmpSize;
	unsigned char w_bCryptedComm = 0;
	unsigned char w_pCryptTempBuf1[CRYPT_DEF_RSA_BYTE + 16];
	unsigned char w_pCryptTempBuf2[CRYPT_DEF_RSA_BYTE + 16];
	
	// check encrypt communication flag
	w_bCryptedComm = GetCryptState();
	if (w_bCryptedComm == 0)
		return ERR_FAIL;
	
	// check command code
	if ((g_pCmdPacket->m_wCMDCode == CMD_TEST_CONNECTION) ||
		(g_pCmdPacket->m_wCMDCode == CMD_GET_OEM_RSA_PUB_KEY) ||
		(g_pCmdPacket->m_wCMDCode == CMD_SET_HOST_RSA_PUB_KEY))
		return ERR_FAIL;
	
	// calculate command data size
	if (g_pCmdPacket->m_wPrefix == CMD_PREFIX_CODE)
		w_nCmdDataSize = CMD_PACKET_LEN;
	else if (g_pCmdPacket->m_wPrefix == CMD_DATA_PREFIX_CODE)
		w_nCmdDataSize = g_pCmdPacket->m_wDataLen + 10;
	else
		return ERR_FAIL;
	w_nCmdDataSize -= PKT_POS_DEC_DATA;

	// set crypted size
	w_nTmpSize = (((w_nCmdDataSize - 1) / CRYPT_DEF_RSA_SPLIT_SIZE) + 1) * CRYPT_DEF_RSA_SPLIT_SIZE;
	
	// padding data
	memset(&g_Packet[w_nCmdDataSize + PKT_POS_DEC_DATA], 0x00, w_nTmpSize - w_nCmdDataSize);
	
	// set encrypt data
	memcpy(w_pCryptTempBuf1, &g_Packet[PKT_POS_DEC_DATA], CRYPT_DEF_RSA_SPLIT_SIZE);
	w_nTotalEncSize = 0;
	for (i = 0; i < w_nTmpSize; i += CRYPT_DEF_RSA_SPLIT_SIZE)
	{
		// padding block
		crt_user_PaddingBlock(w_pCryptTempBuf1, CRYPT_DEF_RSA_SPLIT_SIZE, CRYPT_DEF_RSA_BYTE);

		// set encrypt
		w_nRet = crt_user_RSAEnc(w_pCryptTempBuf1, CRYPT_DEF_RSA_BYTE, w_pCryptTempBuf2, &w_nEncSize, g_pCryptRemoteN, g_pCryptRemoteE);
		if (w_nRet != CRT_SUCCESS)
			return ERR_FAIL;
		
		// set data to command packet
		if (i + CRYPT_DEF_RSA_SPLIT_SIZE < w_nTmpSize)
			memcpy(w_pCryptTempBuf1, &g_Packet[PKT_POS_DEC_DATA + i + CRYPT_DEF_RSA_SPLIT_SIZE], CRYPT_DEF_RSA_SPLIT_SIZE);
		memcpy(&g_Packet[PKT_POS_ENC_DATA + w_nTotalEncSize], &w_pCryptTempBuf2[0], w_nEncSize);

		// set encrypted size
		w_nTotalEncSize += w_nEncSize;
	}

	// set encrypted command parameters
	PKT_VAL_CMD_CODE = CMD_ENCRYPTED_COMMAND;
	PKT_VAL_DATA_LEN = w_nTotalEncSize;
	
	// set checksum
	w_nCalcSum = 0;
	for (i = 0; i < (unsigned int)PKT_POS_CHKSUM; i++)
		w_nCalcSum += g_Packet[i];
	PKT_VAL_CHKSUM = w_nCalcSum;

	// set converted data length
	g_dwPacketSize = PKT_POS_CHKSUM + 2;
	
	return ERR_SUCCESS;
}

int DecryptCommandPacket(void)
{
	unsigned short w_nCalcSum;
	unsigned int i;
	unsigned int w_nDecSize;
	unsigned short w_nTotalSize;
	unsigned int w_nEncSize;
	int w_nRet;
	unsigned char w_pCryptTempBuf[CRYPT_DEF_RSA_BYTE + 16];
	
	// check command code
	if (PKT_VAL_CMD_CODE != CMD_ENCRYPTED_COMMAND)
		return ERR_SUCCESS;
	
	// check encrypted data size
	if ((PKT_VAL_DATA_LEN % CRYPT_DEF_RSA_BYTE) != 0)
		return ERR_FAIL;
	
	// check checksum
	w_nCalcSum = 0;
	for (i = 0; i < (unsigned int)PKT_POS_CHKSUM; i++)
		w_nCalcSum += g_Packet[i];
	if (w_nCalcSum != PKT_VAL_CHKSUM)
		return ERR_FAIL;
	
	// decrypt data
	w_nTotalSize = 0;
	w_nEncSize = (unsigned int)PKT_VAL_DATA_LEN;
	for (i = 0; i < w_nEncSize; i += CRYPT_DEF_RSA_BYTE)
	{
		// rsa decrypt
		w_nRet = crt_user_RSADec(&g_Packet[PKT_POS_ENC_DATA + i], CRYPT_DEF_RSA_BYTE, w_pCryptTempBuf, &w_nDecSize, g_pCryptN, g_pCryptD);
		if (w_nRet != CRT_SUCCESS)
			return ERR_FAIL;

		// remove padding
		crt_user_RemovePaddingBlock(w_pCryptTempBuf, w_nDecSize, CRYPT_DEF_RSA_SPLIT_SIZE);
		
		// set packet buf
		memcpy(&g_Packet[PKT_POS_DEC_DATA + w_nTotalSize], w_pCryptTempBuf, CRYPT_DEF_RSA_SPLIT_SIZE);
		w_nTotalSize += CRYPT_DEF_RSA_SPLIT_SIZE;
	}
	
	return ERR_SUCCESS;
}

