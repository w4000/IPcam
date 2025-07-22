#ifndef __SETUP_ENCRYPTION_H__
#define __SETUP_ENCRYPTION_H__

#include "setup.h"

extern int read_enc_setup_basic(_CAMSETUP *setup);
extern int read_enc_setup_ext(_CAMSETUP_EXT *setup_ext);
extern int read_enc_setup(_CAMSETUP *setup, _CAMSETUP_EXT *setup_ext);

extern int write_enc_setup_basic(_CAMSETUP *setup);
extern int write_enc_setup_ext(_CAMSETUP_EXT *setup_ext);
extern int write_enc_setup(_CAMSETUP *setup, _CAMSETUP_EXT *setup_ext);

extern int write_htdigest(_CAMSETUP *setup);

int encrypt_file(const char* dest, const char* src);
int decrypt_file(const char* dest, const char* src);


#endif /*__SETUP_ENCRYPTION_H__*/
