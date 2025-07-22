
#include "common.h"
#include "vht_auth.h"




//------------------------------------------------------------
//Base64 encoding/decoding.


#define FF_ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))



/* ---------------- private code */
static const uint8_t map2[] =
{
    0x3e, 0xff, 0xff, 0xff, 0x3f, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x01,
    0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1a, 0x1b,
    0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
    0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
    0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33
};

static int
base64_decode(uint8_t *out, const char *in, int out_size)
{
    int i, v;
    uint8_t *dst = out;

    v = 0;
    for (i = 0; in[i] && in[i] != '='; i++) {
        unsigned int index= in[i]-43;
        if (index>=FF_ARRAY_ELEMS(map2) || map2[index] == 0xff)
            return -1;
        v = (v << 6) + map2[index];
        if (i & 3) {
            if (dst - out < out_size) {
                *dst++ = v >> (6 - 2 * (i & 3));
            }
        }
    }

    return dst - out;
}


/*****************************************************************************
* b64_encode: Stolen from VLC's http.c.
* Simplified by Michael.
* Fixed edge cases and made it work from data (vs. strings) by Ryan.
*****************************************************************************/
// Calculate the output size needed to base64-encode x bytes.
#define BASE64_SIZE(x)  (((x)+2) / 3 * 4 + 1)

static char *
base64_encode(char *out, int out_size, const uint8_t *in, int in_size)
{
    static const char b64[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char *ret, *dst;
    unsigned i_bits = 0;
    int i_shift = 0;
    int bytes_remaining = in_size;

    if (in_size >= UINT_MAX / 4 ||
        out_size < BASE64_SIZE(in_size))
        return NULL;
    ret = dst = out;
    while (bytes_remaining) {
        i_bits = (i_bits << 8) + *in++;
        bytes_remaining--;
        i_shift += 8;

        do {
            *dst++ = b64[(i_bits << 6 >> i_shift) & 0x3f];
            i_shift -= 6;
        } while (i_shift > 6 || (bytes_remaining == 0 && i_shift > 0));
    }
    while ((dst - ret) & 3)
        *dst++ = '=';
    *dst = '\0';

    return ret;
}


//-------------------------------- Check authorization -------------------------------------------
#define	VHT_MASTER_ID	"Master"

// Get master password
static char *
ipcam_vht_get_master_pwd (char *buf, int bufsz)
{
	snprintf (buf, bufsz, "%02x:%02x:%02x:%02x:%02x:%02x:vht",
			g_main.mac [0], g_main.mac [1], g_main.mac [2], g_main.mac [3], g_main.mac [4], g_main.mac [5]);

	return buf;
}


// Get auth64 encryption string from id/pwd
// @auth64: Result auth will be stored here.
// @sz : sizeof auth64
static char *
ipcam_vht_get_auth64 (char *uid, char *pwd, char *auth64, int sz)
{
	char tmp [256];

	snprintf (tmp, sizeof (tmp), "%s:%s", uid, pwd);
	base64_encode (auth64, sz, (const uint8_t *)tmp, strlen (tmp));

	return auth64;
}


// Check if the given id/pwd is correct master id/pwd
// Return
//		1, if correct
//		0, if not
int
ipcam_vht_check_master_idpwd (char *uid, char *pwd)
{
	char	tmp [256];

	if (strcmp (uid, VHT_MASTER_ID))
		goto Auth64;

	if (strcmp (pwd, ipcam_vht_get_master_pwd (tmp, sizeof (tmp))))
		goto Auth64;

	return 1;

Auth64:
	{
		char uid64[256];
		char pwd64[256];
		char master_id[64];
		char master_pwd[64];
		sprintf(master_id, VHT_MASTER_ID);
		ipcam_vht_get_master_pwd(master_pwd, sizeof(master_pwd));

		base64_encode (uid64, sizeof(uid64), (const uint8_t *)master_id, strlen (master_id));
		base64_encode (pwd64, sizeof(pwd64), (const uint8_t *)master_pwd, strlen (master_pwd));

		if (strcmp (uid, uid64))
			goto Fail;

		if (strcmp (pwd, pwd64))
			goto Fail;

		return 1;
	}
Fail:
	return 0;

}


// Check if the given auth64 is correct master auth64
// Return
//		1, if correct
//		0, if not
int
ipcam_vht_check_master_auth64 (char *auth64)
{
	char	pwd [256];
	char	tmp [256];

	ipcam_vht_get_auth64 (VHT_MASTER_ID, ipcam_vht_get_master_pwd (pwd, sizeof (pwd)), tmp, sizeof (tmp));

	if (strcmp (auth64, tmp))
		return 0;

	return 1;
}
