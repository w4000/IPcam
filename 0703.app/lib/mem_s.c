#define __STDC_WANT_LIB_EXT1__ 1
#include <errno.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

int memset_s(void *s, size_t smax, int c, size_t n)
{
	int err = 0;
	if(s == NULL)
		return EINVAL;
	if(smax > INT32_MAX)
		return E2BIG;

	if(n > INT32_MAX) {
		err = E2BIG;
		n = smax;
	}

	if(n > smax) {
		err = EOVERFLOW;
		n = smax;
	}

	memset(s, c, n);

	return err;
}

int memcpy_s(void *d, size_t dsz, void *s, size_t sz)
{
	int err = 0;
	if(d == NULL || s == NULL)
		return EINVAL;
	if(dsz > INT32_MAX)
		return E2BIG;

	if(sz > INT32_MAX) {
		err = E2BIG;
		sz = dsz;
	}

	if(sz > dsz) {
		err = EOVERFLOW;
		sz = dsz;
	}

	memcpy(d, s, sz);
	
	return err;
}

int memmove_s(void *d, size_t dsz, void *s, size_t sz)
{
	int err = 0;
	if(d == NULL || s == NULL)
		return EINVAL;
	if(dsz > INT32_MAX)
		return E2BIG;

	if(sz > INT32_MAX) {
		err = E2BIG;
		sz = dsz;
	}

	if(sz > dsz) {
		err = EOVERFLOW;
		sz = dsz;
	}

	memmove(d, s, sz);
	
	return err;
}

int strncpy_s(char *d, size_t dsz, char *s, size_t sz)
{
	int err = 0;
	if(d == NULL || s == NULL)
		return EINVAL;
	if(dsz > INT32_MAX)
		return E2BIG;

	if(sz > INT32_MAX) {
		err = E2BIG;
		sz = dsz;
	}

	if(sz > dsz) {
		err = EOVERFLOW;
		sz = dsz;
	}

	strncpy(d, s, sz);
	
	return err;
}


int snprintf_s(char *dest, size_t dmax, const char *fmt, ...)
{
	int ret;
	va_list arg;
	va_start(arg, fmt);	
	ret = snprintf(dest, dmax, fmt, arg);
	va_end(arg);

	return ret;
}
