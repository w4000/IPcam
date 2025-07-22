#ifndef __API_VIDEO_QUALITY_IQ__
#define __API_VIDEO_QUALITY_IQ__

int api_video_quality_iq(char *request_method, char *data, int id, char *attr, char *resp);
int api_video_quality_iq_range(char *request_method, char *data, int id, char *attr, char *resp);
int api_video_quality_iq_default(char *request_method, char *data, int id, char *attr, char *resp);
int api_video_quality_iq_all(char *request_method, char *data, int id, char *attr, char *resp);

#endif /*__API_VIDEO_QUALITY_IQ__*/

