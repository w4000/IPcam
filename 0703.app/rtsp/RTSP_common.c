
#include "common.h"
#include "lib/net.h"
#include "RTSP_common.h"
#include "device/gpio.h"
#include "lib/pool.h"

#include "lib/md5.h"
#include "lib/base64.h"

int							rtsp_fd = -1;		
static int s_bStopSignal = 0;
static pthread_t s_hThread = 0;
struct HTTPAuthState g_auth;


struct rtp_hdr_s {
	char magic;
	char channel;
	short length;
};


struct st_payload {
	int		p_type;
	char	p_encname [6];
	int		p_media;
	int		p_codecid;
	int		p_clkrate;
	int		p_audch;
} plt [] =
{
	{26, "JPEG",	   AVMEDIA_TYPE_VIDEO,	 CODEC_ID_MJPEG, 90000, -1},
	{-1, "",		   AVMEDIA_TYPE_UNKNOWN, CODEC_ID_NONE, -1, -1}
};

SVR_CTX					r_ctx[32];
int							rtsp_nr = 0;	
int							rtp_nr = 0;		
int							g_nal_nr = 0;	
int new_socket;


void
RTSPCTX_init (SVR_CTX *ctx)
{

	ctx->c_sd = -1;
	ctx->c_state = _WAIT_REQUEST;
	ctx->c_term = 0;
	memset (& ctx->c_cliaddr, 0, sizeof (ctx->c_cliaddr));
	ctx->c_inbuf [0] = '\0'; 
	ctx->c_datsz = 0;
	ctx->c_outbuf [0] = '\0';

	ctx->c_protocol [0] = '\0';
	ctx->c_sid [0] = '\0';
	ctx->c_cmd [0] = '\0';
	ctx->c_url [0] = '\0';
	ctx->c_prot [0] = '\0';
	ctx->c_auth [0] = '\0';
	ctx->c_retcode = 0;
	ctx->c_seq = 0;
	ctx->c_timeout = 0;
	ctx->c_lastcmdtime = 0;
	ctx->c_body = NULL;
	ctx->c_bodylen = 0;
	memset (ctx->c_trans, 0, sizeof (ctx->c_trans));
	ctx->ctid = 0;
	ctx->c_nrtrans = 0;
	ctx->c_rstart = 0;
	ctx->c_rend = 0;
	ctx->c_realchal [0] = '\0';
	ctx->c_server [0] = '\0';
	ctx->c_notice = 0;
	ctx->c_location [0] = '\0';
	ctx->c_getparam_supported = 0;

	memset (ctx->c_c, 0, sizeof (ctx->c_c));

	ctx->c_childidx = 0;
	ctx->c_p = NULL;
	ctx->c_std = -1;
	ctx->c_rtcpsd = -1;
	ctx->c_svrsd = -1;
	ctx->c_rsvrsd = -1;

	memset (& ctx->c_svndest, 0, sizeof (ctx->c_svndest));
	memset (& ctx->c_rsvrdest, 0, sizeof (ctx->c_rsvrdest));

	ctx->c_rtpprot = -1;
	ctx->c_rtpurl [0] = '\0';
	ctx->c_maxpayload = 1460;
	ctx->c_ssrc = 0;
	ctx->c_pt = -1;
	ctx->c_rtpseq = 0;
	ctx->c_clrat = 0;
	ctx->c_wst = 0;
	ctx->c_ntpt = 0;
	ctx->c_wclk = 0;
	ctx->c_tmptm_video = 0;
	ctx->c_last_stamp = -1;
	init_clist (& ctx->c_nal_h);
	ctx->c_nalnr = 0;
	ctx->c_nrsent = 0;
	ctx->c_octet = 0;
	/*
	init_clist (& ctx->c_nalhead2);
	ctx->c_nalnr2 = 0;
	ctx->c_nrrecv = 0;
	*/
	ctx->c_tcpnal = NULL;
	ctx->c_pkthdl = NULL;

	{// Init stream struct.
		int	cam;
		memset (& ctx->c_stream, 0, sizeof (ctx->c_stream));
		foreach_cam_ch (cam)
			init_live (& ctx->c_stream.live [cam]);


		end_foreach
	}
	ctx->c_streamurl [0] = '\0';
	ctx->id = 0;	
	ctx->c_mt = -1;
	ctx->c_bt = 0;
	ctx->c_gbltm = 0;
	ctx->c_oldtm = 0;
	ctx->c_cam = -1;
	ctx->c_nalstc [0] = 0x00;	
	ctx->c_nalstc [1] = 0x00;
	ctx->c_nalstc [2] = 0x00;
	ctx->c_nalstc [3] = 0x01;
	ctx->c_stcsz = 4;

	ctx->c_next_I = 0;
	ctx->n_astate = NULL;
#ifdef SUPPORT_H265
    ctx->c_is_ua_vlc = 0;
#endif
#ifdef USE_DF2_SUPPORT
	ctx->c_tile_mod = 1;
#else
	ctx->c_tile_mod = 0;
#endif
	ctx->c_tile_cnt = 0;
}




void
RTSPCTX_release (SVR_CTX *ctx)
{
	int 	nr = 0;

	{
		int	cam;
		struct st_stream	*svr = & ctx->c_stream;
		
		foreach_cam_ch (cam)
			if (ISLIVE (svr, cam))
				release_live (& svr->live [cam]);

		end_foreach
	}

	{
		ST_NALU	*buf = NULL;
		while (! clist_empty (& ctx->c_nal_h)) {
			buf = clist_entry (ctx->c_nal_h.next, ST_NALU, u_list);
			del_clist (& buf->u_list);
			___mem_free (buf);
			ctx->c_nalnr --;
			nr ++;

			g_nal_nr--;
		}
		
	}

	if (ctx->c_tcpnal) {
		___mem_free (ctx->c_tcpnal);
		ctx->c_tcpnal = NULL;
	}

	if (ctx->n_astate) {
		___mem_free (ctx->n_astate);
		ctx->n_astate = NULL;
	}

	shutdown (ctx->c_sd, SHUT_RDWR);
	CLOSE_SOCKET (ctx->c_sd);
	
	{
		ctx->c_std = -1;
		ctx->c_rtcpsd = -1;
	}
	shutdown (ctx->c_svrsd, SHUT_RDWR);
	CLOSE_SOCKET (ctx->c_svrsd);
	shutdown (ctx->c_rsvrsd, SHUT_RDWR);
	CLOSE_SOCKET (ctx->c_rsvrsd);
}





SVR_CTX * 
find_rtp_session (char * sesid)
{
	SVR_CTX *tmp = NULL;
	int			i = 0;
	
	_foreach_context	(tmp, i)
		if (! strncmp (tmp->c_sid, sesid, 32))
			return tmp;
	end_foreach

	return NULL;
}



int
SDP_get_playload (int codec)
{
	int	pt = -1;
	int	i = 0;

	for (i = 0; plt [i].p_type >= 0; ++ i) {
		if (plt[i].p_codecid == codec) {
			pt = plt [i].p_type;
		}
	}

	if (pt < 0) {
		pt = 96;
	}

	// return pt;
	return 26;
}


static void
SDP_init (ST_SDPSSN *s)
{
    memset (s, 0, sizeof (ST_SDPSSN));

	s->sdp_version = 0;
	s->version = 1;
	s->user = "-";
	s->src_addr = "127.0.0.1";	  
	s->src_type = "IP4";
	s->name = "IPcam";
}




static void
SDP_write_address(char *buff, int size, const char *dar,
							  const char *dt, int ttl)
{
	if (dar) {
		if (!dt)
			dt = "IP4";
			STRCAT_BUF (buff, size, "c=IN %s %s\r\n", dt, dar);
	}
}




static void
SDP_write_header (char *buff, int size, struct sdp_session_level *s)
{
    STRCAT_BUF (buff, size, "v=%d\r\n"
                            "o=- %d %d IN %s %s\r\n"
                            "s=%s\r\n",
                            s->sdp_version,
                            s->id, s->version, s->src_type, s->src_addr,
                            s->name);

    SDP_write_address (buff, size, s->dst_addr, s->dst_type, s->ttl);

    STRCAT_BUF (buff, size, "t=%d %d\r\n",
                            s->start_time, s->end_time);
}




static int
__SDP_get_address(char *dar, int size, int *ttl, const char *u)
{
    int p;
    const char *pp;
    char pt[32];

    get_split(pt, sizeof(pt), NULL, 0, dar, size, &p, NULL, 0, u);

    *ttl = 0;

    if (strcmp(pt, "rtp")) {
		return 0;
    }


    return p;
}


static char *
SDP_write_media_attributes (SVR_CTX *ctx, char *buff, int size, int codec_id, int payload_type)
{
	// char	*config = NULL;
	int		xres = 0;
	int		yres = 0;
	int		samprate = 0;
	VIDEOPROP	*v = NULL;

	assert (VALIDCH (ctx->c_cam));

	if (ISLIVE (& ctx->c_stream, ctx->c_cam)) {
		ST_SSFBUILD* b = & g_file.ssf_build [g_file.ssf_index][ctx->c_cam];
		v = b->videop;
	}
	else {
		assert (VOD (& ctx->c_stream, ctx->c_cam).playobj);
		v = VOD (& ctx->c_stream, ctx->c_cam).playobj->videop;
	}

	xres = v->xres;
	yres = v->yres;
	switch (codec_id) {
	case CODEC_ID_H265:
		STRCAT_BUF (buff, size, "a=rtpmap:%d H265/90000\r\n"
		                        "a=fmtp:%d\r\n",
		                         payload_type,
		                         payload_type);
		break;
	case CODEC_ID_H264:
		STRCAT_BUF (buff, size, "a=rtpmap:%d H264/90000\r\n"
		                        "a=fmtp:%d packetization-mode=1\r\n",
		                         payload_type,
		                         payload_type);
		break;
	case 26:
		// __SDP_write_property (ctx);
		STRCAT_BUF (buff, size, "a=rtpmap:26 JPEG/90000\r\n");
		break;


	default:
		/* Nothing special to do here... */
		break;
	}


	return buff;
}

static int GET_CODECID (GUID	clsid)
{
	return
	(clsid.Data1 == h265_clsid.Data1)	?	CODEC_ID_H265 :
	(clsid.Data1 == h264_clsid.Data1)	?	CODEC_ID_H264 :
	(clsid.Data1 == 0xaabbccdd)	?	26 : CODEC_ID_H264;
}



int GET_CLKRATE (int	codecid)
{
	return 90000;

}

static void SDP_write_media (SVR_CTX *ctx, char *buff, int size, int media_type, char *dar, char *dt, int p, int ttl)
{
	char	*t = NULL;
	int		pt = 0;
	//int		i;

	VIDEOPROP	*v = NULL;

	assert (VALIDCH (ctx->c_cam));

	if (ISLIVE (& ctx->c_stream, ctx->c_cam)) {
		ST_SSFBUILD* b = & g_file.ssf_build [g_file.ssf_index][ctx->c_cam];
		v = b->videop;
	}
	else {
		assert (VOD (& ctx->c_stream, ctx->c_cam).playobj);
		v = VOD (& ctx->c_stream, ctx->c_cam).playobj->videop;
	}

	if (!v)
	{
		return;
	}

	int _codec_id = 26;

	do {
		if (media_type == 0) {
			_codec_id = ctx->cvc = GET_CODECID (v->clsid);
		}
		pt = SDP_get_playload (_codec_id);

	} while (0);

	if (_codec_id == 26)
		STRCAT_BUF (buff, size, "m=video %d RTP/AVP 26\r\n", p);
	else 
		STRCAT_BUF (buff, size, "m=video %d RTP/AVP %d\r\n", p, pt);
	SDP_write_address (buff, size, dar, dt, ttl);


	STRCAT_BUF (buff, size, "b=AS:%d\r\n", 500);

	SDP_write_media_attributes (ctx, buff, size, _codec_id, pt);

}


int
SDP_create (SVR_CTX *ctx, int n_files, char *buf, int size)
{
	ST_SDPSSN	s;
	int 	i;
	int		p = 0;
	int		ttl = 0;
	char	dest[32];


    memset (buf, 0, size);

	SDP_init (& s);

    if (n_files == 1) {
        p = __SDP_get_address (dest, sizeof(dest), & ttl, "rtp://0.0.0.0");
		s.ttl = 0;

        if (dest[0]) {
            s.dst_addr = dest;
            s.dst_type = "IP4";
        }
    }

    SDP_write_header (buf, size, & s);

    dest[0] = 0;
    for (i = 0; i < n_files; i++) {

        if (n_files != 1) {
            p = __SDP_get_address(dest, sizeof (dest), & ttl,  "rtp://0.0.0.0");
			s.ttl = 0;
        }

		// Media video.
		SDP_write_media (ctx, buf, size, 0, dest[0] ? dest : NULL, s.dst_type, (p > 0) ? p + 0 : 0, ttl);
		if (p <= 0)
			STRCAT_BUF (buf, size, "a=control:streamid=%d\r\n", 0);

    }

    return 0;
}




static void
handle_basic_params(struct HTTPAuthState *state, const char *k,
                                int l, char **d, int *dl)
{
    if (!strncmp(k, "realm=", l)) {
        *d     =        state->realm;
        *dl = sizeof(state->realm);
    }
}



static void
parse_key_value(const char *s, parse_key_val_cb callback_get_buf, void *c)
{
    const char *p = s;
    /* Parse k=value pairs. */
    for (;;) {
        const char *k;
        char *d = NULL, *de;
        int l, dl = 0;

        /* Skip whitespace and potential commas. */
        while (*p && (isspace(*p) || *p == ','))
            p++;
        if (!*p)
            break;

        k = p;

        if (!(p = strchr(k, '=')))
            break;
        p++;
        l = p - k;

        callback_get_buf(c, k, l, &d, &dl);
        de = d + dl - 1;

        if (*p == '\"') {
            p++;
            while (*p && *p != '\"') {
                if (*p == '\\') {
                    if (!p[1])
                        break;
                    if (d && d < de)
                        *d++ = p[1];
                    p += 2;
                } else {
                    if (d && d < de)
                        *d++ = *p;
                    p++;
                }
            }
            if (*p == '\"')
                p++;
        } else {
            for (; *p && !(isspace(*p) || *p == ','); p++)
                if (d && d < de)
                    *d++ = *p;
        }
        if (d)
            *d = 0;
    }
}


void
http_auth_handle_header(SVR_CTX *ctx, const char *k, char *value)
{
	char d[1024];
	memset(d, 0x0, sizeof(d));

	if (!strcmp(k, "Authorization") || !strcmp(k, "WWW-Authenticate")) {
		char *p;
		struct HTTPAuthState * state = NULL;

       	assert (ctx->c_node);
		if (ctx->n_astate == NULL) {
			// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, sizeof (struct HTTPAuthState));
			ctx->n_astate = (struct HTTPAuthState *) ___mem_malloc (sizeof (struct HTTPAuthState));
		}

        state = ctx->n_astate;
        memset ((void *) state, 0, sizeof (struct HTTPAuthState));


		if (get_stristart (value, "Basic ", &p)) {
			state->auth_type = HTTP_AUTH_BASIC;
			parse_key_value(p, (parse_key_val_cb) handle_basic_params, (void *) state);
		}

	}
}

char *
http_auth_create_response (struct HTTPAuthState *state, char *d, int destsz, const char *uid, const char *passwd, const char *path, const char *method)
{


	// 

	if (!d)
		return NULL;

	if (!state)
		return NULL;
	

	// printf("! %s:%d > state->auth_type ? %d\n", __func__, __LINE__, state->auth_type );


    if (state->auth_type == HTTP_AUTH_BASIC) {
		char	*p = NULL;
		char	auth[512];	// "ID:PWD" format.

		snprintf (auth, sizeof (auth), "%s:%s", uid, passwd);
		snprintf (d, destsz, "Authorization: Basic ");
		p = d + strlen (d);
		base64_encode(p, d + destsz - p, (uint8_t*)auth, strlen(auth));
    }

    return d;
}



static SVR_CTX* __find_slot_by_sd(int sd)
{
	SVR_CTX* slot;
	int i;

	foreach_context(slot, i)
	{
		if (sd == slot->c_std)
		{
			return slot;
		}
	}
	end_foreach

	return NULL;
}

void RTSP_open_listen (void)
{
	struct	sockaddr_in		my_addr;
    int tmp = 1;


    rtsp_fd = socket(AF_INET,SOCK_STREAM,0);
    if (rtsp_fd < 0)
		goto err;

    setsockopt(rtsp_fd, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(tmp));


    if (g_setup_ext.rtsp_info.rtsp_port == 0)
		g_setup_ext.rtsp_info.rtsp_port = 554;

	memset(&my_addr, 0, sizeof(struct sockaddr_in));
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	my_addr.sin_port = htons (g_setup_ext.rtsp_info.rtsp_port);

    if (bind (rtsp_fd, (struct sockaddr *) & my_addr, sizeof (my_addr)) < 0)
		goto err;

    if (listen (rtsp_fd, 5) < 0)
		goto err;

	fcntl(rtsp_fd, F_SETFL, fcntl(rtsp_fd, F_GETFL, NULL) | O_NONBLOCK); 

	return;

err:
	if (rtsp_fd >= 0) {
		close (rtsp_fd);
		rtsp_fd = -1;
	}
}


static int RTSP_recv (SVR_CTX *ctx)
{
	int	rdsz = 0;


	if (! CONTEXT_IS_SVR (ctx))
		return -1;

	if (ctx->c_state != _WAIT_REQUEST && ctx->c_state != _SEND_DATA)
		return -1;

	if (ctx->c_state == _WAIT_REQUEST) {
		rdsz = recv (ctx->c_sd, ctx->c_inbuf + ctx->c_datsz, 1, 0);

		if (rdsz < 0) {
			if (errno != EAGAIN && errno != EINTR) {
				CONTEXT_TERM (ctx, "RTSP_recv_err");
				return -1;
			}
			else {
				return 1;
			}
		}
		else if (rdsz == 0) {  
			CONTEXT_TERM (ctx, "RTSP_recv_connection_closed");
			return -1;
		}

		ctx->c_datsz += rdsz;

		if (ctx->c_datsz >= 4096) {  
			CONTEXT_TERM (ctx, "RTSP can not be this long.");
			return -1;
		}

		if ((ctx->c_datsz >= 2 && ! memcmp (ctx->c_inbuf + ctx->c_datsz - 2, "\n\n", 2)) ||
			(ctx->c_datsz >= 4 && ! memcmp(ctx->c_inbuf + ctx->c_datsz - 4, "\r\n\r\n", 4)))
		{// Whole request is read, so parse it!!
			RTSP_parse_protocol (ctx);

			ctx->c_datsz = 0;
			ctx->c_inbuf[0] = 0;
			return 0;
		}
		else if (/*ctx->c_datsz == 1 &&*/ ctx->c_inbuf [0] == '$') { 
			ctx->c_state = _SEND_DATA;

			return 0;
		}

		return 1;
	}

	return 0;
}


void RTSP_send (SVR_CTX *ctx)
{
	int ret = 0;

	if (ctx->c_sd < 0)
	{
		return;
	}

	if (ctx->c_state != _SEND_REPLY)
	{
		return;
	}

	ret = __sendn (ctx->c_sd, ctx->c_outbuf, strlen (ctx->c_outbuf));
	if (ret < 0) {
		CONTEXT_TERM (ctx, "RTSP_send_err");
	}

	*(char *)ctx->c_outbuf = '\0';	
	ctx->c_state = _WAIT_REQUEST;
}


static void DeleteParentSlot(SVR_CTX	*parent)
{
	SVR_CTX	*child = NULL;
	int			j = 0;
	int	loging = 0;

	_foreach_context (child, j)
		if (child->c_p == parent)
		{

			if(CONTEXT_IS_SVR (parent) && loging == 0) {
				loging = 1;
			}

			if (_CONTEXT_IS_SVR (child)) {
				rtp_nr --;
			}
			RTSPCTX_release (child);
			RTSPCTX_init (child);	
		}
	end_foreach

	if (rtsp_nr > 0)
		rtsp_nr --;


	RTSPCTX_release (parent);
	RTSPCTX_init (parent);	
}



int handle_rtsp_rtp_session (struct pollfd *poll_table)
{
	SVR_CTX	*poll_ctx  [20];
	int			nr = 8;
	int			i = 0;
	int			j = 0;

	int 		check_nr = -1;

	static int accept_close[20];
	static int accept_close_cnt[20] = {0, };

	do {	
		if (check_nr != nr)
		{
			// printf("! %s:%d > nr? %d \n", __func__, __LINE__, nr);
		}		


		int				nr_ctx = 0;		
		SVR_CTX		*slot = NULL;
		int 			term = 0;

		foreach_context(slot, i)

			if (slot->c_term) {	 
				if (CONTEXT_IS_SVR(slot))
				{
					DeleteParentSlot(slot);
				}
				else if (_CONTEXT_IS_SVR (slot))
				{
					DeleteParentSlot(slot->c_p);
				}


				continue;
			}


			if (CONTEXT_IS_SVR (slot) && (slot->c_state == _WAIT_REQUEST || slot->c_state == _SEND_DATA)) {
				poll_table [nr_ctx].fd = slot->c_sd;
				poll_table [nr_ctx].events = POLLIN;
			}
			else if (CONTEXT_IS_SVR (slot) && (slot->c_state == _SEND_REPLY)) {
				poll_table [nr_ctx].fd = slot->c_sd;
				poll_table [nr_ctx].events = POLLOUT;
			}
			else if (_CONTEXT_IS_SVR (slot) && (slot->c_state == _SEND_DATA)) {
				poll_table [nr_ctx].fd = slot->c_std;
				poll_table [nr_ctx].events = POLLOUT;

				if (_IS_LIVE (slot))
				{
					if (slot->c_mt == 0) {
						_enquenal (slot);
						 
						// if (slot->c_nrsent > 0){
						// }
					}
					else if (slot->c_mt == 2) {
						_enquenal (slot);
					}
				}
			}
			else
				continue;

			poll_ctx [nr_ctx] = slot;
			nr_ctx++;
		end_foreach



		{
			static __u64 s_poll_err_tm = 0;
			if(poll (poll_table, nr_ctx, 0) <= 0) {
				if(nr_ctx > 0) {
					if(s_poll_err_tm == 0)
						s_poll_err_tm = g_timer.current2;

					__u64 poll_err_duration = g_timer.current2 - s_poll_err_tm;

					if(poll_err_duration > 30000) {
						SVR_CTX *slot1 = NULL;
						int client_cnt = 0;

						foreach_context(slot1, i)
							slot1->c_term = 1;
							client_cnt++;
						end_foreach

					}
				}
				// printf("! %s:%d > handle_rtsp_rtp_session >> break \n", __func__, __LINE__);
				break; 
			}
			else {
				s_poll_err_tm = 0;
			}
		}



		SVR_CTX	*find_slot[32] = { NULL, };
		for(i = 0; i < nr_ctx; i ++)
		{
			if(poll_ctx [i]->c_std >= 0)
				find_slot[i] = __find_slot_by_sd(poll_ctx [i]->c_std);
		}

		for (j = 0; j < 100; j ++) {
			for (i = 0; i < nr_ctx; i ++) {

				// printf("poll_table [%d].revents? %d, _CONTEXT_IS_SVR (poll_ctx [%d])? %d, poll_ctx[%d]->c_term? %d \n", i, poll_table [i].revents, i, _CONTEXT_IS_SVR (poll_ctx [i]), i, poll_ctx[i]->c_term);
				if ((poll_table [i].revents & POLLOUT) && _CONTEXT_IS_SVR (poll_ctx [i]) && poll_ctx[i]->c_term == 0) {
					if(g_nal_nr > 4) {
						if(__send_data (poll_ctx [i]) == -2) {
							term = 1;

							if (accept_close_cnt[i]++ > 100)
							{
								poll_ctx [i]->c_term = 1;
							}


							continue;
						}
						else 
						{
							accept_close_cnt[i] = 0;
						}
					}
					else 
					{
						// printf("g_nal_nr? %d \n", g_nal_nr);
					}

				}
				else {


					if(poll_ctx[i]->c_term != 0) {
						// printf("! %s:%d > poll_ctx[%d]->c_term? %d \n", __func__, __LINE__, i, poll_ctx[i]->c_term);

						if (CONTEXT_IS_SVR(slot))
						{
							DeleteParentSlot(slot);
						}
						else if (_CONTEXT_IS_SVR (slot))
						{
							DeleteParentSlot(slot->c_p);
						}
					}
				}
			}

			if (g_nal_nr < 5 || term == 1) {
				// printf("! %s:%d > handle_rtsp_rtp_session >> break \n", __func__, __LINE__);
				break;
			}
		}


#if 1
		for (i = 0; i < nr_ctx; i ++) {
			if(CONTEXT_IS_SVR(poll_ctx[i])) {
				if (poll_table [i].revents & POLLIN) {
					int	nr_send = 5;

					while(nr_send-- && !poll_ctx[i]->c_term) {
						int read_data = RTSP_recv (poll_ctx [i]);
						if(read_data != 1 || poll_ctx[i]->c_state != _WAIT_REQUEST) {
							// printf("! %s:%d > handle_rtsp_rtp_session >> break \n", __func__, __LINE__);
							break;
						}
					}
				}
				if(poll_ctx[i]->c_term)
				{
					// printf("! %s:%d > handle_rtsp_rtp_session >> break \n", __func__, __LINE__);
					break;
				}
				RTSP_send (poll_ctx [i]);
			}
		}
#endif 


	} while (nr --);

	return 0;
}

void RTSP_serv (void)
{
	struct pollfd	poll_table [32 + 4];

	do {	
		SVR_CTX	*slot = NULL;	

		if(g_main.isNetUpgrade == 1) {
			break;
		}

		if (rtsp_fd < 0) {
			break;
		}

		poll_table [0].fd = rtsp_fd;
		poll_table [0].events = POLLIN;

		if (poll(poll_table, 1, 0) <= 0) {
			break; 	
		}

		if (! (poll_table [0].revents & POLLIN)) {
			break;
		}

		slot = CONTEXT_EMPTY ();
		if (!slot) {
			break;	
		}

		slot->c_sd = accept_svrsock (rtsp_fd, & slot->c_cliaddr);

		slot->c_state = _WAIT_REQUEST;
		slot->c_term = 0;

		fcntl(slot->c_sd, F_SETFL, fcntl(slot->c_sd, F_GETFL, NULL) | O_NONBLOCK);

		rtsp_nr ++;

	} while (0);


	handle_rtsp_rtp_session (poll_table);
}



int
_session (SVR_CTX *rtp_c, _TransportField *th, SVR_CTX *rtsp_c)
{
	struct sockaddr	addr;
	socklen_t		len = 0;

    switch (rtp_c->c_rtpprot) {

    case _LOWER_TRANSPORT_TCP:
	default:
		rtp_c->c_std = rtsp_c->c_sd;
		rtp_c->c_rtcpsd = rtsp_c->c_sd;

		rtp_c->c_maxpayload = 5 * 1024 - 19;
        break;
    }
    return 0;

err:
	return -1;
}


int __send_data (SVR_CTX *rtp_c)
{
	char msg[128];
	int			ret = 0;
	ST_NALU		*nu = NULL, *nu_next;
	int 		sd = 0;


	struct clist* head = & rtp_c->c_nal_h;

	if (clist_empty (head))
		return -1;		

	nu = clist_entry (head->next, ST_NALU, u_list);

	int cnt = 0;

	while (nu != (ST_NALU*)head)
	{
		nu_next = (ST_NALU*)nu->u_list.next;

		sd = (nu->u_IsRtcp) ? rtp_c->c_rtcpsd : rtp_c->c_std;

		  {

			ret = send (sd,  nu->u_start, nu->u_left, 0);
			if (ret <= 0) {
				if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
					// printf("!err\n");
					goto err;
				} else {
					// rtp_c->c_term =1;
					break;
				}
				
			}

			if (ret > 0) {
				nu->u_start += (ret);
				nu->u_left -= (ret);
			}
		}

		if (nu->u_left <= 0) { 
		
			{
				rtp_c->c_nalnr --;
				rtp_c->c_nrsent ++;
				rtp_c->c_octet += nu->u_plsz;
			}

			g_nal_nr--;

			del_clist (& nu->u_list);
			___mem_free (nu);

			nu = nu_next;

			if (cnt++ >= 0)
			{
				break;
			}
		}
	}

	print_mem_count();
	return 0;
	
	print_mem_count();
err:
	return -2;
}

//h264
static void
NALU_init (SVR_CTX *rtp_c, ST_NALU * unit, int index, int single, __u8 FU0, __u8 FU1, char * p, int p_size, int last, ST_FRAME* st_frame_ref)
{
	
	init_clist (& unit->u_list);

	unit->u_index = index;
	unit->u_IsRtcp = 0;

	unit->st_frame_ref = st_frame_ref;
	unit->last = last;

	if (single) {
		memcpy (unit->u_fu, p, p_size);
		unit->u_plsz = p_size;
		unit->u_start = unit->u_fu;
		unit->u_left = p_size;
	}
	else {
		unit->u_fu [0] = FU0;
		unit->u_fu [1] = FU1;
		memcpy (unit->u_p, p, p_size);
		unit->u_plsz = p_size;
		unit->u_start = unit->u_fu;
		unit->u_left = 2 + p_size;
	}

	{
		__u8	*tmp = unit->u_rtp;

		setnetbuf (tmp, char, (char) (2 << 6));
		setnetbuf (tmp, char, (char)((rtp_c->c_pt & 0x7F) | ((last & 0x01) << 7)));
		setnetbuf (tmp, short, (short) rtp_c->c_rtpseq);
		setnetbuf (tmp, int, (int) rtp_c->c_wst);
		setnetbuf (tmp, int, (int) rtp_c->c_ssrc);

		unit->u_start -= 12;
		unit->u_left += 12;
	}

	
	if (rtp_c->c_rtpprot == _LOWER_TRANSPORT_TCP) {
		__u8	*tmp = unit->u_rtsp;

		setnetbuf (tmp, char, (char) '$');
		setnetbuf (tmp, char, (char) rtp_c->id);
		setnetbuf (tmp, short, (short) unit->u_left);

		unit->u_start -= 4;
		unit->u_left +=  4;
	}

}


//h265
static void
NALU_init2 (SVR_CTX *rtp_c, ST_NALU_H265 * unit, int index, int single, __u8 FU0, __u8 FU1, __u8 FU2, char * p, int p_size, int last, ST_FRAME* st_frame_ref)
{
	init_clist (& unit->u_list);

	unit->u_index = index;
	unit->u_IsRtcp = 0;

	unit->st_frame_ref = st_frame_ref;
	unit->last = last;

	if (single) {
		memcpy (unit->u_fu, p, p_size);
		unit->u_plsz = p_size;
		unit->u_start = unit->u_fu;
		unit->u_left = p_size;
	}
	else {
		unit->u_fu [0] = FU0;
		unit->u_fu [1] = FU1;
		unit->u_fu [2] = FU2;
		memcpy (unit->u_p, p, p_size);
		unit->u_plsz = p_size;
		unit->u_start = unit->u_fu;
		unit->u_left = 3 + p_size;
	}

	{
		__u8	*tmp = unit->u_rtp;
		setnetbuf (tmp, char, (char) (2 << 6));
		setnetbuf (tmp, char, (char)((rtp_c->c_pt & 0x7F) | ((last & 0x01) << 7)));
		setnetbuf (tmp, short, (short) rtp_c->c_rtpseq);
		setnetbuf (tmp, int, (int) rtp_c->c_wst);
		setnetbuf (tmp, int, (int) rtp_c->c_ssrc);

		unit->u_start -= 12;
		unit->u_left += 12;
	}

	if (rtp_c->c_rtpprot == _LOWER_TRANSPORT_TCP) {
		__u8	*tmp = unit->u_rtsp;

		setnetbuf (tmp, char, (char) '$');
		setnetbuf (tmp, char, (char) rtp_c->id);
		setnetbuf (tmp, short, (short) unit->u_left);

		unit->u_start -= 4;
		unit->u_left +=  4;
	}


}

typedef struct __JPEG {
	int type;
	int q;
	int width;
	int height;
	int luma_table;
	int chroma_table;
	unsigned char *quant[16];
	unsigned char *scan_data;
	int scan_data_len;
	int init_done;
	int ts_incr;
	unsigned int timestamp;
} _JPEG;

#define PUT_16(p,v) ((p)[0]=((v)>>8)&0xff,(p)[1]=(v)&0xff)
#define GET_16(p) (((p)[0]<<8)|(p)[1])

static int _parse_DQT( _JPEG *out, unsigned char *d, int len )
{
	int i;

	for( i = 0; i < len; i += 65 )
	{
		if( ( d[i] & 0xF0 ) != 0 )
		{
			printf( "Unsupported quant table precision 0x%X!\n",
					d[i] & 0xF0 );
			return -1;
		}
		out->quant[d[i] & 0xF] = d + i + 1;
	}
	return 0;
}


static int _parse_SOF( _JPEG *out, unsigned char *d, int len )
{
	int c;

	out->chroma_table = -1;
	if( d[0] != 8 )
	{
		return -1;
	}
	out->height = GET_16( d + 1 );
	out->width = GET_16( d + 3 );
	
	if( ( out->height & 0x7 ) || ( out->width & 0x7 ) )
	{
		return -1;
	}
	out->width >>= 3;
	out->height >>= 3;
	if( d[5] != 3 )
	{
		return -1;
	}
	/* Loop over the parameters for each component */
	for( c = 6; c < 6 + 3 * 3; c += 3 )
	{
		if( d[c + 2] >= 16 || ! out->quant[d[c + 2]] )
		{
			return -1;
		}
		switch( d[c] ) /* d[c] contains the component ID */
		{
		case 1: /* Y */
			if( d[c + 1] == 0x11 ) out->type = 0;
			else if( d[c + 1] == 0x22 ) out->type = 1;
			else
			{
				return -1;
			}
			out->luma_table = d[c + 2];
			break;
		case 2: /* Cb */
		case 3: /* Cr */
			if( d[c + 1] != 0x11 )
			{
				return -1;
			}
			if( out->chroma_table < 0 )
				out->chroma_table = d[c + 2];
			else if( out->chroma_table != d[c + 2] )
			{
				return -1;
			}
			break;
		default:
			return -1;
		}
	}
	return 0;
}

static int _parse_DHT( _JPEG *out, unsigned char *d, int len )
{
	/* We should verify that this coder uses the standard Huffman tables */
	return 0;
}

static int make_header(_JPEG *rtp_jpeg, __u8 *fu, char * p, int p_size, ST_FRAME* st_frame_ref)
{
	int i;
	int fin = 0;
	int blen = 0;
	int plen = 0;

	for(i = 0; i < 16; ++i) rtp_jpeg->quant[i] = NULL;

	for(i = 0; i < p_size; i += blen + 2) {
		if(p[i] != 0xFF) {
			rtp_jpeg->scan_data_len = 0;
			return -1;
		}

		if(p[i+1] == 0xD8) blen = 0;
		else blen = GET_16(p + i + 2);

		switch(p[i+1]) {
			case 0xDB:
				if(_parse_DQT(rtp_jpeg, p + i + 4, blen - 2) < 0) {
					rtp_jpeg->scan_data_len = 0;
					return -1;
				}
				break;
			case 0xC0:
				if(_parse_SOF(rtp_jpeg, p + i + 4, blen - 2) < 0) {
					rtp_jpeg->scan_data_len = 0;
					return -1;
				}
				break;
			case 0xC4:
				if(rtp_jpeg->init_done) break;
				if(_parse_DHT(rtp_jpeg, p + i + 4, blen - 2) < 0) {
					rtp_jpeg->scan_data_len = 0;
					return -1;
				}
				rtp_jpeg->init_done = 1;
				break;
			case 0xDA:
				rtp_jpeg->scan_data = p + i + 2 + blen;
				rtp_jpeg->scan_data_len = p_size - i - 2 - blen;
				fin = 1;
				break;
		}
		if(fin == 1) break;
	}

	if(fin == 0) {
		return -1;
	}

	char *vhdr = fu;
	char *qhdr = vhdr + 8;
	char *lhdr = qhdr + 4;
	char *chdr = lhdr + 64;

	vhdr[0] = 0;
	vhdr[4] = rtp_jpeg->type;
	vhdr[5] = 255;
	vhdr[6] = rtp_jpeg->width;
	vhdr[7] = rtp_jpeg->height;

	qhdr[0] = 0;
	qhdr[1] = 0;
	PUT_16(qhdr + 2, 2 * 64);

	unsigned char *q = rtp_jpeg->quant[rtp_jpeg->luma_table];
	memcpy(lhdr, q, 64);
	q = rtp_jpeg->quant[rtp_jpeg->chroma_table];
	memcpy(chdr, q, 64);

	for(i = 0; i < rtp_jpeg->scan_data_len; i += plen) {
		plen = rtp_jpeg->scan_data_len - i;
		vhdr[1] = i >> 16;
		vhdr[2] = (i >> 8) & 0xff;
		vhdr[3] = i & 0xff;
	}

	return 0;
}


// mjpeg
static void
NALU_init3 (SVR_CTX *rtp_c, ST_NALU_JPEG * unit, __u8 *fu, int index, int single, _JPEG *rtp_jpeg, int offset, char * p, int p_size, int last, ST_FRAME* st_frame_ref)
{
	int i;
	int hdrsz = (offset == 0)?140:140-132;	// nal jpeg hdr sz - jpeg quant sz 
	init_clist (& unit->u_list);

	unit->u_index = index;
	unit->u_IsRtcp = 0;
	unit->st_frame_ref = st_frame_ref;
	unit->last = last;
	fu[1] = offset >> 16;
	fu[2] = (offset >> 8) & 0xFF;
	fu[3] = offset & 0xFF;

	



	for(i = 0; i < 140; i++) {
		unit->u_fu[i] = fu[i];
	}


	rtp_jpeg->timestamp = rtp_c->c_wst;

	unit->u_plsz = p_size;
	unit->u_start = unit->u_fu;
	unit->u_left = hdrsz + p_size;
	memcpy (unit->u_p - ((offset > 0)?132:0), p, p_size);

	// printf("u_left = %d (%d)(p_size:%d, hdrsz:%d)\n", unit->u_left, st_frame_ref->rawsz, p_size, hdrsz);

	{// Build the RTP header.
		__u8	*tmp = unit->u_rtp;

		setnetbuf (tmp, char, (char) (2 << 6));		// version
		setnetbuf (tmp, char, (char)((26 & 0x7F) | ((last & 0x01) << 7)));
		setnetbuf (tmp, short, (short) rtp_c->c_rtpseq);
		setnetbuf (tmp, int, (int) rtp_c->c_wst);


		setnetbuf (tmp, int, (int) rtp_c->c_ssrc);


		unit->u_start -= 12;		// rtp header // 12byte
		unit->u_left += 12;
	}

	// Build the RTP/TCP header.
	{
		__u8	*tmp = unit->u_rtsp;

		setnetbuf (tmp, char, (char) '$');
		setnetbuf (tmp, char, (char) rtp_c->id);
		setnetbuf (tmp, short, (short) unit->u_left);


		unit->u_start -= 4;	// rtp header 
		unit->u_left +=  4;
		

	}

}

static int rtp_write_uint16(uint8_t *output, uint16_t nVal)
{
    output[1] = nVal & 0xff;
    output[0] = nVal >> 8;

    return 2;
}


static void
__RTP_NAL_header (SVR_CTX *rtp_c, char * p, int sz, ST_FRAME* frame)
{
	_JPEG *rtp_jpeg = NULL;
	ST_NALU_JPEG *unit = NULL;
	// nal jpeg hdr
	__u8 fu[140];
	int offset = 0;

	// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, sizeof(_JPEG));
	rtp_jpeg = (_JPEG *)___mem_malloc(sizeof(_JPEG));
	if(make_header(rtp_jpeg, fu, p, sz, frame) < 0) {
		goto End;
	}

	if (sz <= rtp_c->c_maxpayload) { 
		// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, sizeof(ST_NALU_JPEG)+sz);
		unit = (ST_NALU_JPEG *)___mem_malloc(sizeof(ST_NALU_JPEG) + sz);
		p = rtp_jpeg->scan_data;
		sz = rtp_jpeg->scan_data_len;
		NALU_init3(rtp_c, unit, fu, -1, 1, rtp_jpeg, offset, p, sz, 1, frame);
		addto_clist_tail(&rtp_c->c_nal_h, &unit->u_list);
		rtp_c->c_nalnr ++;
		rtp_c->c_rtpseq ++;

		g_nal_nr++;
	}
	else {
		int index = 0;
		p = rtp_jpeg->scan_data;
		sz = rtp_jpeg->scan_data_len;
		while ((sz + 140) > rtp_c->c_maxpayload) {
			int rawsz = rtp_c->c_maxpayload - 140;
			// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, sizeof(ST_NALU_JPEG)+rawsz);
			unit = (ST_NALU_JPEG *)___mem_malloc(sizeof(ST_NALU_JPEG) + rawsz);
			NALU_init3(rtp_c, unit, fu, index++, 0, rtp_jpeg, offset, p, rawsz, 0, frame);
			addto_clist_tail(&rtp_c->c_nal_h, &unit->u_list);
			rtp_c->c_nalnr++;
			rtp_c->c_rtpseq++;

			g_nal_nr++;

			offset += rawsz;
			p += rawsz;
			sz -= rawsz;
		}

		// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, sizeof(ST_NALU_JPEG)+sz);
		unit = (ST_NALU_JPEG *)___mem_malloc(sizeof(ST_NALU_JPEG) + sz);
		NALU_init3(rtp_c, unit, fu, index++, 0, rtp_jpeg, offset, p, sz, 1, frame);
		addto_clist_tail(&rtp_c->c_nal_h, &unit->u_list);
		rtp_c->c_nalnr++;
		rtp_c->c_rtpseq++;

		g_nal_nr++;
	}

End:
	if(rtp_jpeg != NULL)
	{
		___mem_free(rtp_jpeg);
	}
}


static void
___header (SVR_CTX *rtp_c, char * p, int sz, ST_FRAME* frame)
{
	ST_NALU		*nu = NULL;

	if (sz <= rtp_c->c_maxpayload) {
		
		// namba
		// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, sizeof(ST_NALU)+rtp_c->c_maxpayload);   //5153
		nu = (ST_NALU *) ___mem_malloc (sizeof (ST_NALU) + rtp_c->c_maxpayload);
		NALU_init (rtp_c, nu, -1, 1, 0, 0, p, sz, 1, frame);
		addto_clist_tail (& rtp_c->c_nal_h, & nu->u_list);
		rtp_c->c_nalnr ++;
		rtp_c->c_rtpseq ++;

		g_nal_nr++;

	}
	else { 

		__u8	FU[2];		
		__u8	type = p [0] & 0x1F;
		__u8	nri = p [0] & 0x60;

		FU [0] = 28;		
		FU [0] |= nri;
		FU [1] = type;
		FU [1] |= 1 << 7;

		p += 1;
		sz -= 1;

		int index = 0;
		while ((sz + 2) > rtp_c->c_maxpayload) {
			int rawsz = rtp_c->c_maxpayload - 2;


			// namba
			// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, sizeof (ST_NALU) + rawsz);
			nu = (ST_NALU *) ___mem_malloc (sizeof (ST_NALU) + rawsz);
			NALU_init (rtp_c, nu, index++, 0, FU[0], FU[1], p, rawsz, 0, frame);
			addto_clist_tail (& rtp_c->c_nal_h, & nu->u_list);
			rtp_c->c_nalnr ++;
			rtp_c->c_rtpseq ++;

			g_nal_nr++;


			p += rawsz;
			sz -= rawsz;

			bitunset (FU [1], 7);
		}

		bitset (FU [1], 6);

		// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, sizeof(ST_NALU)+sz);
		nu = (ST_NALU *) ___mem_malloc (sizeof (ST_NALU) + sz);
		NALU_init (rtp_c, nu, index++, 0, FU[0], FU[1], p, sz, 1, frame);
		addto_clist_tail (& rtp_c->c_nal_h, & nu->u_list);
		rtp_c->c_nalnr ++;
		rtp_c->c_rtpseq ++;

		g_nal_nr++;

	}
}

static void
___header2 (SVR_CTX *rtp_c, char * p, int sz, ST_FRAME* frame)
{
	ST_NALU_H265 *nu = NULL;

	__u8 type = (p [0] & 0x7E) >> 1;

	if (sz <= rtp_c->c_maxpayload) {
		// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, sizeof(ST_NALU_H265)+rtp_c->c_maxpayload);
		nu = (ST_NALU_H265 *) ___mem_malloc (sizeof (ST_NALU_H265) + rtp_c->c_maxpayload);
		NALU_init2 (rtp_c, nu, -1, 1, 0, 0, 0, p, sz, 1, frame);
		addto_clist_tail (& rtp_c->c_nal_h, & nu->u_list);
		rtp_c->c_nalnr ++;
		rtp_c->c_rtpseq ++;

		g_nal_nr++;
	}
	else { 

		__u8	FU[3];	

		FU [0] = (p[0] & 0x81) | (49 << 1);
		FU [1] = p[1];
		FU [2] = type | 0x80;

		p += 2;
		sz -= 2;

		int index = 0;
		while ((sz + 3) > rtp_c->c_maxpayload) {
			int rawsz = rtp_c->c_maxpayload - 3;

			// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, sizeof (ST_NALU_H265) + rawsz);
			nu = (ST_NALU_H265 *) ___mem_malloc (sizeof (ST_NALU_H265) + rawsz);
			NALU_init2 (rtp_c, nu, index++, 0, FU[0], FU[1], FU[2], p, rawsz, 0, frame);
			addto_clist_tail (& rtp_c->c_nal_h, & nu->u_list);
			rtp_c->c_nalnr ++;
			rtp_c->c_rtpseq ++;

			g_nal_nr++;

			p += rawsz;
			sz -= rawsz;

			bitunset (FU [2], 7);
		}

		bitset (FU [2], 6);

		// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, sizeof (ST_NALU_H265) + sz);
		nu = (ST_NALU_H265 *) ___mem_malloc (sizeof (ST_NALU_H265) + sz);
		NALU_init2 (rtp_c, nu, index++, 0, FU[0], FU[1], FU[2], p, sz, 1, frame);
		addto_clist_tail (& rtp_c->c_nal_h, & nu->u_list);
		rtp_c->c_nalnr ++;
		rtp_c->c_rtpseq ++;

		g_nal_nr++;
	}

}

void
make_nal_header (SVR_CTX *rtp_c, ST_FRAME *frame)
{
	char	* buf = frame->data_unit;
	int		size = frame->hdrsz + frame->rawsz;

	// Skip DATAUNIT header.
	buf += 11;
	size -= 11;

	if (! rtp_c->c_bt) { 
		rtp_c->c_bt = frame->stamp;
		rtp_c->c_wclk = rtp_c->c_wst = 0;
	}
	else {
		__s64 stamp = (frame->stamp - rtp_c->c_bt) * (rtp_c->c_clrat / 1000); 

		if (stamp < 0 || stamp < (__u64) rtp_c->c_wst) {
			stamp = rtp_c->c_wst + 90;
		}

		rtp_c->c_wst = (__u32) stamp;
	}

	rtp_c->c_ntpt = (get_time_msec ()* 1000) + _OFFSET_US;



	if(buf[0] == 0xFF && buf[1] == 0xD8) {
		__RTP_NAL_header (rtp_c, buf, size, frame);
	}
	else 
	{
		char *r =  find_startcode (buf, buf + size);
		if (r >= buf + size) { 
			if(rtp_c->cvc == CODEC_ID_H265) {
				___header2 (rtp_c, buf, size, frame);
			}
			else {
				___header (rtp_c, buf, size, frame);
			}
		}
		else {
			while (r < buf + size) {
				char	*r1;

				while(!*(r++));
				if(rtp_c->c_tile_mod) {
					unsigned char nal_type = (r[0] & 0x7E) >> 1;
					r1 = find_startcode_skip (r, buf + size, (nal_type < 32)?rtp_c->c_tile_cnt:0);
				}
				else {
					r1 = find_startcode (r, buf + size);
				}

				if(rtp_c->cvc == CODEC_ID_H265) {
					if(rtp_c->c_tile_mod) {
						___header2 (rtp_c, r, r1 - r, frame);
					}
					else {
						___header2 (rtp_c, r, r1 - r, frame);
					}
				}
				else {
					___header (rtp_c, r, r1 - r, frame);
				}

				r = r1;
			}
		}
	}
}


static struct st_frame		*
_liveframe (SVR_CTX *ctx)
{
	struct st_stream	*svr = NULL;
	struct st_frame		*buf = NULL;	
	int					cam = -1;

	assert (VALIDCH (ctx->c_cam));

	if (! _CONTEXT_IS_SVR (ctx))
	{
		printf("_CONTEXT_IS_SVR FAIL!\n");
		return NULL;
	}

	if (ctx->c_state != _SEND_DATA)
	{
		printf("State is not _SEND_DATA!\n");
		return NULL;
	}

	svr = & ctx->c_stream;
	cam = ctx->c_cam;


	pthread_mutex_lock (& LIVE_FRAME_BUF_CH (cam)->mutex);

	if (LIVE_WAITING_IFRAME (svr, cam)) {
		pthread_mutex_unlock(& LIVE_FRAME_BUF_CH (cam)->mutex);
		return NULL;
	}
	else if ((!LIVE (svr, cam).send) && LIVE (svr, cam).sync) {
		buf = clist_entry(LIVE (svr, cam).sync, struct st_frame, iframe);
		if(buf) buf->refcnt++;
		LIVE (svr, cam).send = LIVE (svr, cam).sync;
		LIVE (svr, cam).sync = NULL;
	}
	else if (LIVE (svr, cam).send && (!LIVE (svr, cam).sync)) {
		if (!WE_HAVE_FRAMES_IN_BUFFER(& LIVE (svr, cam), cam)) {
			pthread_mutex_unlock(& LIVE_FRAME_BUF_CH (cam)->mutex);
			return NULL;
		}
		buf = clist_entry (LIVE (svr, cam).send, struct st_frame, iframe);
		if(buf) if(buf->refcnt > 0) buf->refcnt--;
		buf = clist_entry (LIVE (svr, cam).send->next, struct st_frame, iframe);
		if(buf) buf->refcnt++;
		LIVE (svr, cam).send = LIVE (svr, cam).send->next;

	}
	else if (LIVE (svr, cam).send && LIVE (svr, cam).sync) {
		int is_ok = 0;
		struct st_frame *p1, *p2;
		p1 = clist_entry(LIVE (svr, cam).sync, struct st_frame, iframe);
		p2 = clist_entry(LIVE (svr, cam).send, struct st_frame, iframe);
		if(p1 && p2) {
			if (p1->stamp < p2->stamp + 500)
				is_ok = 1;


		}
		if (WE_HAVE_FRAMES_IN_BUFFER(& LIVE (svr, cam), cam) && is_ok)
		{
			buf = clist_entry (LIVE (svr, cam).send, struct st_frame, iframe);
			if(buf) if(buf->refcnt > 0) buf->refcnt--;
			buf = clist_entry (LIVE (svr, cam).send->next, struct st_frame, iframe);
			if(buf) buf->refcnt++;
			LIVE (svr, cam).send = LIVE (svr, cam).send->next;

			if (LIVE (svr, cam).send == LIVE (svr, cam).sync)
				LIVE (svr, cam).sync = NULL;

		}
		else
		{
			buf = clist_entry(LIVE (svr, cam).send, struct st_frame, iframe);
			if(buf) if(buf->refcnt > 0) buf->refcnt--;
			buf = clist_entry(LIVE (svr, cam).sync, struct st_frame, iframe);
			if(buf) buf->refcnt++;
			LIVE (svr, cam).send = LIVE (svr, cam).sync;
			LIVE (svr, cam).sync = NULL;

		}
	}

	buf = clist_entry(LIVE (svr, cam).send, struct st_frame, iframe);
	pthread_mutex_unlock (& LIVE_FRAME_BUF_CH (cam)->mutex);


	if (buf == NULL)
	{
		return NULL;
	}

	if (ctx->c_mt == 3)
	{
		if (DATAUNIT_TYPE ((DATAUNIT *) buf->data_unit) != 10) 
		{
			return NULL;
		}
		else {
			socklen_t sock_size = 0;
			struct sockaddr_in sock;
			sock_size = sizeof (sock);
			memset (&sock, 0, sock_size);
			getpeername (ctx->c_std, (struct sockaddr*) &sock, &sock_size);
		}
	}
	else {
		if (DATAUNIT_TYPE ((DATAUNIT *) buf->data_unit) != SSF_VIDEO) {
			return NULL;
		}
	}

	return buf;
}

int RTSP_MAX_BUF 	= (512);

void
_enquenal (SVR_CTX *ctx)
{
	struct st_frame *frame = NULL;
	struct st_stream *svr = NULL;
	int	cam = -1;

	frame = _liveframe (ctx);
	svr = & ctx->c_stream;
	cam = ctx->c_cam;

	if (!frame || !svr)
		return;


	make_nal_header (ctx, frame);

	LIVE (svr, cam).stamp = DATAUNIT_STAMP (frame->data_unit);
	LIVE (svr, cam).numsent ++;
}



static void
_reply_header (SVR_CTX *ctx, int	retcode)
{
	time_t		ti;
	struct tm	*tm;
	char		tmpstr [32];
	char		* str =
				(retcode == 200) ? 				"OK" :
				(retcode == 405) ?				"Method Error" :
				(retcode == 401) ?				"Unauthorized Error" :
				(retcode == 453) ?				"Bandwidth Error" :
				(retcode == 454) ?				"Session Error" :
				(retcode == 455) ?				"Method Error" :
				(retcode == 459) ?				"Unknown Error" :
				(retcode == 460) ?				"Unknown Error" :
				(retcode == 461) ?				"Unknown Error" :
				(retcode == 500) ?				"Unknown Error" :
				(retcode == 503) ?				"Unknown Error" :
				(retcode == 505) ?				"Unknown Error" :
												"Unknown Error";

    struct HTTPAuthState *http_authstate;
	if(ctx->n_astate == NULL) {
		// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, sizeof (struct HTTPAuthState));
		ctx->n_astate = (struct HTTPAuthState *) ___mem_malloc (sizeof (struct HTTPAuthState));
	}
	http_authstate = ctx->n_astate;
	http_authstate->auth_type = HTTP_AUTH_BASIC;

	_WR (ctx, "RTSP/1.0 %d %s\r\n", retcode, str);
	_WR (ctx, "CSeq: %d\r\n", ctx->c_seq);



	// ! retcode? 401
	// ! g_setup_ext.rtsp_info.use_rtsp_security_select_set? 0



	// if (retcode == 401 && g_setup_ext.rtsp_info.use_rtsp_security_select_set)
	if (retcode == 401 && g_setup_ext.rtsp_info.use_security)
	{
		_WR (ctx, "WWW-Authenticate: Basic realm=\"DFaHJ2C7W6FWDzNWdAq==\"\r\n");
	}

	ti = time (NULL);
	tm = gmtime (& ti);
	strftime (tmpstr, sizeof (tmpstr), "%a, %d %b %Y %H:%M:%S", tm);
	_WR (ctx, "Date: %s GMT\r\n", tmpstr);

	if (retcode != 200)
		_WR (ctx, "\r\n");
}




int date_get_num(char **pp, int n_min, int n_max, int len_max)
{
    int i, val, c;
    char *p;

    p = *pp;
    val = 0;
    for(i = 0; i < len_max; i++) {
        c = *p;
        if (!isdigit(c))
            break;
        val = (val * 10) + c - '0';
        p++;
    }
    /* no number read ? */
    if (p == *pp)
        return -1;
    if (val < n_min || val > n_max)
        return -1;
    *pp = p;
    return val;
}



static char *
small_strptime(char *p, char *fmt,
                           struct tm *dt)
{
    int c, val;

    for(;;) {
        c = *fmt++;
        if (c == '\0') {
            return p;
        } else if (c == '%') {
            c = *fmt++;
            switch(c) {
            case 'H':
                val = date_get_num(&p, 0, 23, 2);
                if (val == -1)
                    return NULL;
                dt->tm_hour = val;
                break;
            case 'M':
                val = date_get_num(&p, 0, 59, 2);
                if (val == -1)
                    return NULL;
                dt->tm_min = val;
                break;
            case 'S':
                val = date_get_num(&p, 0, 59, 2);
                if (val == -1)
                    return NULL;
                dt->tm_sec = val;
                break;
            case 'Y':
                val = date_get_num(&p, 0, 9999, 4);
                if (val == -1)
                    return NULL;
                dt->tm_year = val - 1900;
                break;
            case 'm':
                val = date_get_num(&p, 1, 12, 2);
                if (val == -1)
                    return NULL;
                dt->tm_mon = val - 1;
                break;
            case 'd':
                val = date_get_num(&p, 1, 31, 2);
                if (val == -1)
                    return NULL;
                dt->tm_mday = val;
                break;
            case '%':
                goto match;
            default:
                return NULL;
            }
        } else {
        match:
            if (c != *p)
                return NULL;
            p++;
        }
    }
    return p;
}
static time_t
mktimegm(struct tm *tm)
{
	time_t t;

	int y = tm->tm_year + 1900, m = tm->tm_mon + 1, d = tm->tm_mday;

	if (m < 3) {
		m += 12;
		y--;
	}

	t = 86400 *
		(d + (153 * m - 457) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 719469);

	t += 3600 * tm->tm_hour + 60 * tm->tm_min + tm->tm_sec;

	return t;
}

static int
__SVR_parse_time (__s64 *timeval, char *datestr, int duration)
{
    char *p;
    int64_t t;
    struct tm dt;
    int i;
    static char * date_fmt[] = {
        "%Y-%m-%d",
        "%Y%m%d",
    };
    static char * time_fmt[] = {
        "%H:%M:%S",
        "%H%M%S",
    };
    char *q;
    int is_utc, len;
    char lastch;
    int negative = 0;

#undef time
    time_t now = time(0);

    len = strlen(datestr);
    if (len > 0)
        lastch = datestr[len - 1];
    else
        lastch = '\0';
    is_utc = (lastch == 'z' || lastch == 'Z');

    memset(&dt, 0, sizeof(dt));

    p = datestr;
    q = NULL;
    if (!duration) {
        if (!strncasecmp(datestr, "now", len)) {
            *timeval = (int64_t) now * 1000000;
            return 0;
        }

        for (i = 0; i < TBLSZ(date_fmt); i++) {
            q = small_strptime(p, date_fmt[i], &dt);
            if (q) {
                break;
            }
        }

        if (!q) {
            if (is_utc) {
                dt = *gmtime(&now);
            } else {
                dt = *localtime(&now);
            }
            dt.tm_hour = dt.tm_min = dt.tm_sec = 0;
        } else {
            p = q;
        }

        if (*p == 'T' || *p == 't' || *p == ' ')
            p++;

        for (i = 0; i < TBLSZ(time_fmt); i++) {
            q = small_strptime(p, time_fmt[i], &dt);
            if (q) {
                break;
            }
        }
    } else {
        if (p[0] == '-') {
            negative = 1;
            ++p;
        }
        q = small_strptime(p, time_fmt[0], &dt);
        if (!q) {
            dt.tm_sec = strtol(p, (char **)&q, 10);
            if (q == p) {
                *timeval = INT64_MIN;
                return -EINVAL;
            }
            dt.tm_min = 0;
            dt.tm_hour = 0;
        }
    }

    if (!q) {
        *timeval = INT64_MIN;
        return -EINVAL;
    }

    if (duration) {
        t = dt.tm_hour * 3600 + dt.tm_min * 60 + dt.tm_sec;
    } else {
        dt.tm_isdst = -1;      
        if (is_utc) {
            t = mktimegm(&dt);
        } else {
            t = mktime(&dt);
        }
    }

    t *= 1000000;

    if (*q == '.') {
        int val, n;
        q++;
        for (val = 0, n = 100000; n >= 1; n /= 10, q++) {
            if (!isdigit(*q))
                break;
            val += n * (*q - '0');
        }
        t += val;
    }
    *timeval = negative ? -t : t;
    return 0;
}

void
__SVR_parser_range (SVR_CTX *ctx, char *p)
{
	char buf[256];

	p += strspn(p, SPACE_CHARS);
	if (! get_stristart((char *)p, "npt=", &p))
	    return;

	ctx->c_rstart = AV_NOPTS_VALUE;
	ctx->c_rend = AV_NOPTS_VALUE;

	get_word_sep (buf, sizeof(buf), "-", &p);
	__SVR_parse_time (& ctx->c_rstart, buf, 1);
	if (*p == '-') {
	    p++;
	    get_word_sep(buf, sizeof(buf), "-", &p);
	    __SVR_parse_time (& ctx->c_rend, buf, 1);
	}
}

static void
__SVR_transport (SVR_CTX *ctx, char *p)
{
    char	_protocol[16];
    char	profile[16];
    char	_tpt[16];
    char	parameter[16];
    _TransportField *th = NULL;
    char	buf[256];


#define	__SVR_parse_range(min_ptr, max_ptr, pp)\
	do {\
		char *__p__;\
		int __v__;\
		\
		__p__ = *(pp);\
		__p__ += strspn(__p__, SPACE_CHARS);\
		__v__ = strtol(__p__, (char **)&__p__, 10);\
		if (*__p__ == '-') {\
			__p__++;\
			*(min_ptr) = __v__;\
			__v__ = strtol(__p__, (char **)&__p__, 10);\
			*(max_ptr) = __v__;\
		} else {\
			*(min_ptr) = __v__;\
			*(max_ptr) = __v__;\
		}\
		*(pp) = __p__;\
	} while (0)


	ctx->c_nrtrans = 0;

    for (;;) {
        p += strspn(p, SPACE_CHARS);
        if (*p == '\0')
            break;

        th = & ctx->c_trans [ctx->c_nrtrans];

        get_word_sep (_protocol, sizeof(_protocol), "/", &p);

        if (!strcasecmp (_protocol, "rtp")) {
            get_word_sep(profile, sizeof(profile), "/;,", &p);
            _tpt[0] = '\0';
            if (*p == '/')
                get_word_sep(_tpt, sizeof(_tpt), ";,", &p);

            th->transport = _TRANSPORT_SVR;
        } 

        if (!strcasecmp(_tpt, "TCP"))
            th->_tpt = _LOWER_TRANSPORT_TCP;

        if (*p == ';')
            p++;

        while (*p != '\0' && *p != ',') {
            get_word_sep(parameter, sizeof(parameter), "=;,", &p);
            if (!strcmp(parameter, "port")) {
                if (*p == '=') {
                    p++;
                    __SVR_parse_range(&th->port_min, &th->port_max, &p);
                }
            } else if (!strcmp(parameter, "client_port")) {
                if (*p == '=') {
                    p++;
                    __SVR_parse_range(&th->client_port_min,
                                     &th->client_port_max, &p);
                }
            } else if (!strcmp(parameter, "server_port")) {
                if (*p == '=') {
                    p++;
                    __SVR_parse_range(&th->server_port_min,
                                     &th->server_port_max, &p);
                }
            } else if (!strcmp(parameter, "interleaved")) {
                if (*p == '=') {
                    p++;
                    __SVR_parse_range(&th->interleaved_min,
                                     &th->interleaved_max, &p);
                }
            } else if (!strcmp(parameter, "ttl")) {
                if (*p == '=') {
                    p++;
                    th->ttl = strtol(p, (char **)&p, 10);
                }
            } else if (!strcmp(parameter, "destination")) {
                if (*p == '=') {
                    p++;
                    get_word_sep(buf, sizeof(buf), ";,", &p);
					inet_aton (buf, & th->destination);
                }
            } else if (!strcmp(parameter, "source")) {
                if (*p == '=') {
                    p++;
                    get_word_sep(buf, sizeof(buf), ";,", &p);
                    strncpy (th->source, buf, sizeof(th->source));
                }
            }

            while (*p != ';' && *p != '\0' && *p != ',')
                p++;
            if (*p == ';')
                p++;

        }
        if (*p == ',')
            p++;

        ctx->c_nrtrans ++;
    }

}


void
__SVR_parse_line (SVR_CTX *ctx, char * buf)
{

	char	* p = buf;
	printf("w4000 %s, %d \r\n",__FUNCTION__,__LINE__);
	if (get_stristart(buf, "Session:", &p)) {
		int t;

		get_word_sep (ctx->c_sid, sizeof (ctx->c_sid), ";", &p);
		if (get_stristart(p, ";timeout=", &p) &&(t = strtol(p, NULL, 10)) > 0)
			ctx->c_timeout = t;
	}
	else if (get_stristart (p, "Content-Length:", &p)) {
		ctx->c_bodylen = strtol(p, NULL, 10);
	}
	else if (get_stristart(p, "Transport:", &p)) {
	    __SVR_transport (ctx, p);
	}
	else if (get_stristart(p, "CSeq:", &p)) {
		ctx->c_seq = strtol(p, NULL, 10);
	}
	else if (get_stristart(p, "Range:", &p)) {
	    __SVR_parser_range (ctx, p);
	}
	else if (get_stristart(p, "RealChallenge1:", &p)) {
		p += strspn(p, SPACE_CHARS);
		strncpy (ctx->c_realchal, p, sizeof (ctx->c_realchal));
	} else if (get_stristart(p, "Server:", &p)) {
		p += strspn(p, SPACE_CHARS);
		strncpy (ctx->c_server, p, sizeof(ctx->c_server));
	} else if (get_stristart(p, "Notice:", &p) || get_stristart (p, "X-Notice:", &p)) {
		ctx->c_notice = strtol(p, NULL, 10);
	} else if (get_stristart(p, "Location:", &p)) {
		p += strspn(p, SPACE_CHARS);
		strncpy (ctx->c_location, p, sizeof(ctx->c_location));
	}
	else if (get_stristart(p, "Authorization: Basic", &p)) {
		p += strspn(p, SPACE_CHARS);
		memset(ctx->c_auth, 0, sizeof(ctx->c_auth));
		base64_decode((uint8_t*)ctx->c_auth, p, sizeof(ctx->c_auth));
	}
	else if (get_stristart(p, "Authorization: ", &p)) {
		p += strspn(p, SPACE_CHARS);
		http_auth_handle_header(ctx, "Authorization", p);
	}
	else if (get_stristart(p, "Require: ", &p)) {
	}

}


static size_t
av_strlcpy(char *d, const char *s, size_t size)
{
    size_t len = 0;
    while (++len < size && *s)
        *d++ = *s++;
    if (len <= size)
        *d = 0;
    return len + strlen(s) - 1;
}



void
get_split(char *pt, int proto_size,
                  char *a, int a_size,
                  char *hn, int hns,
                  int *ptp,
                  char *ph2, int path_size,
                  const char *u)
{
    const char *p, *ls, *at, *col, *brk;


#define FFMIN(a,b) ((a) > (b) ? (b) : (a))


    if (ptp)              		*ptp = -1;
    if (proto_size > 0)         pt[0] = 0;
    if (a_size > 0) a[0] = 0;
    if (hns > 0)     			hn[0] = 0;
    if (path_size > 0)          ph2[0] = 0;

    if ((p = strchr(u, ':'))) {
        av_strlcpy (pt, u, FFMIN(proto_size, p + 1 - u));
        p++; 
        if (*p == '/') p++;
        if (*p == '/') p++;
    } else {
        av_strlcpy (ph2, u, path_size);
        return;
    }

    ls = strchr(p, '/');
    if(!ls)
        ls = strchr(p, '?');
    if(ls)
        av_strlcpy (ph2, ls + 1, path_size);
    else
        ls = &p[strlen(p)]; 

    if (ls != p) {
        if ((at = strchr(p, '@')) && at < ls) {
            av_strlcpy (a, p,
                       FFMIN(a_size, at + 1 - p));
            p = at + 1; 
        }

        if (*p == '[' && (brk = strchr(p, ']')) && brk < ls) {
            av_strlcpy (hn, p + 1,
                       FFMIN(hns, brk - p));
            if (brk[1] == ':' && ptp)
                *ptp = atoi(brk + 2);
        } else if ((col = strchr(p, ':')) && col < ls) {
            av_strlcpy (hn, p,
                       FFMIN(col + 1 - p, hns));
            if (ptp) *ptp = atoi(col + 1);
        } else
            av_strlcpy (hn, p,
                       FFMIN(ls + 1 - p, hns));
    }
}



static void
__SVR_cmd_describe (SVR_CTX *ctx, char *u)
{
	char		ph1[1024];
    char		*ph2 = NULL;
	char		content [2048];
	unsigned int	len = 0;
    struct sockaddr_in	my_addr;
	int			retcode = 200;

	len = sizeof (my_addr);
	getsockname (ctx->c_sd, (struct sockaddr *) & my_addr, & len);

    get_split(NULL, 0, NULL, 0, NULL, 0, NULL, ph1, sizeof(ph1), u);
    ph2 = ph1;
    if (*ph2 == '/')
        ph2++;

	{
		char	* p =NULL;
		int 	cam = -1;
		int i;

		for (i=0; i<CAM_MAX; i++)
		{
			if ((p = strcasestr (ph2, g_setup_ext.rtsp_uri[i]))) {
				
				cam = i;
				break;
			}
		}


		if ((cam >= 0)) {

			if (! VALIDCH(cam)) { 
				retcode = 503;
				goto err;
			}

			if (! CAMERA_EXIST (cam)) {
				retcode = 503;
				goto err;
			}

			ctx->c_cam = cam;
			SS_SETFLAG (& ctx->c_stream, ctx->c_cam, __SS_LIVE);
		}
		else {
			retcode = 503;
			goto err;
		}
	}

	SDP_create (ctx, 1, content, sizeof (content));

	_reply_header (ctx, 200);
	_WR (ctx, "CSeq: %d\r\n", ctx->c_seq);
	_WR (ctx, "Content-Base: %s/\r\n", u);
	_WR (ctx, "Content-Type: application/sdp\r\n");
	_WR (ctx, "Content-Length: %d\r\n", strlen (content));
	_WR (ctx, "\r\n");
	_WR (ctx, content);
	return;

err:
	_reply_header (ctx, retcode);
	return;
}


static void
__SVR_cmd_options (SVR_CTX *ctx, const char *u)
{
    _WR (ctx, "RTSP/1.0 %d %s\r\n", 200, "OK");
    _WR (ctx, "CSeq: %d\r\n", ctx->c_seq);
    _WR (ctx, "Public: %s\r\n", "OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE");
	

    _WR (ctx, "\r\n");
}

static void
__SVR_cmd_setup (SVR_CTX *ctx, const char *u)
{
	int			i = 0;
	int			retcode = 200;
	SVR_CTX	*rtp_c = NULL;			// The RTP context created.
    char		ph1[1024];
    char		*ph2;
	_TransportField *th = NULL;

    get_split(NULL, 0, NULL, 0, NULL, 0, NULL, ph1, sizeof(ph1), u);
    ph2 = ph1;
    if (*ph2 == '/')
        ph2++;

	if (strcasestr (ph2, "stream")) {
		char	* p = NULL;

		if ((p = strcasestr (ph2, "streamid="))) {
			sscanf (p, "streamid=%d", & ctx->id);
		}
	}
	else {// NO stream found.
		retcode = 503;
		goto err;
	}

	if (!th) {
		for (i = 0; i < ctx->c_nrtrans; i ++) {
			if (ctx->c_trans [i]._tpt == _LOWER_TRANSPORT_TCP) { // Try TCP.
				th = & ctx->c_trans [i];
				ctx->ctid = i;
				break;
			}
		}
	}

	if (! th) {
		retcode = 461;
		goto err;
	}


\
	{
		if (ctx->c_sid [0] == '\0')
			snprintf (ctx->c_sid, sizeof (ctx->c_sid), "%d", ctx->c_sd);

		if (ctx->c_protocol [0] == '\0')
			snprintf (ctx->c_protocol, sizeof (ctx->c_protocol), "RTSP");

		rtp_c = CONTEXT_EMPTY ();
		if (!rtp_c) {
			retcode = 453;
			goto err;
		}


		snprintf (rtp_c->c_protocol, sizeof (rtp_c->c_protocol), "RTP");
	}

	rtp_c->c_rtpprot = th->_tpt;

	{ 
		__u32	sesid = 0;

		if (_session (rtp_c, th, ctx) < 0)
			goto err;

		int bSession = 0;

		int j;
		for (j = 0; j < 4; j++)
		{
			if (ctx->c_c[j] && ctx->c_c[j] != rtp_c)
			{
				memcpy(rtp_c->c_sid, ctx->c_c[j]->c_sid, sizeof(rtp_c->c_sid));
				bSession = 1;
				break;
			}
		}

		if (!bSession)
		{
			random_seed (sizeof (sesid), & sesid);
			snprintf (rtp_c->c_sid, sizeof (ctx->c_sid), "%x", sesid);
		}

		rtp_nr ++;
	}


	rtp_c->id = ctx->id;
	rtp_c->c_p = ctx;
	assert (ctx->id < 4);
	ctx->c_c [ctx->id] = rtp_c;

	random_seed (sizeof (rtp_c->c_ssrc), & rtp_c->c_ssrc);

	if (ctx->id == 0)
	{
		rtp_c->c_mt = 0;
		rtp_c->cvc = ctx->cvc;
		rtp_c->c_pt = SDP_get_playload (ctx->cvc);
		rtp_c->c_clrat = GET_CLKRATE (ctx->cvc);
	}

	rtp_c->c_cam = ctx->c_cam;
	memcpy (rtp_c->c_stream.flag, ctx->c_stream.flag, sizeof (rtp_c->c_stream.flag));
	memcpy (rtp_c->c_trans, ctx->c_trans, sizeof (ctx->c_trans));
	rtp_c->ctid = ctx->ctid;
	rtp_c->c_nrtrans = ctx->c_nrtrans;

    _reply_header (ctx, 200);
	_WR (ctx, "Session: %s\r\n", rtp_c->c_sid);

	switch(rtp_c->c_rtpprot) {
	case _LOWER_TRANSPORT_TCP:
		_WR (ctx, "Transport: RTP/AVP/TCP;interleaved=%d-%d",
					rtp_c->id * 2, rtp_c->id * 2 + 1);
		break;


	default:
		break;
	}

	_WR (ctx, "\r\n");
	_WR (ctx, "\r\n");
	return;

err:
	_reply_header (ctx, retcode);
	return;
}


static void
__SVR_cmd_play (SVR_CTX *ctx, char *u)
{
	SVR_CTX *rtp_c = NULL;	 
	int			retcode = 200;

	rtp_c = find_rtp_session (ctx->c_sid);
	if (! rtp_c) {
		retcode = 454;
		goto err;
	}

	rtp_c->c_state = _SEND_DATA;

	struct st_CTX* parent = rtp_c->c_p;
	if (parent)
	{
		int i;
		for (i = 0; i < 4; i++)
		{
			if (parent->c_c[i])
			{
				parent->c_c[i]->c_state = _SEND_DATA;
			}
		}
	}

	_reply_header (ctx, 200);
	_WR (ctx, "Session: %s\r\n", rtp_c->c_sid);
	_WR (ctx, "RTP-Info: u=%sstreamid=0;seq=0;rtptime=0,u=%sstreamid=1;seq=0;rtptime=0\r\n", u, u);
	_WR (ctx, "\r\n");



	return;

err:
	_reply_header (ctx, retcode);
	return;
}


static void
__SVR_cmd_pause (SVR_CTX *ctx, char *u)
{
	SVR_CTX *rtp_c = NULL;	 
	int 		retcode = 200;


	rtp_c = find_rtp_session (ctx->c_sid);
	if (! rtp_c) {
		retcode = 454;
		goto err;
	}

	rtp_c->c_state = _WAIT_REQUEST;


	_reply_header (ctx, 200);
	_WR (ctx, "Session: %s\r\n", rtp_c->c_sid);
	_WR (ctx, "\r\n");
	return;

err:
	_reply_header (ctx, retcode);
	return;

}


static void
__SVR_cmd_teardown (SVR_CTX *ctx, char *u)
{
	SVR_CTX *rtp_c = NULL;	// The RTP context for command.
	int 		retcode = 200;

	rtp_c = find_rtp_session (ctx->c_sid);
	if (! rtp_c) {
		retcode = 454;
		goto err;
	}

	_reply_header (ctx, 200);
	_WR (ctx, "Session: %s\r\n", rtp_c->c_sid);
	_WR (ctx, "\r\n");



	CONTEXT_TERM (rtp_c, "RTSP_teardown");
	return;

err:
	_reply_header (ctx, retcode);
	return;
}

int
RTSP_parse_protocol (SVR_CTX *ctx)
{

	const char *str_command[7] = {
		"DESCRIBE",
		"OPTIONS",
		"SETUP",
		"SET_PARAMETER",
		"PLAY",
		"PAUSE",
		"TEARDOWN",
	};

	char c[32];
	char u[1024];
	char protocol[32];
	char	*p = ctx->c_inbuf;

	ctx->c_inbuf [ctx->c_datsz] = '\0';

#ifdef SUPPORT_H265
    if(strstr(ctx->c_inbuf,"LibVLC"))
    {
        ctx->c_is_ua_vlc = 1;
    }
#endif
	p = str_getword (c, sizeof (c), p);
	p =	str_getword (u, sizeof (u), p);
	p =	str_getword (protocol, sizeof (protocol), p);

	snprintf (ctx->c_cmd, sizeof (ctx->c_cmd), c);
	snprintf (ctx->c_url, sizeof (ctx->c_url), u);
	snprintf (ctx->c_prot, sizeof (ctx->c_prot), protocol);

	if (!g_setup_ext.rtsp_info.use_rtsp) {
		_reply_header (ctx, 503);
		goto out;
    }

	if (strcmp (ctx->c_prot, "RTSP/1.0") != 0) {
		_reply_header (ctx, 505);
		goto out;
    }

	{// Parse each line of header.
		char	*p1 = NULL;
		char	*p2 = NULL;
		//int		len = 0;
		unsigned int		len = 0;
		char	line[1024];

		while (*p != '\n' && *p != '\0')
			p++;
		if (*p == '\n')
			p++;
		while (*p != '\0') {
			p1 = memchr(p, '\n', (char *) ctx->c_inbuf + ctx->c_datsz - p);
			if (!p1)
				break;
			p2 = p1;
			if (p2 > p && p2[-1] == '\r')
				p2--;

			if (p2 == p)  
				break;
			len = p2 - p;
			if (len > sizeof (line) - 1)
				len = sizeof (line) - 1;
			memcpy (line, p, len);
			line [len] = '\0';

			__SVR_parse_line (ctx, line);

			p = p1 + 1;
		}
	}


	if (g_setup_ext.rtsp_info.use_security)
	{

		{
			if (strlen(ctx->c_auth) == 0)
				get_split(NULL, 0, ctx->c_auth, sizeof(ctx->c_auth), NULL, 0, NULL, NULL, 0, u);

			if (strlen(ctx->c_auth) == 0)
			{
				_reply_header (ctx, 401);
				goto out;
			}

			char user[256];
			int i, ok = 0;
			for (i=0; i<10; i++)   
			{
				if (SETUP_USR_AVAIL(&g_setup, i))
				{
					{
						sprintf(user, "%s:%s", SETUP_USR_NAME(&g_setup, i), SETUP_USR_PWD(&g_setup, i));

						if (strcmp(user, ctx->c_auth) == 0)
						{
							ok = 1;
							break;
						}
					}
				}
			}
			if (!ok)
			{
				_reply_header (ctx, 401);
				goto out;
			}
			// printf("ok? %d, idx? %d \n", ok, i);
		}
	}

	if (!strcmp(c, str_command[0]))
		__SVR_cmd_describe(ctx, u);
	else if (!strcmp(c, str_command[1]))
		__SVR_cmd_options(ctx, u);
	else if (!strcmp(c, str_command[2]))
		__SVR_cmd_setup(ctx, u);
	else if (!strcmp(c, str_command[4]))
		__SVR_cmd_play (ctx, u);
	else if (!strcmp(c, str_command[5]))
		__SVR_cmd_pause (ctx, u);
	else if (!strcmp(c, str_command[6]))
		__SVR_cmd_teardown(ctx, u);
	else
		_reply_header (ctx, 405);


out:
	ctx->c_state = _SEND_REPLY;
	return 0;
}




#if defined(ACCESS_RESTRICTION)
#elif defined(USE_DF2_SUPPORT)
#else
							{
								f = 1;
								break;
							}
#endif	/* ACCESS_RESTRICTION */

