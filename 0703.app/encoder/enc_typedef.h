
#ifndef ENC_TYPEDEF_H_
#define ENC_TYPEDEF_H_

typedef enum {
    VideoStd_AUTO = 0,    
    VideoStd_D1_NTSC,     
    VideoStd_D1_PAL,      
    VideoStd_720P_30,     
    VideoStd_1080P_30,
    VideoStd_COUNT
} VideoStd_Type;

typedef struct {
    int          		captureFd;
    int          		resizerFd;
    int          		previewerFd;
	int          		started;

    VideoStd_Type		videoStd;

    int					capWidth;
    int					capHeight;
	unsigned int		colorSpace;
} Capture_Object;

typedef struct Buffer_Object{
    char				*userPtr;
    long				physPtr;
    char				*userPtr_alloc;
    long				physPtr_alloc;
	long				numBytes;     
	int					id;
    int					width;
    int					height;
	int					lineLength;  
} Buffer_Object;

typedef struct {
	int 				ch;
	char				szFilename[256];
	FILE*				fp;

	int					SkipEncode;
	int 				capture_width, capture_height;
	long				videoBitRate, videoFrameRate;
	int 				groupOfPicture;
	int					QVal;
	int					bIsCBR;
	int					Iref_mode;
	int					codec;
} VIDEO_ENCODER_INFO;


typedef struct {
	char				szJpegFilename[256];
	FILE*				fp;

	int					videoCh;
	int 				capture_width, capture_height;
	int					jpegFrameRate;
	int					jpegQvalue;
} JPEG_ENCODER_INFO;

typedef struct {
	int					display_width, display_height;
} DISPLAY_INFO;

#endif /* ENC_TYPEDEF_H_ */
