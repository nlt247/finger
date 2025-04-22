// crypt_user.h

#ifndef	__CRYPT_USER_H_
#define	__CRYPT_USER_H_


// includes --------------------------------------------------------------------


// defines ---------------------------------------------------------------------
#define	CRYPT_DEF_RSA_BITS				512
#define	CRYPT_DEF_RSA_BYTE				(CRYPT_DEF_RSA_BITS / 8)
#define CRYPT_DEF_RSA_SPLIT_SIZE		(CRYPT_DEF_RSA_BYTE - 4)


// exported variables ----------------------------------------------------------
// cryptor keys
extern unsigned char g_pCryptN[];
extern unsigned char g_pCryptE[];
extern unsigned char g_pCryptD[];
extern unsigned char g_pCryptRemoteN[];
extern unsigned char g_pCryptRemoteE[];


// exported functions ----------------------------------------------------------
int crt_user_Init(void);

int crt_user_RSAKeyGen(
	unsigned char *p_pN, // size = CRYPT_DEF_RSA_BYTE
	unsigned char *p_pE, // size = CRYPT_DEF_RSA_BYTE (Public Key)
	unsigned char *p_pD // size = CRYPT_DEF_RSA_BYTE (Private Key)
);

int crt_user_RSAEnc(
	unsigned char *p_pPlainBuf, // Original Data(In)
	unsigned int p_nPlainBufSize,
	unsigned char *p_pCipherBuf, // Encrypted Data(Out)
	unsigned int *p_nCipherBufSize,
	unsigned char *p_pN,
	unsigned char *p_pE // Public Key
);

int crt_user_RSADec(
	unsigned char *p_pCipherBuf, // Encrypted Data(In)
	unsigned int p_nCipherBufSize,
	unsigned char *p_pPlainBuf, // Decrypted Data(Out)
	unsigned int *p_nPlainBufSize,
	unsigned char *p_pN,
	unsigned char *p_pD // Private Key
);

int crt_user_PaddingBlock(
	unsigned char *p_pBuf,
	unsigned int p_nBufSize,
	unsigned int p_nBlockSize
);

int crt_user_RemovePaddingBlock(
	unsigned char *p_pBlockBuf,
	unsigned int p_nBlockSize,
	unsigned int p_nBufSize
);

void SetCryptState(unsigned char p_bCryptState);
unsigned char GetCryptState(void);


// return values ---------------------------------------------------------------
#define	CRT_SUCCESS						0
#define	CRT_FAIL						1
#define	CRT_INVALID_PARAM				2


#endif //!__CRYPT_USER_H_
