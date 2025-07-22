/*
Copyright (c) 2003-2010, Mark Borgerding

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the author nor the names of any contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifdef __KERNEL__
#else
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#endif
#define MAX_FFT_SIZE    (1024)

#define FIXED_POINT  (32)
#ifdef FIXED_POINT
#if (FIXED_POINT == 32)
#define kfft_type int
#else
#define kfft_type short
#endif
#else
#ifndef kfft_type
/*  default is float */
#define kfft_type float
# endif
#endif

struct kfft_data {
	kfft_type r;
	kfft_type i;
};

struct kfft_data_16 {   // for 16bit cos,sin table
	short r;
	short i;
};

// e.g. an fft of length 128 has 4 factors as far as kissfft is concerned 4*4*4*2
#define MAXFACTORS 5
struct kfft_info {
	int nfft;
	int inverse;
	int factors[2 * MAXFACTORS];
	struct kfft_data_16 twiddles[1];    // for 16bit cos,sin table
};

// --------------------------------------------------------------------------
//  Explanation of macros dealing with complex math:
//
//   C_MUL(m,a,b)         : m = a*b
//   C_FIXDIV( c , div )  : if a fixed point impl., c /= div. noop otherwise
//   C_SUB( res, a,b)     : res = a - b
//   C_SUBFROM( res , a)  : res -= a
//   C_ADDTO( res , a)    : res += a
// --------------------------------------------------------------------------
#ifdef FIXED_POINT
#if (FIXED_POINT==32)
#define FRACBITS 31
#define SAMPPROD long long
#define SAMP_MAX 2147483647     // 0x7fffffff
#else
#define FRACBITS 15
#define SAMPPROD int
#define SAMP_MAX 32767
#endif

#define SAMP_MIN -SAMP_MAX

#define KFFT_COS(phase)  floor(.5+SAMP_MAX * cos (phase))
#define KFFT_SIN(phase)  floor(.5+SAMP_MAX * sin (phase))
#define HALF_OF(x) ((x)>>1)

#if defined(CHECK_OVERFLOW)
#define CHECK_OVERFLOW_OP(a,op,b)  \
	if ( (SAMPPROD)(a) op (SAMPPROD)(b) > SAMP_MAX || (SAMPPROD)(a) op (SAMPPROD)(b) < SAMP_MIN ) { \
		fprintf(stderr,"WARNING:overflow @ " __FILE__ "(%d): (%d " #op" %d) = %ld\n",__LINE__,(a),(b),(SAMPPROD)(a) op (SAMPPROD)(b) );  }
#endif

#define smul(a,b) ( (SAMPPROD)(a)*(b) )
#define sround( x )  (kfft_type)( ( (x) + (1<<(FRACBITS-1)) ) >> FRACBITS )
#define sround_16( x )  (kfft_type)( ( (x) + (1<<(15-1)) ) >> 15 )

//#define sround_16( x )  (kfft_type)( ( (x) + (1<<(16-1)) ) >> 16 )

#define S_MUL(a,b) sround( smul(a,b) )
#define S_MUL_16(a,b) sround_16( smul(a,b) )

//#define S_MUL_16(a,b) sround_16( smul(a,b) )

#define C_MUL(m,a,b) \
	do{ (m).r = sround( smul((a).r,(b).r) - smul((a).i,(b).i) ); \
		(m).i = sround( smul((a).r,(b).i) + smul((a).i,(b).r) ); }while(0)
#define C_MUL_16(m,a,b) \
	do{ (m).r = sround_16( smul((a).r, (b).r) - smul((a).i, (b).i) ); \
		(m).i = sround_16( smul((a).r, (b).i) + smul((a).i, (b).r) ); }while(0)

#define DIVSCALAR(x,k) \
	(x) = sround( smul(  x, SAMP_MAX/k ) )
#define C_FIXDIV(c,div) \
	do {    DIVSCALAR( (c).r , div);  \
		DIVSCALAR( (c).i  , div); }while (0)

#define SHIFTSCALAR(x,k) \
	(x) = sround( smul(  x, SAMP_MAX>>k ) )
#define C_FIXSHIFT(c,shift) \
	do {    SHIFTSCALAR( (c).r , shift);  \
		SHIFTSCALAR( (c).i  , shift); }while (0)

#define C_MULBYSCALAR( c, s ) \
	do{ (c).r =  sround( smul( (c).r , s ) ) ;\
		(c).i =  sround( smul( (c).i , s ) ) ; }while(0)
#define C_MULBYSCALAR_16( c, s ) \
	do{ (c).r =  sround_16( smul( (c).r , s ) ) ;\
		(c).i =  sround_16( smul( (c).i , s ) ) ; }while(0)

#else  /* not FIXED_POINT*/

#define KFFT_COS(phase) (kfft_type) cos(phase)
#define KFFT_SIN(phase) (kfft_type) sin(phase)
#define HALF_OF(x) ((x)*.5)

#define S_MUL(a,b) ( (a)*(b) )
#define C_MUL(m,a,b) \
	do{ (m).r = (a).r*(b).r - (a).i*(b).i;\
		(m).i = (a).r*(b).i + (a).i*(b).r; }while(0)
#define C_FIXDIV(c,div) /* NOOP */
#define C_MULBYSCALAR( c, s ) \
	do{ (c).r *= (s);\
		(c).i *= (s); }while(0)
#endif

#ifndef CHECK_OVERFLOW_OP
#define CHECK_OVERFLOW_OP(a,op,b) /* noop */

#define  C_ADD( res, a,b)\
	do { \
		CHECK_OVERFLOW_OP((a).r,+,(b).r)\
		CHECK_OVERFLOW_OP((a).i,+,(b).i)\
		(res).r=(a).r+(b).r;  (res).i=(a).i+(b).i; \
	}while(0)
#define  C_SUB( res, a,b)\
	do { \
		CHECK_OVERFLOW_OP((a).r,-,(b).r)\
		CHECK_OVERFLOW_OP((a).i,-,(b).i)\
		(res).r=(a).r-(b).r;  (res).i=(a).i-(b).i; \
	}while(0)
#define C_ADDTO( res , a)\
	do { \
		CHECK_OVERFLOW_OP((res).r,+,(a).r)\
		CHECK_OVERFLOW_OP((res).i,+,(a).i)\
		(res).r += (a).r;  (res).i += (a).i;\
	}while(0)

#define C_SUBFROM( res , a)\
	do {\
		CHECK_OVERFLOW_OP((res).r,-,(a).r)\
		CHECK_OVERFLOW_OP((res).i,-,(a).i)\
		(res).r -= (a).r;  (res).i -= (a).i; \
	}while(0)
#endif




struct kfft_info *kfft_alloc(struct kfft_info *st, int nfft, int inverse_fft);
void _kiss_fft(struct kfft_info *cfg, const struct kfft_data *fin, struct kfft_data *fout);

