#ifndef __MP4_ENCODER_H__
#define __MP4_ENCODER_H__

#include "mp4v2/mp4v2.h"

typedef struct _MP4ENC_S {
	MP4FileHandle file;
	MP4TrackId video_id;
	MP4TrackId audio_id;
	int width;
	int height;
	int fps;
	int timescale;
} MP4ENC_S;

typedef MP4ENC_S* MP4ENC;

MP4ENC mp4file_create(const char *filename, int width, int height, int fps, int timescale);
void mp4file_close(MP4ENC mp4enc);

int mp4file_write_video(MP4ENC mp4enc, const void *buff, const int length, const int is_iframe);
// int mp4file_write_audio(MP4ENC mp4enc, const void *buff, const int length);

#endif /*__MP4_ENCODER_H__*/
