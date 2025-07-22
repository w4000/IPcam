
#include "common.h"


char *cm_ci_enc2txt(char *pzOut, void *pIn, int cbIn)
{
	t_cm_sha256_ctx oCtx;
	unsigned char aSha[C_CM_IM_SHA256_OUT_SIZ];

	//  sha encoding
	cm_sha256_starts(&oCtx);
	cm_sha256_update(&oCtx, (unsigned char *)pIn, cbIn);
	cm_sha256_finish(&oCtx, aSha);

	//  base64 encoding
	return cm_base64_encode(pzOut, C_CM_IM_CIP_OUT_MAX, aSha, C_CM_IM_SHA256_OUT_SIZ);
}

