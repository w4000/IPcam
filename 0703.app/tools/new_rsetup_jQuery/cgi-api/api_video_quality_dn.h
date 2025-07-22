#ifndef __API_VIDEO_QUALITY_DN__
#define __API_VIDEO_QUALITY_DN__

int api_video_quality_dn(char *request_method, char *data, int id, char *attr, char *resp);
int api_video_quality_dn_range(char *request_method, char *data, int id, char *attr, char *resp);
int api_video_quality_dn_default(char *request_method, char *data, int id, char *attr, char *resp);
int api_video_quality_dn_all(char *request_method, char *data, int id, char *attr, char *resp);

#endif /*__API_VIDEO_QUALITY_DN__*/

