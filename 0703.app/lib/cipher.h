
#ifndef __CHIPHER_H__
#define __CHIPHER_H__


#define   C_CM_IM_CIP_OUT_MAX                 (32*4+2)

//  ID/PWD 인증용 해시 함수 (암호화O, 복호X)
extern char *cm_ci_enc2txt(char *pzOut, void *pIn, int cbIn);
	//  pIn     : [IN] 입력 문자열 또는 임의의 데이터, 길이에 제한없음.
	//  cbIn    : [IN] 멀티바이트 문자열의 경우 null문자 포함개수, 임의의 데이터일 경우 데이터의 바이트수
	//  pzOut   : [OU] C_CM_IM_CIP_OUT_MAX(130) byte 이상의 여유 공간이 있어야 합니다.
	//  return  : pzOut과 동일 포인터 값을 반환

#endif
