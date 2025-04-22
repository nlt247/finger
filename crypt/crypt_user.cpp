// crypt_user.cpp

// includes --------------------------------------------------------------------
#include <string.h>

#include "crypt_user.h"
#include "rsa.h"

using namespace crypto::rsa;


// private variables -----------------------------------------------------------
unsigned char m_bCryptedComm = 0;


// exported variables ----------------------------------------------------------
// cryptor keys
unsigned char g_pCryptN[CRYPT_DEF_RSA_BYTE];
unsigned char g_pCryptE[CRYPT_DEF_RSA_BYTE];
unsigned char g_pCryptD[CRYPT_DEF_RSA_BYTE];
unsigned char g_pCryptRemoteN[CRYPT_DEF_RSA_BYTE];
unsigned char g_pCryptRemoteE[CRYPT_DEF_RSA_BYTE];


// private consts --------------------------------------------------------------
// cryptor keys
const unsigned char g_pHostRSAKeyN[CRYPT_DEF_RSA_BYTE] = {
	0xf8, 0x36, 0x70, 0x75, 0xd7, 0xf0, 0x4d, 0xc0, 0x20, 0x6f, 0xb4, 0x1a, 0x91, 0x23, 0x47, 0x24,
	0x62, 0x1a, 0xea, 0x43, 0xa5, 0x9b, 0xaa, 0x60, 0xd1, 0x89, 0x81, 0x16, 0xb9, 0x9c, 0xa3, 0xc3,
	0x90, 0x13, 0x33, 0xcb, 0xf9, 0xaa, 0xb8, 0x52, 0x3a, 0x34, 0xcd, 0x3c, 0x47, 0x50, 0x0e, 0xd6,
	0xb1, 0x2f, 0xb5, 0x22, 0x7d, 0x87, 0x53, 0x39, 0xe9, 0x17, 0x1e, 0x6f, 0x12, 0x75, 0x77, 0x37
};
const unsigned char g_pHostRSAKeyE[CRYPT_DEF_RSA_BYTE] = {
	0x36, 0x88, 0x03, 0x9c, 0x71, 0x05, 0x8b, 0xd2, 0xfc, 0xdc, 0x0c, 0x86, 0x6d, 0x9c, 0xec, 0x9a,
	0x3d, 0x89, 0xbe, 0x2e, 0xd1, 0xe6, 0xf0, 0x1c, 0x0e, 0xac, 0x21, 0x96, 0x15, 0x68, 0x46, 0x74,
	0x58, 0x00, 0x96, 0x6d, 0xda, 0xbc, 0xd9, 0x5c, 0xa9, 0x38, 0x8c, 0xdf, 0x80, 0xa0, 0x7f, 0x33,
	0x2f, 0x06, 0x48, 0x4f, 0x44, 0xf0, 0xd1, 0x93, 0xf5, 0xd4, 0x98, 0x15, 0x1f, 0xc5, 0x20, 0x1b
};
const unsigned char g_pHostRSAKeyD[CRYPT_DEF_RSA_BYTE] = {
	0x3f, 0xed, 0xb9, 0x52, 0x40, 0xe1, 0x02, 0xb1, 0x61, 0xbd, 0x6f, 0x0f, 0xa8, 0xd5, 0xa9, 0xd8,
	0x61, 0xa7, 0x9a, 0xdf, 0x44, 0xc3, 0xa0, 0xf1, 0xb5, 0x71, 0x1f, 0xd4, 0x25, 0x94, 0xb0, 0x97,
	0x31, 0xa0, 0xaa, 0xcb, 0x03, 0xc8, 0x16, 0x36, 0xfe, 0x32, 0x7f, 0xff, 0x9a, 0xa0, 0x80, 0x21,
	0xb0, 0xac, 0xc4, 0x6f, 0x02, 0xcb, 0x33, 0x49, 0x98, 0x41, 0xde, 0xbb, 0x99, 0x7d, 0xd6, 0x53
};


// exported functions ----------------------------------------------------------
int crt_user_Init(void)
{
	return CRT_SUCCESS;
}

int crt_user_RSAKeyGen(
	unsigned char *p_pN, // size = CRYPT_DEF_RSA_BYTE
	unsigned char *p_pE, // size = CRYPT_DEF_RSA_BYTE (Public Key)
	unsigned char *p_pD // size = CRYPT_DEF_RSA_BYTE (Private Key)
)
{
	// check parameters
	if ((p_pN == NULL) || (p_pE == NULL) || (p_pD == NULL))
		return CRT_INVALID_PARAM;
	
	// key generate
	memcpy(p_pN, g_pHostRSAKeyN, CRYPT_DEF_RSA_BYTE);
	memcpy(p_pE, g_pHostRSAKeyE, CRYPT_DEF_RSA_BYTE);
	memcpy(p_pD, g_pHostRSAKeyD, CRYPT_DEF_RSA_BYTE);
	
	return CRT_SUCCESS;
}

int crt_user_RSAEnc(
	unsigned char *p_pPlainBuf, // Original Data(In)
	unsigned int p_nPlainBufSize,
	unsigned char *p_pCipherBuf, // Encrypted Data(Out)
	unsigned int *p_nCipherBufSize,
	unsigned char *p_pN,
	unsigned char *p_pE // Public Key
)
{
	size_t w_nEncLen;
	
	// check parameters
	if ((p_pPlainBuf == NULL) ||
		(p_nPlainBufSize == 0) ||
		(p_pCipherBuf == NULL) ||
		(p_nCipherBufSize == NULL) ||
		(p_pN == NULL) ||
		(p_pE == NULL))
		return CRT_INVALID_PARAM;

	// make key
	BigInt w_bnKeyN(p_pN, CRYPT_DEF_RSA_BYTE);
	BigInt w_bnKeyE(p_pE, CRYPT_DEF_RSA_BYTE);
	RSAKeyInfo w_pEncKey(w_bnKeyN, w_bnKeyE);
	
	// encrypt buffer
	w_nEncLen = RSA::encrypt(w_pEncKey, p_pPlainBuf, p_nPlainBufSize, p_pCipherBuf);
	
	// return size
	*p_nCipherBufSize = w_nEncLen;
	
	return CRT_SUCCESS;
}

int crt_user_RSADec(
	unsigned char *p_pCipherBuf, // Encrypted Data(In)
	unsigned int p_nCipherBufSize,
	unsigned char *p_pPlainBuf, // Decrypted Data(Out)
	unsigned int *p_nPlainBufSize,
	unsigned char *p_pN,
	unsigned char *p_pD // Private Key
)
{
	size_t w_nDecLen;
	
	// check parameters
	if ((p_pCipherBuf == NULL) ||
		(p_nCipherBufSize == 0) ||
		(p_pPlainBuf == NULL) ||
		(p_nPlainBufSize == NULL) ||
		(p_pN == NULL) ||
		(p_pD == NULL))
		return CRT_INVALID_PARAM;

	// make key
	BigInt w_bnKeyN(p_pN, CRYPT_DEF_RSA_BYTE);
	BigInt w_bnKeyD(p_pD, CRYPT_DEF_RSA_BYTE);
	RSAKeyInfo w_pDecKey(w_bnKeyN, w_bnKeyD);
	
	// decrypt buffer
	w_nDecLen = RSA::decrypt(w_pDecKey, p_pCipherBuf, p_nCipherBufSize, p_pPlainBuf);

	// return size
	*p_nPlainBufSize = w_nDecLen;
	
	return CRT_SUCCESS;
}

int crt_user_PaddingBlock(
	unsigned char *p_pBuf,
	unsigned int p_nBufSize,
	unsigned int p_nBlockSize
)
{
	int i;
	unsigned int w_nPaddingSize;

	// check parameter
	if (p_pBuf == NULL)
		return CRT_INVALID_PARAM;
	if (p_nBlockSize < p_nBufSize)
		return CRT_INVALID_PARAM;

	// check size
	if (p_nBlockSize == p_nBufSize)
		return CRT_SUCCESS;
	if (p_nBufSize == 0)
	{
		memset(p_pBuf, 0x00, p_nBlockSize);
		return CRT_SUCCESS;
	}
	
	// move buffer
	w_nPaddingSize = p_nBlockSize - p_nBufSize;
	w_nPaddingSize = w_nPaddingSize / 2;
	for (i = (int)p_nBufSize - 1; i >= 0; i--)
		p_pBuf[i + w_nPaddingSize] = p_pBuf[i];

	// padding buffer
	for (i = 0; i < (int)w_nPaddingSize; i++)
	{
		p_pBuf[i] = 0x00;
		p_pBuf[p_nBufSize + w_nPaddingSize + i] = 0x00;
	}

	return CRT_SUCCESS;
}

int crt_user_RemovePaddingBlock(
	unsigned char *p_pBlockBuf,
	unsigned int p_nBlockSize,
	unsigned int p_nBufSize
)
{
	unsigned int i;
	unsigned int w_nPaddingSize;

	// check parameter
	if (p_pBlockBuf == NULL)
		return CRT_INVALID_PARAM;
	if (p_nBlockSize < p_nBufSize)
		return CRT_INVALID_PARAM;
	if (p_nBufSize == 0)
		return CRT_INVALID_PARAM;

	// check size
	if (p_nBlockSize == p_nBufSize)
		return CRT_SUCCESS;
	
	// move buffer
	w_nPaddingSize = p_nBlockSize - p_nBufSize;
	w_nPaddingSize = w_nPaddingSize / 2;
	for (i = 0; i < p_nBufSize; i++)
		p_pBlockBuf[i] = p_pBlockBuf[i + w_nPaddingSize];

	return CRT_SUCCESS;
}

void SetCryptState(unsigned char p_bCryptState)
{
	m_bCryptedComm = p_bCryptState;
}

unsigned char GetCryptState(void)
{
	return m_bCryptedComm;
}
