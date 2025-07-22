#ifndef __API_VIDEO_QUALITY_AWB__
#define __API_VIDEO_QUALITY_AWB__

int api_video_quality_awb(char *request_method, char *data, int id, char *attr, char *resp);
int api_video_quality_awb_range(char *request_method, char *data, int id, char *attr, char *resp);
int api_video_quality_awb_default(char *request_method, char *data, int id, char *attr, char *resp);
int api_video_quality_awb_all(char *request_method, char *data, int id, char *attr, char *resp);

#endif /*__API_VIDEO_QUALITY_AWB__*/

