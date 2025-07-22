#include "common.h"
#include "file/ssfsearch.h"
#include "rtsp/RTSP_common.h"


#define MAX_LIV_FRAME_IN_LIST	(35*3)	


#define _DIRECT_WRITE_

#ifndef _DIRECT_WRITE_
char g_write_buf[5][MAX_WRITE_BUF_SZ];
int  g_write_buf_used[5];
int  g_write_buf_stack = 0;
#endif

const 	GUID	h265_clsid = {0xc35fa882,0xdb32,0x44a0,{0x87,0xe,0xeb,0xc5,0x8a,0xaa,0xa9,0x59}};
const 	GUID	h264_clsid = {0x7e098aae,0x9e8e,0x4d3f,{0xb9,0x69,0x08,0xb9,0xca,0xf2,0x6d,0xa1}};		
const 	GUID	jpeg_clsid = {0xaabbccdd,0x9e8f,0x4d3e,{0xb9,0x69,0x08,0xb9,0xca,0xf2,0x6d,0xa1}};		

char * get_propobj (char * ssfhdr, int len, dword objid) {

#define MAX_PROPERTY_OBJECT	12

	char *start = NULL;
	char *end = NULL;
	int i=0;

	assert (ssfhdr);

	start = ssfhdr + sizeof (SSFIDENT) + sizeof (SSFHEADER);
	end = ssfhdr + len;

	for (i = 0; start < end; start += LE32_TO_HOST (*((dword *)(start+sizeof(dword))))) {
		if (LE32_TO_HOST (*((dword *)start)) == objid) {
			return start;
		}

		if (i > MAX_PROPERTY_OBJECT)
			break;
		i ++;
	}
	return NULL;
}

void ssfbuild_close(ST_SSFBUILD * build) {
	assert (build);


	if (build->fd_ssf >= 0 && HDD_RW_ENABLE) {
#ifdef _DIRECT_WRITE_
		unsigned filesz;


		/*
		filesz = ftell(build->fp_ssf);
		fflush(build->fp_ssf);
		fclose(build->fp_ssf);
		build->fp_ssf = NULL;
		*/

		pthread_mutex_lock(&g_file.ssf_write_mutex);
			SSFBUILD_FLUSH_SSFHDR_2(build);

			filesz = getfilesz(build->fd_ssf);
			fsync (build->fd_ssf);
			close (build->fd_ssf);
			build->fd_ssf = -1;
			build->fd_ssf_pos = 0;
		pthread_mutex_unlock(&g_file.ssf_write_mutex);



		if (filesz <= SSFHDR_MINSZ) {
			remove_file (build->ssfname, 1);
		}
#else
		char* ssfname = _mem_malloc2(strlen(build->ssfname) + 2);
		strcpy(ssfname, build->ssfname);
		send_msg (ENCODER, FILEMAN, _CAMMSG_CLOSE_FILE, build->fd_ssf, (int)ssfname, 0);
#endif

	}

	if (build->fd_idx >= 0 && HDD_RW_ENABLE) {

		unsigned filesz;



		pthread_mutex_lock(&g_file.ssf_write_mutex);
			SSFBUILD_FLUSH_IDXHDR_2(build);

			filesz = getfilesz(build->fd_idx);
			fsync(build->fd_idx);
			close(build->fd_idx);
			build->fd_idx = -1;
			build->fd_idx_pos = 0;
		pthread_mutex_unlock(&g_file.ssf_write_mutex);

		if (filesz <= IDXHDR_MINSZ) {
			remove_file (build->idxname, 1);
		}

	}

	if (HDD_RW_ENABLE)
	{
		pthread_mutex_lock(&g_file.ssf_write_mutex);
			SSFBUILD_FLUSH_RDXHDR (&g_file.rdx_build[g_file.rdx_index]);
		pthread_mutex_unlock(&g_file.ssf_write_mutex);
	}

	pthread_mutex_destroy (& build->mutex);

	ssfbuild_init(build);
}

void rdxbuild_close(ST_RDXBUILD * build) {

	if (build->fd_rdx >= 0) {
		unsigned filesz;

		filesz = getfilesz(build->fd_rdx);
		fsync(build->fd_rdx);
		close(build->fd_rdx);
		build->fd_rdx = -1;
	}

	pthread_mutex_destroy (& build->mutex);

	rdxbuild_init(build);
}

int ssf_build (ST_SSFBUILD *build, const char *ssfroot, struct tm *creatime, unsigned ch, PAYLOAD_TYPE_E enType, unsigned xres, unsigned yres,
		unsigned samp_rate, unsigned bits_rate) {

	assert (build);
	assert (build->ssf == -1);
	assert (ssfroot);
	assert (creatime);
	assert (VALIDCH (ch));


#if 0
#else

	sprintf (build->ssfname, "%s/%04d%02d%02d/%02d%02d%02d%02d%c%s", ssfroot,
		1900+creatime->tm_year, creatime->tm_mon+1, creatime->tm_mday,
		creatime->tm_hour, creatime->tm_min, creatime->tm_sec,
		ch, (creatime->tm_isdst > 0)?'D':'S', SSF_SSF_EXTENSION);


	if (HDD_RW_ENABLE) {
		int create_retry = 10;

create_file:

		g_file.ssf_buf_pos[g_file.ssf_buf_idx] = 0;

		pthread_mutex_lock(&g_file.ssf_write_mutex);
			build->fd_ssf = open (build->ssfname, O_RDWR|O_CREAT, 0644);
		pthread_mutex_unlock(&g_file.ssf_write_mutex);

		if (build->fd_ssf < 0)
		{

			if (errno == ENOSPC)
				return -ERR_NOSPC;

			if (create_retry-- <= 0)
				return -ERR_IO;
			else
				goto create_file;
		}

		lseek (build->fd_ssf, 0, SEEK_SET);
	}
	else {
		build->fd_ssf = -1;
	}
#endif

	int sz;
	sz = ssf_hdr_build(build, creatime, ch, enType, xres, yres, samp_rate, bits_rate);

	if (sz > 0)
	{

		assert (build->filep);
		assert (build->videop);
		assert (build->indexp);
		assert (build->dataobj);

		if (HDD_RW_ENABLE ) {

			int ret = -1;

			pthread_mutex_lock(&g_file.ssf_write_mutex);
				ret = write (build->fd_ssf, build->ssfhdrs, sz);
			pthread_mutex_unlock(&g_file.ssf_write_mutex);

			if (ret < 0) {
				printf("write\n");
				return -1;
			}

			build->fd_ssf_pos = lseek(build->fd_ssf, 0, SEEK_CUR);
			build->fd_ssf_open_first = 1;
		}
	}

	return sz;
}

static int get_encmode()
{
	static int encmode = -1;
	encmode = ENCRYPT_NONE;

	return encmode;
}

int ssf_hdr_build (ST_SSFBUILD *build, struct tm *creatime, unsigned ch, PAYLOAD_TYPE_E enType, unsigned xres, unsigned yres, unsigned samp_rate, unsigned bits_rate)
{
	SSFHEADER	*gblhdr = NULL;
	FILEPROP	*filep = NULL;
	VIDEOPROP	*videop = NULL;
	IFRAMEPROP	*indexp = NULL;
	DATAHDR		*dataobj = NULL;
	char * hdrs = build->ssfhdrs, *tmp = NULL;
	int sz = 0;

	assert (build);
	assert (build->ssf == -1);
	assert (creatime);
	assert (VALIDCH (ch));

	sprintf (hdrs, " ");

	gblhdr = (SSFHEADER *) (hdrs + sizeof (SSFIDENT));
	gblhdr->id = HOST_TO_LE32 (SSFID_GLOBALHDR);
	gblhdr->size = HOST_TO_LE32 (0);	

	filep = (FILEPROP *)((char *)gblhdr + sizeof (SSFHEADER));
	filep->id = HOST_TO_LE32 (SSFID_FILEPROP);
	filep->size = HOST_TO_LE32 (0);
	filep->version = 30;
	memset(filep->site, 0, 8);
	filep->date.sd_gmt = HOST_TO_LE16 (0);
	filep->date.sd_year = HOST_TO_LE16 (creatime->tm_year + 1900);
	filep->date.sd_month = creatime->tm_mon+1;
	filep->date.sd_day = creatime->tm_mday;
	filep->date.sd_hour = creatime->tm_hour;
	filep->date.sd_min = creatime->tm_min;
	filep->date.sd_sec = creatime->tm_sec;
	filep->date.sd_info = 0;
	filep->date.sd_dst = creatime->tm_isdst;

	filep->enc_mode = (g_setup_ext.caminfo_ext[0].encryption)?get_encmode():ENCRYPT_NONE;
	filep->enc_key = (get_encmode()==ENCRYPT_NONE) ? 0 : HOST_TO_LE32(bf_get_key());
	filep->pwd_crc = HOST_TO_LE32(0);
	filep->duration = HOST_TO_LE32(0);		
	filep->pair = 2;			
	filep->security = SECURITY_USER;
	filep->lang = 0; 
	filep->channel = ch;
	filep->title_len = 30;
	{
#ifdef ARCH_ARM
		memset(filep->title, 0, 32);
		strncpy(filep->title, g_setup.cam [ch].title, filep->title_len);
		tmp = filep->var;
		*tmp = 0;
		tmp += sizeof(byte);	
		tmp += sizeof(byte);
		tmp += sizeof(byte);
		tmp += sizeof(byte);
#else
#endif
	}
	filep->size = HOST_TO_LE32 (tmp-(char *)filep);


	videop = (VIDEOPROP *)tmp;
	videop->id = HOST_TO_LE32 (SSFID_VIDEOPROP);
	videop->size = 0;

	if(enType == PT_H265) {
		videop->clsid.Data1 = HOST_TO_LE32 (h265_clsid.Data1);
		videop->clsid.Data2 = HOST_TO_LE16 (h265_clsid.Data2);
		videop->clsid.Data3 = HOST_TO_LE16 (h265_clsid.Data3);
		memcpy(videop->clsid.Data4, h265_clsid.Data4, 8);
	}
	else if(enType == PT_H264) {
		videop->clsid.Data1 = HOST_TO_LE32 (h264_clsid.Data1);
		videop->clsid.Data2 = HOST_TO_LE16 (h264_clsid.Data2);
		videop->clsid.Data3 = HOST_TO_LE16 (h264_clsid.Data3);
		memcpy(videop->clsid.Data4, h264_clsid.Data4, 8);
	}
	else if(enType == PT_JPEG) {
		videop->clsid.Data1 = HOST_TO_LE32 (jpeg_clsid.Data1);
		videop->clsid.Data2 = HOST_TO_LE16 (jpeg_clsid.Data2);
		videop->clsid.Data3 = HOST_TO_LE16 (jpeg_clsid.Data3);
		memcpy(videop->clsid.Data4, jpeg_clsid.Data4, 8);
	}
	videop->seq = SSF_VIDEO;
	videop->type = 0;
	videop->xres = HOST_TO_LE16 (xres);
	videop->yres = HOST_TO_LE16 (yres);
	videop->irate = g_setup.cam [ch].gop;	
	{
#ifdef ARCH_ARM
		tmp = videop->var;
		memset(videop->var, 0, 5);
		tmp += 5;
		*tmp = 0;
#else
		tmp = videop->var;
		memset(videop->var, 0, 5); tmp += 5;
		*tmp = 0; tmp+=sizeof(byte);	
#endif
	}

#ifdef __USE_RTSP_SERVER__
	{
        tmp = videop->var;
		*tmp = VIDEOPROP_SPSPPS_SZ;	
		tmp += 1;
        memset (tmp, 0, VIDEOPROP_SPSPPS_SZ);
        tmp += VIDEOPROP_SPSPPS_SZ;
		tmp = __ARCH_ALIGN__ (tmp, typeof (videop->id));
	}
#endif

	videop->size = HOST_TO_LE32 (tmp-(char *)videop);




	indexp = (IFRAMEPROP *)tmp;
	indexp->id = HOST_TO_LE32 (SSFID_IFRAMEPROP);
	indexp->size = 0;
	indexp->offset = HOST_TO_LE32 (0);
	indexp->xsize = 0;
	tmp = indexp->var;
#ifdef ARCH_ARM
	tmp += sizeof(byte);
	tmp += sizeof(byte);
	tmp += sizeof(byte);
#endif
	indexp->size = HOST_TO_LE32 (tmp-(char *)indexp);




	gblhdr->size = HOST_TO_LE32 (tmp-(char *)gblhdr);


	dataobj = (DATAHDR *)tmp;
	dataobj->id = HOST_TO_LE32 (SSFID_DATAHDR);
	dataobj->size = HOST_TO_LE32 (sizeof(DATAHDR));		

	{
		sz = dataobj->dataunit - (char *) hdrs;

		build->ssfhdrsz = sz;


		build->filep = (FILEPROP*)get_propobj (hdrs, sz, SSFID_FILEPROP);
		build->videop = (VIDEOPROP*)get_propobj (hdrs, sz, SSFID_VIDEOPROP);
		build->indexp = (IFRAMEPROP*)get_propobj (hdrs, sz, SSFID_IFRAMEPROP);
		build->dataobj = (DATAHDR*)get_propobj (hdrs, sz, SSFID_DATAHDR);
	}


	return sz;
}


static int idx_build(ST_SSFBUILD *build, const char *ssfroot, struct tm *creatime, unsigned ch) {
	IFRAMEINFO	*ifrminfo = NULL;
	char * hdrs = build->idxhdrs;
	int create_retry = 60;

	assert (build);
	assert (ssfroot);
	assert (creatime);
	assert (VALIDCH (ch));


#if 0
#else
	sprintf(build->idxname, "%s/%04d%02d%02d/%02d%02d%02d%02d%c%s", ssfroot,
		1900+creatime->tm_year, creatime->tm_mon+1, creatime->tm_mday,
		creatime->tm_hour, creatime->tm_min, creatime->tm_sec,
		ch, (creatime->tm_isdst > 0)?'D':'S', SSF_IDX_EXTENSION);

create_file:



	build->fd_idx = -1;
	ifrminfo = (IFRAMEINFO *)hdrs;
	ifrminfo->id = HOST_TO_LE32 (SSFID_IFRAMEINDEX);
	ifrminfo->size = HOST_TO_LE32 (sizeof(IFRAMEINFO));	
	memcpy(&ifrminfo->rindex.createtm, creatime, sizeof(struct tm));
	memset(ifrminfo->rindex.data, 0x00, sizeof(RECORDINDEX));
	ifrminfo->count = HOST_TO_LE16 (0);			

	{
		int sz= sizeof(IFRAMEINFO);
		build->idxhdrsz = sz;

		build->iframep = (IFRAMEINFO *) build->idxhdrs;

		if (HDD_RW_ENABLE ) {
			int ret;

			pthread_mutex_lock(&g_file.ssf_write_mutex);
				ret = write (build->fd_idx, hdrs, sz);
			pthread_mutex_unlock(&g_file.ssf_write_mutex);

			if (ret < 0) {
				perror("write");
				return -1;
			}

			build->fd_idx_pos = sz;
			build->fd_idx_open_first = 1;

		}
	}

	build->chInfo.intra_net = FALSE;
	build->chInfo.intra_ssf = FALSE;

	return 0;
#endif
}

int rdx_build(ST_RDXBUILD *build, const char *rdxfilename) {

	RECORDINFO	*rdxinfo = NULL;
	char * hdrs = build->rdxhdrs;
	boolean rdx_init = FALSE;

	assert (build);
	assert (rdxfilename);



	build->fd_rdx = -1;
	return 0;
}


int ssf_idx_build(ST_SSFBUILD *build, ST_SSFBUILD *last_build, unsigned ch, PAYLOAD_TYPE_E enType, unsigned xres, unsigned yres, unsigned samp_rate, unsigned bits_rate, struct timeval *now) {
	struct tm	creatime;
	unsigned hdd;
	int sz, ret = -1;

	assert (build);
	assert (VALIDCH (ch));

	hdd = g_file.hdd;
	{
		time_t tm;
		tm = now->tv_sec;
		localtime_r(& tm, & creatime);	
	}

	creatime.tm_isdst = g_timer.current.tm_isdst;

	build->cam = ch;
	build->creatime = creatime;
	build->start = 0;


	if ((sz = ssf_build (build, ssfroot[hdd], & build->creatime, ch, enType, xres, yres, samp_rate, bits_rate)) < 0)
		goto err;

	build->chInfo.filesz = sz;

	if ((ret=idx_build (build, ssfroot[hdd], & build->creatime, ch)) < 0)
		goto err;

	{
		long encryptkey;

		encryptkey = FILEPROP_ENCKEY(build->filep);
		BF_Init (&build->ctx, &encryptkey);
	}
	build->numalarm = 0;
	build->recmode = 0;

	return 0;
err:
	ssfbuild_close(build);
	return ret;
}


static int __idx_addrecidx (ST_SSFBUILD * build, __u32 stamp)
{
	int npos, npos_min;
	int addsec;
	unsigned mode;
	struct tm ent = build->iframep->rindex.createtm;
	struct tm mktm = ent;
	boolean is_dst = FALSE;
	int ch = build->filep->channel;

	addsec = (int)(HOST_TO_LE32(stamp) /1000);
	tm_addsec2(&ent, addsec);

	if (ent.tm_hour != mktm.tm_hour) {
		return 0;
	}

	npos = RDX_NPOS_STAMP (SSFBUILD_CH (ch)->creatime, stamp);
	npos_min = RDX_NPOS_MIN_STAMP (SSFBUILD_CH (ch)->creatime, stamp);

	if (g_setup.dst) {
		if (datetime_isdst_end(&ent) && (build->creatime.tm_isdst)) {
				is_dst = TRUE;
		}
	}

	else 
	{
		switch (CAMERA_ENCMODE(ch)) {

		default:
			mode = (is_dst)? 11 : 1;
			break;
		}
	}


	if (build->iframep->rindex.data[npos] < mode)
		build->iframep->rindex.data[npos] = mode;

	if (g_file.rdx_build[g_file.rdx_index].rdxp->data[RDX_NPOS (ch, npos)] < mode)
		g_file.rdx_build[g_file.rdx_index].rdxp->data[RDX_NPOS (ch, npos)] = mode;

	if (g_file.rdx_build[g_file.rdx_index].rdxp->min_data[RDX_NPOS_MIN (ch, npos_min)] < mode)
		g_file.rdx_build[g_file.rdx_index].rdxp->min_data[RDX_NPOS_MIN (ch, npos_min)] = mode;



	return 0;
}

static int __idx_addindx (ST_SSFBUILD * build,  off_t offs, __u32 stamp, int scale, int interval)
{
	IFRAMEINDEX	index;

	assert (build);

	if (build->fd_idx < 0)
		return 0;

	index.offset = HOST_TO_LE32(offs);
	index.timecode = HOST_TO_LE32(stamp);

	index.scale = HOST_TO_LE32(scale);
	index.interval = HOST_TO_LE32(interval);

	pthread_mutex_lock(&g_file.idx_buf_mutex);
	{
		int sz = sizeof (IFRAMEINDEX);
		int idx = g_file.idx_buf_idx;

		if (g_file.idx_buf_pos[idx] + sz < MAX_IDX_BUF_SIZE)
		{
			int chk = 1;
			if (build->fd_idx_open_first == 1)
			{

				if (index.offset != build->ssfhdrsz)
				{
					chk = 0;
				}
				else
				{
					build->fd_idx_open_first = 2;
				}
			}

			if (chk)
			{
				memcpy(g_file.idx_buf[idx] + g_file.idx_buf_pos[idx], (char *)& index, sz);
				g_file.idx_buf_pos[idx] += sz;

				build->fd_idx_pos += sz;

			}
		}
		else
		{
		}
	}
	pthread_mutex_unlock(&g_file.idx_buf_mutex);

	build->iframep->size = HOST_TO_LE32((LE32_TO_HOST(build->iframep->size)+sizeof(IFRAMEINDEX)));
	build->iframep->count = HOST_TO_LE16((LE16_TO_HOST(build->iframep->count)+1));

	__idx_addrecidx (build, stamp);

	return 0;
}

static int __ssf_addunit (ST_SSFBUILD * build,  char * buf, size_t sz, VENC_STREAM_S *pstStream, int hdrsz, __u32 stamp, off_t * offs, int bIsIFrame)
{
	static int full_cnt = 0;
	int ret = 0;

	assert (build);
	assert (buf);

	if (build->fd_ssf < 0)
		return 0;

	{
		* offs = build->fd_ssf_pos;

		pthread_mutex_lock(&g_file.ssf_buf_mutex);
		int idx = g_file.ssf_buf_idx;
		if (g_file.ssf_buf_pos[idx] + sz < MAX_SSF_BUF_SIZE)
		{
			int chk = 1;
			if (build->fd_ssf_open_first == 1)
			{

				if (!bIsIFrame)
				{
					chk = 0;
				}
				else
				{
					build->fd_ssf_open_first = 2;
				}
			}

			if (chk)
			{
				if (pstStream)
				{
					int i, len2 = 0;

					memcpy(g_file.ssf_buf[idx] + g_file.ssf_buf_pos[idx], buf, hdrsz);

					for (i = 0; i < pstStream->u32PackCount; i++)
					{
						memcpy(g_file.ssf_buf[idx] + g_file.ssf_buf_pos[idx] + hdrsz + len2, pstStream->pstPack[i].pu8Addr, pstStream->pstPack[i].u32Len);
						len2 += pstStream->pstPack[i].u32Len;
					}
				}
				else
					memcpy(g_file.ssf_buf[idx] + g_file.ssf_buf_pos[idx], buf, sz);

				g_file.ssf_buf_pos[idx] += sz;
				if (g_file.ssf_buf_pos[idx] > g_file.min_ssf_buf_size)
					g_file.ssf_buf_pos_flush[idx] = g_file.ssf_buf_pos[idx];


				build->fd_ssf_pos += sz;
				build->chInfo.filesz += sz;
				build->dataobj->size = HOST_TO_LE32((LE32_TO_HOST(build->dataobj->size) +sz));
			}

			full_cnt = 0;
		}
		else
		{
			if (full_cnt++ > 500)
			{
				send_msg (ENCODER, MAIN, _CAMMSG_REBOOT, 7, 0, 0);
			}
			else
			{
			}

			build->chInfo.intra_ssf = FALSE;
		}
		pthread_mutex_unlock(&g_file.ssf_buf_mutex);

		build->filep->duration = HOST_TO_LE32(stamp);
	}

	return ret;
}


#ifdef _STATIC_MEM_ALLOC_
#define FRM_BUF_SIZE_1	10	// 10
#define FRM_BUF_SIZE_2	10	// 20
#define FRM_BUF_SIZE_3	10	// 30
#define FRM_BUF_SIZE_4	30	// 60
#define FRM_BUF_SIZE_5	40	// 100
#define FRM_BUF_SIZE_6	30	// 130
#define FRM_BUF_SIZE_7	20	// 150
#define FRM_BUF_SIZE_8	10	// 160

static char g_frm_buf_1[FRM_BUF_SIZE_1][5000];
static char g_frm_buf_2[FRM_BUF_SIZE_2][7000];
static char g_frm_buf_3[FRM_BUF_SIZE_3][10000];
static char g_frm_buf_4[FRM_BUF_SIZE_4][30000];
static char g_frm_buf_5[FRM_BUF_SIZE_5][50000];
static char g_frm_buf_6[FRM_BUF_SIZE_6][60000];
static char g_frm_buf_7[FRM_BUF_SIZE_7][70000];
static char g_frm_buf_8[FRM_BUF_SIZE_8][102400];

#define MAX_FRM_BUF_CNT		8
#define MAX_FRM_BUF_SIZE	(160)	
static int g_frm_buf_size_limit[MAX_FRM_BUF_CNT] = { 5000, 7000, 10000, 30000, 50000, 60000, 70000, 102400 };
static int g_frm_buf_size_array[MAX_FRM_BUF_CNT] = { FRM_BUF_SIZE_1, FRM_BUF_SIZE_2, FRM_BUF_SIZE_3, FRM_BUF_SIZE_4, FRM_BUF_SIZE_5, FRM_BUF_SIZE_6, FRM_BUF_SIZE_7, FRM_BUF_SIZE_8 };
static int g_frm_buf_size_pos[MAX_FRM_BUF_CNT]   = { 0, };
static char* g_frm_buf[MAX_FRM_BUF_SIZE];
static char g_frm_buf_use[MAX_FRM_BUF_SIZE];

void init_frame_buf()
{
	int i, j, p = 0;

	for (i = 0; i < MAX_FRM_BUF_CNT; i++)
	{
		for (j = 0; j < g_frm_buf_size_array[i]; j++)
		{
			switch (i)
			{
				case 0: g_frm_buf[p++] = g_frm_buf_1[j];	break;
				case 1: g_frm_buf[p++] = g_frm_buf_2[j];	break;
				case 2: g_frm_buf[p++] = g_frm_buf_3[j];	break;
				case 3: g_frm_buf[p++] = g_frm_buf_4[j];	break;
				case 4: g_frm_buf[p++] = g_frm_buf_5[j];	break;
				case 5: g_frm_buf[p++] = g_frm_buf_6[j];	break;
				case 6: g_frm_buf[p++] = g_frm_buf_7[j];	break;
				case 7: g_frm_buf[p++] = g_frm_buf_8[j];	break;
			}
		}
	}

	for (i = 0; i < MAX_FRM_BUF_CNT; i++)
	{
		for (j = 0; j < g_frm_buf_size_array[i]; j++)
		{
			switch (i)
			{
				case 0: g_frm_buf[p++] = g_frm_buf_1[j];	break;
				case 1: g_frm_buf[p++] = g_frm_buf_2[j];	break;
				case 2: g_frm_buf[p++] = g_frm_buf_3[j];	break;
				case 3: g_frm_buf[p++] = g_frm_buf_4[j];	break;
				case 4: g_frm_buf[p++] = g_frm_buf_5[j];	break;
				case 5: g_frm_buf[p++] = g_frm_buf_6[j];	break;
				case 6: g_frm_buf[p++] = g_frm_buf_7[j];	break;
				case 7: g_frm_buf[p++] = g_frm_buf_8[j];	break;
			}
		}
	}

	memset(g_frm_buf_1, 0, sizeof(g_frm_buf_1));
	memset(g_frm_buf_2, 0, sizeof(g_frm_buf_2));
	memset(g_frm_buf_3, 0, sizeof(g_frm_buf_3));
	memset(g_frm_buf_4, 0, sizeof(g_frm_buf_4));
	memset(g_frm_buf_5, 0, sizeof(g_frm_buf_5));
	memset(g_frm_buf_6, 0, sizeof(g_frm_buf_6));
	memset(g_frm_buf_7, 0, sizeof(g_frm_buf_7));
	memset(g_frm_buf_8, 0, sizeof(g_frm_buf_8));

	memset(g_frm_buf_use, 0, sizeof(g_frm_buf_use));

	g_frm_buf_size_pos[0] = 0;
	for (i = 1; i < MAX_FRM_BUF_CNT; i++)
	{
		g_frm_buf_size_pos[i] = g_frm_buf_size_pos[i-1] + g_frm_buf_size_array[i-1];
	}

}

struct st_frame* _get_frame_buffer(int size)
{
	int i, j;
	for (i = 0; i < MAX_FRM_BUF_CNT; i++)
	{
		if (size <= g_frm_buf_size_limit[i])
		{
			int p = g_frm_buf_size_pos[i];

			for (j = 0; j < g_frm_buf_size_array[i]; j++)
			{
				if (g_frm_buf_use[p + j] == 0)
				{
					g_frm_buf_use[p + j] = 1;
					return (struct st_frame*)g_frm_buf[p + j];
				}
			}

		}
	}
	
	return (struct st_frame*)_mem_malloc2(size);
}

void _free_frame_buffer(struct st_frame* p)
{
	int i, chk = 0;
	for (i = 0; i < MAX_FRM_BUF_SIZE; i++)
	{
		if ((struct st_frame*)g_frm_buf[i] == p)
		{
			g_frm_buf_use[i] = 0;
			chk = 1;
			break;
		}
	}

	if (chk == 0)
	{
		_mem_free(p);
	}
}
#else
void init_frame_buf()
{
}

inline struct st_frame* _get_frame_buffer(int size)
{
	return _mem_malloc2(size);
}

void _free_frame_buffer(struct st_frame* p)
{
	_mem_free(p);
}
#endif


#define	NONE_SSF 0


int ssf_addunit(ST_SSFBUILD *build, ST_ENCUNIT *unit, VENC_STREAM_S *pstStream)
{
	static int __reboot_chk = 0;

	uint64 filesz;
	int unitsz, ret = 0;
	unsigned stamp;
	__u64 unit_tm;

	char * buf;
	off_t pos = -1;

	assert (build);
	assert (unit);

	if ((filesz = (build->chInfo.filesz + (uint64)unit->sz)) > ((uint64)MAX_SSFFILESZ)) {

		build->chInfo.filesz = 0;

		send_msg (ENCODER, ENCODER, _CAMMSG_RESTART_SSF, 0, 0, 0);
	}

	if (build->start == 0)
		build->start = unit->tm;

#if NONE_SSF
	stamp = 0;
	unit_tm = 0;
	buf = unit->buf;
#else
	stamp = max (0, (int)(unit->tm - build->start));
	unit_tm = unit->tm;
	buf = unit->buf;
#endif
	do {
		char 	*tmp = NULL;
		size_t	dataunitsz, hdrsz;
		struct st_frame	*frame = NULL;

		tmp = buf;
		tmp-=sizeof(byte); *tmp = 0;	
		tmp-=sizeof(byte); *tmp = 0;	

		unitsz = unit->sz;

#ifdef ARCH_ARM
		tmp -= sizeof (dword);
		__ARCH_ASSIGN_ENDIAN__ (* (dword *) tmp, stamp);
#else
		tmp -= sizeof(dword);
		*((dword *)tmp) = HOST_TO_LE32(stamp);
#endif

		tmp -= sizeof(byte);
#if NONE_SSF
		*tmp = (byte)0;							
#else
		*tmp = (byte)(unit->type);				
#endif
		*tmp |= (0 << 5);						


#if NONE_SSF
		*tmp |= (0 << 6);	
		*tmp |= (0 << 7);		
#else
		*tmp |= (((unit->seq == 10)? 1 : 0) << 6);	
		*tmp |= (((unit->seq)? 0 : 1) << 7);		
#endif

#ifdef ARCH_ARM
		tmp -= sizeof (dword);
		dataunitsz = (buf - tmp) + unitsz;
		__ARCH_ASSIGN_ENDIAN__ (* (dword *) tmp, dataunitsz);
#else
		tmp -= sizeof(dword);
		dataunitsz = (buf-tmp) + unitsz;
		*((dword *)tmp) = HOST_TO_LE32(dataunitsz);	
#endif

		hdrsz = dataunitsz - unitsz;

		if (unit->ch == 0)
		{
			if (!build->filep)
			{
				static int __chk = 0;

				if (__chk++ > 100)
				{
					__chk = 0;

					if (__reboot_chk++ > 10)
					{
						send_msg (ENCODER, MAIN, _CAMMSG_REBOOT, 7, 0, 0);
					}
				}

				goto out;
			}
			else
				__reboot_chk = 0;
		}

		do {
			if (g_netsvr.th.state != STAT_RUNNING) {
				break;
			}

			//
			if ((!build->chInfo.intra_net && unit->seq && unit->type == SSF_VIDEO)
					&& (unit->seq != 10)
					) {
			}
			else
			{
				size_t bufsz = sizeof(struct st_frame) + dataunitsz;

				if ((LIVE_FRAME_BUF_CH (unit->ch)->numframe > MAX_LIV_FRAME_IN_LIST))
				{
					live_release_frame(unit->ch, MAX_LIV_FRAME_IN_LIST);
				}

				frame = _get_frame_buffer(bufsz);

				if (frame == NULL)
				{
					break;
				}

				if (pstStream)  
				{
					int i, len2 = 0;

					memcpy(frame->data_unit, tmp, hdrsz);
					for (i = 0; i < pstStream->u32PackCount; i++)
					{
						memcpy(frame->data_unit + hdrsz + len2, pstStream->pstPack[i].pu8Addr, pstStream->pstPack[i].u32Len);
						
						len2 += pstStream->pstPack[i].u32Len;

					}
				}
				else
					memcpy(frame->data_unit, tmp, dataunitsz);

				frame_init(frame, unit_tm, unit->type, -1, unit->seq, hdrsz, unitsz, bufsz, unit->save, unit->scale, unit->interval);
				sync_live_framebuf(frame, unit->ch);
				live_release_frame(unit->ch, MAX_LIV_FRAME_IN_LIST);
			}

			if (!unit->seq)
				build->chInfo.intra_net = TRUE;
		} while (0);

		do {
			if (unit->ch > 0) break;

			if (!unit->save) {
				build->chInfo.intra_ssf = FALSE;
				break;
			}

			if (unit->type == SSF_VIDEO)
			{
				if (g_setup_ext.iframe_only_rec)
				{
					if (unit->seq)	
						break;
				}

				if (!build->chInfo.intra_ssf && unit->seq) {
					if (build->chInfo.prealrm_frms <= 0) 			
						break;
				}
			}

			ret = __ssf_addunit (build, tmp, dataunitsz, pstStream, hdrsz, stamp, & pos, !unit->seq);
			if (ret < 0)
				goto out;

			build->chInfo.savetm = unit_tm;

		} while (0);
	} while (0);

	do {
		if (unit->ch > 0) break;

		if (! unit->save) {
			break;
		}

		if (unit->seq || pos < 0)
			break;


		ret = __idx_addindx (build, pos, stamp, unit->scale, unit->interval);

#ifdef _DIRECT_WRITE_
		//SSFBUILD_FLUSH_SSFHDR (build);
#else
		char* hdrs = _mem_malloc2(build->ssfhdrsz);
		memcpy(hdrs, build->ssfhdrs, build->ssfhdrsz);
		int hdrssz = build->ssfhdrsz;
		send_msg (ENCODER, FILEMAN, _CAMMSG_FLUSH_FILE, build->fd_ssf, (int)hdrs, hdrssz);
#endif

		if (ret < 0)
			goto out;

		build->chInfo.intra_ssf = TRUE;
		build->chInfo.prealrm_frms = 0;
	} while (0);
	ret = 0;

out:
	/*pthread_mutex_unlock (& build->mutex);*/

	return ret;
}


int ssf_addlist (ST_SSFBUILD * build, struct clist * head)
{
	struct clist volatile * tmp;
	struct st_frame * frame = NULL;
	off_t  pos, i_pos = 0;
	int ch, n = 0;
	boolean intra = FALSE;

	assert (build);
	assert (head);

	ch = build->filep->channel;

	foreach_clist_entry (head, tmp)
	{
		frame = clist_entry (tmp, struct st_frame, iframe);

		if (frame->save)
			continue;

		frame->save = 1;

		if (frame->stamp < build->chInfo.savetm)
			continue;

		if (!intra && frame->index != 0) {
			continue;
		}

		__ssf_addunit (build, frame->data_unit, frame->hdrsz + frame->rawsz, NULL, 0, DATAUNIT_STAMP (frame->data_unit), & pos, intra);
		i_pos = pos;

		if (frame->index == 0)
		{
			__idx_addindx (build, i_pos, DATAUNIT_STAMP (frame->data_unit), frame->scale, frame->interval);

			intra = TRUE;
		}
		n ++;
	}


	return n;
}
