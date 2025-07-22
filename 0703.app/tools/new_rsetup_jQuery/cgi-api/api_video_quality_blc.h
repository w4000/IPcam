#ifndef __API_VIDEO_QUALITY_BLC__
#define __API_VIDEO_QUALITY_BLC__

int api_video_quality_blc(char *request_method, char *data, int id, char *attr, char *resp);
int api_video_quality_blc_range(char *request_method, char *data, int id, char *attr, char *resp);
int api_video_quality_blc_default(char *request_method, char *data, int id, char *attr, char *resp);
int api_video_quality_blc_all(char *request_method, char *data, int id, char *attr, char *resp);

#endif /*__API_VIDEO_QUALITY_BLC__*/

