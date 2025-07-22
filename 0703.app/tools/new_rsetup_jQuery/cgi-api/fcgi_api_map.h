#ifndef __FCGI_API_MAP__
#define __FCGI_API_MAP__

#include "fcgi_common.h"
#include "api_system_information.h"
#ifdef NARCH
#include "api_video_quality_iq.h"
#include "api_video_quality_ae.h"
#include "api_video_quality_blc.h"
#include "api_video_quality_awb.h"
#include "api_video_quality_dn.h"
#endif


#define MAKE_API_MAP(api) {#api, api}//w4000 api

const static struct {
	const char *name;
	int (*func)(char *, char *, int, char *, char *);
} api_map[] = {
	// MAKE_API_MAP(api_echo),
	MAKE_API_MAP(api_system_information),
	MAKE_API_MAP(api_system_factory),
	MAKE_API_MAP(api_system_factory_mac),

	MAKE_API_MAP(api_video_quality_iq),
	MAKE_API_MAP(api_video_quality_iq_range),
	MAKE_API_MAP(api_video_quality_iq_default),
	MAKE_API_MAP(api_video_quality_iq_all),
	MAKE_API_MAP(api_video_quality_ae),
	MAKE_API_MAP(api_video_quality_ae_default),
	MAKE_API_MAP(api_video_quality_ae_all),
	MAKE_API_MAP(api_video_quality_blc),
	MAKE_API_MAP(api_video_quality_blc_range),
	MAKE_API_MAP(api_video_quality_blc_default),
	MAKE_API_MAP(api_video_quality_blc_all),
	MAKE_API_MAP(api_video_quality_awb),
	MAKE_API_MAP(api_video_quality_awb_range),
	MAKE_API_MAP(api_video_quality_awb_default),
	MAKE_API_MAP(api_video_quality_awb_all),
	MAKE_API_MAP(api_video_quality_dn),
	MAKE_API_MAP(api_video_quality_dn_range),
	MAKE_API_MAP(api_video_quality_dn_default),
	MAKE_API_MAP(api_video_quality_dn_all),


};

#endif /*__FCGI_API_MAP__*/
