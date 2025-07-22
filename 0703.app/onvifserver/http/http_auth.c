

#include "sys_inc.h"
#include "rfc_md5.h"
#include "http_auth.h"


/***************************************************************************************/

/* calculate H(A1) as per spec */
void DigestCalcHA1(
	IN const char * pszAlg,
	IN const char * pszUserName,
	IN const char * pszRealm,
	IN const char * pszPassword,
	IN const char * pszNonce,
	IN const char * pszCNonce,
	OUT HASHHEX SessionKey
)
{
	MD5_CTX Md5Ctx;
	HASH HA1;

	MD5Init(&Md5Ctx);
	MD5Update(&Md5Ctx, (uint8 *)pszUserName, (uint32)strlen(pszUserName));
	MD5Update(&Md5Ctx, (uint8 *)(":"), 1);
	MD5Update(&Md5Ctx, (uint8 *)pszRealm, (uint32)strlen(pszRealm));
	MD5Update(&Md5Ctx, (uint8 *)(":"), 1);
	MD5Update(&Md5Ctx, (uint8 *)pszPassword, (uint32)strlen(pszPassword));
	MD5Final(HA1, &Md5Ctx);

	if (strcmp(pszAlg, "md5-sess") == 0)
	{
		MD5Init(&Md5Ctx);
		MD5Update(&Md5Ctx, HA1, HASHLEN);
		MD5Update(&Md5Ctx, (uint8 *)(":"), 1);
		MD5Update(&Md5Ctx, (uint8 *)pszNonce, (uint32)strlen(pszNonce));
		MD5Update(&Md5Ctx, (uint8 *)(":"), 1);
		MD5Update(&Md5Ctx, (uint8 *)pszCNonce, (uint32)strlen(pszCNonce));
		MD5Final(HA1, &Md5Ctx);
	};

	BinToHexStr(HA1, SessionKey);
};

void DigestCalcResponseHash(
	IN HASHHEX HA1, /* H(A1) */
	IN const char * pszNonce, /* nonce from server */
	IN const char * pszNonceCount, /* 8 hex digits */
	IN const char * pszCNonce, /* client nonce */
	IN const char * pszQop, /* qop-value: "", "auth", "auth-int" */
	IN const char * pszMethod, /* method from the request */
	IN const char * pszDigestUri, /* requested URL */
	IN HASHHEX HEntity, /* H(entity body) if qop="auth-int" */
	OUT HASH  RespHash /* request-digest or response-digest */
)
{
	MD5_CTX Md5Ctx;
	HASH HA2;
	HASHHEX HA2Hex;

	// calculate H(A2)
	MD5Init(&Md5Ctx);
	MD5Update(&Md5Ctx, (uint8 *)pszMethod, (uint32)strlen(pszMethod));
	MD5Update(&Md5Ctx, (uint8 *)(":"), 1);
	MD5Update(&Md5Ctx, (uint8 *)pszDigestUri, (uint32)strlen(pszDigestUri));

	if (strcmp(pszQop, "auth-int") == 0)
	{
		MD5Update(&Md5Ctx, (uint8 *)(":"), 1);
		MD5Update(&Md5Ctx, (uint8 *)(&HEntity[0]), HASHHEXLEN);
	};

	MD5Final(HA2, &Md5Ctx);
	BinToHexStr(HA2, HA2Hex);

	// calculate response
	MD5Init(&Md5Ctx);
	MD5Update(&Md5Ctx, (uint8 *)(&HA1[0]), HASHHEXLEN);
	MD5Update(&Md5Ctx, (uint8 *)(":"), 1);
	MD5Update(&Md5Ctx, (uint8 *)pszNonce, (uint32)strlen(pszNonce));
	MD5Update(&Md5Ctx, (uint8 *)(":"), 1);

	if (*pszQop)
	{
		MD5Update(&Md5Ctx, (uint8 *)pszNonceCount, (uint32)strlen(pszNonceCount));
		MD5Update(&Md5Ctx, (uint8 *)(":"), 1);
		MD5Update(&Md5Ctx, (uint8 *)pszCNonce, (uint32)strlen(pszCNonce));
		MD5Update(&Md5Ctx, (uint8 *)(":"), 1);
		MD5Update(&Md5Ctx, (uint8 *)pszQop, (uint32)strlen(pszQop));
		MD5Update(&Md5Ctx, (uint8 *)(":"), 1);
	};

	MD5Update(&Md5Ctx, (uint8 *)(&HA2Hex[0]), HASHHEXLEN);
	MD5Final(RespHash, &Md5Ctx);
};

void DigestCalcResponse(
	IN HASHHEX HA1, /* H(A1) */
	IN const char * pszNonce, /* nonce from server */
	IN const char * pszNonceCount, /* 8 hex digits */
	IN const char * pszCNonce, /* client nonce */
	IN const char * pszQop, /* qop-value: "", "auth", "auth-int" */
	IN const char * pszMethod, /* method from the request */
	IN const char * pszDigestUri, /* requested URL */
	IN HASHHEX HEntity, /* H(entity body) if qop="auth-int" */
	OUT HASHHEX Response /* request-digest or response-digest */
)
{
	HASH RespHash;

    DigestCalcResponseHash(HA1, pszNonce, pszNonceCount, pszCNonce, pszQop, pszMethod, pszDigestUri, HEntity, RespHash);
	BinToHexStr(RespHash, Response);
};

BOOL DigestAuthProcess(HD_AUTH_INFO * p_auth, HD_AUTH_INFO * p_lauth, const char * method, const char * password)
{

	return FALSE;
}





