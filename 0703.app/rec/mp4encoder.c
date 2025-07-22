#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mp4encoder.h"


typedef struct _MP4ENC_NaluUnit
{
    int type;
    int size;
    unsigned char *addr;
} MP4ENC_NaluUnit;

static int _Find(void *pNAL, unsigned int *count, unsigned char *bitstream, unsigned int streamSize);

MP4ENC mp4file_create(const char *filename, int width, int height, int fps, int timescale)
{
	MP4ENC_S *mp4enc;
	mp4enc = (MP4ENC_S *)malloc(sizeof(MP4ENC_S));
	if(mp4enc == NULL) {
		printf("%s: Failed to memory allocation!!!\n", __FUNCTION__);
		return NULL;
	}

	if(filename == NULL || width < 0 || height < 0 || fps < 0 || timescale < 0) {
		printf("%s: Invalied param!!!\n", __FUNCTION__);
		goto Fail;
	}

	//MP4LogSetLevel(MP4_LOG_VERBOSE4);

	mp4enc->file = MP4Create(filename, 0);
	if(mp4enc->file == MP4_INVALID_FILE_HANDLE) {
		printf("%s: Failed to create mp4 file!!!\n", __FUNCTION__);
		goto Fail;
	}
	mp4enc->video_id = MP4_INVALID_TRACK_ID;
	mp4enc->audio_id = MP4_INVALID_TRACK_ID;

	mp4enc->width = width;
	mp4enc->height = height;
	mp4enc->fps = fps;
	mp4enc->timescale = timescale;
	MP4SetTimeScale(mp4enc->file, timescale);

#if 0
    mp4enc->audio_id = MP4AddALawAudioTrack(mp4enc->file, 8000);
	if(mp4enc->audio_id == MP4_INVALID_TRACK_ID) {
		MP4SetTrackTimeScale(mp4enc->file, mp4enc->audio_id, 8000);
	}

	MP4SetAudioProfileLevel(mp4enc->file, 0x2);
#endif

	const MP4Tags *tagMP4 = MP4TagsAlloc();
    MP4TagsFetch(tagMP4, mp4enc->file);
    MP4TagsSetName(tagMP4, "IPCamera");
    MP4TagsSetDescription(tagMP4, "IPCamera EVT");
    MP4TagsSetCopyright(tagMP4, " ");
    uint8_t n8 = 9; /* movie */
    MP4TagsSetMediaType(tagMP4, &n8);
    MP4TagsStore(tagMP4, mp4enc->file);
    MP4TagsFree(tagMP4);

	return (MP4ENC)mp4enc;

Fail:
	free((void *)mp4enc);
	return NULL;
}

void mp4file_close(MP4ENC h)
{
	if(h == NULL) {
		return;
	}

	MP4ENC_S *mp4enc = (MP4ENC_S *)h;

	if(mp4enc->file != MP4_INVALID_FILE_HANDLE)	{
		MP4Close(mp4enc->file, MP4_CLOSE_DO_NOT_COMPUTE_BITRATE);
	}

	free((void *)mp4enc);
	return;
}

int mp4file_write_video(MP4ENC h, const void *buff, const int length, const int is_iframe)
{
	if(h == NULL) {
		return -1;
	}

	// printf("[%s:%d] %x %d %d\n", __FUNCTION__, __LINE__, buff, length, is_iframe);
	MP4ENC_S *mp4enc = (MP4ENC_S *)h;

	if(mp4enc->file == MP4_INVALID_FILE_HANDLE) {
		goto Fail;
	}

	if(buff == NULL || length <= 0) {
		goto Fail;
	}

	int i;
    unsigned int count = 0;
    MP4ENC_NaluUnit n[8];
    int result = _Find(n, &count, (unsigned char *)buff, length);

    if(result == -1) {
		goto Fail;
	}

	for(i = 0; i < count; i++) {
		if(n[i].type == 7) {
			if(mp4enc->video_id != MP4_INVALID_TRACK_ID) {
				continue;
			}

			if(mp4enc->video_id == MP4_INVALID_TRACK_ID) {
				mp4enc->video_id = MP4AddH264VideoTrack(mp4enc->file,
						mp4enc->timescale,
						(MP4Duration)(mp4enc->timescale / mp4enc->fps),
						mp4enc->width, mp4enc->height,
						n[i].addr[1], // sps[1] AVCProfileIndication
						n[i].addr[2], // sps[2] profile_compat
						n[i].addr[3], // sps[3] AVCLevelIndication
						3);           // 4 bytes length before each NAL unit

				if(mp4enc->video_id == MP4_INVALID_TRACK_ID) {
					continue;
				}
			}

			MP4SetVideoProfileLevel(mp4enc->file, 0x7f);  //1); //Simple Profile @ Level 3 0x7f
			MP4AddH264SequenceParameterSet(mp4enc->file, mp4enc->video_id,
					n[i].addr, n[i].size);
			MP4SetTrackTimeScale(mp4enc->file, mp4enc->video_id, mp4enc->timescale);
		}
		else if (n[i].type == 8) {
			if(mp4enc->video_id == MP4_INVALID_TRACK_ID) {
				continue;
			}

			MP4AddH264PictureParameterSet(mp4enc->file, mp4enc->video_id,
					n[i].addr, n[i].size);
		}
		else {
			if(mp4enc->video_id == MP4_INVALID_TRACK_ID) {
				continue;
			}

			int data_len = n[i].size + 4;
			unsigned char *data = (unsigned char *)malloc(data_len);
			data[0] = n[i].size >> 24;
			data[1] = n[i].size >> 16;
			data[2] = n[i].size >> 8;
			data[3] = n[i].size & 0xff;
			memcpy((void *)(data + 4), n[i].addr, n[i].size);
			result = MP4WriteSample(mp4enc->file, mp4enc->video_id,
					(const uint8_t *)data, data_len, MP4_INVALID_DURATION, 0, is_iframe);

			free((void *)data);
			data = NULL;

			if(result < 0) {
				break;
			}
		}
	}

	return 0;
Fail:
	return -1;
}

static int _Find(void *pNAL, unsigned int *count, unsigned char *bitstream, unsigned int streamSize)
{
    MP4ENC_NaluUnit *n = (MP4ENC_NaluUnit *)pNAL;
    int index = -1;
    unsigned char *bs = bitstream;
    unsigned int head;
    unsigned char ntype;
    unsigned char *last_byte_bitstream = bitstream + streamSize - 1;

    while(bs <= last_byte_bitstream) {
        head = (bs[3] << 24) | (bs[2] << 16) | (bs[1] << 8) | bs[0];

        if(head == 0x01000000) {
            index++;
            bs += 4;
            ntype = 0x1F & bs[0];
            n[index].type = ntype;
            n[index].addr = bs;

            if(index > 0) {
                n[index - 1].size = bs - 4 - n[index - 1].addr; // cut off 4 bytes of delimiter
            }

            if(ntype == 1 || ntype == 5) {
                n[index].size = last_byte_bitstream - bs + 1;
                break;
            }
        }
        else if (bs[3] != 0) {
            bs += 4;
        }
        else if (bs[2] != 0) {
            bs += 3;
        }
        else if (bs[1] != 0) {
            bs += 2;
        }
        else {
            bs += 1;
        }
    }

	// sleep(1);
    *count = index + 1;

    if(*count == 0) {
		return -1;
	}

    return 0;
}
