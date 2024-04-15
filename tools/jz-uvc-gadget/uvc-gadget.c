/*
 * UVC gadget test application
 *
 * Copyright (C) 2010 Ideas on board SPRL <laurent.pinchart@ideasonboard.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 */

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <linux/usb/ch9.h>
#include <linux/usb/video.h>
#include <linux/videodev2.h>

#include "imp/imp_framesource.h"
#include "imp/imp_system.h"
#include "imp/imp_log.h"
#include "sample-common.h"

#include "uvc.h"
#define  TAG    "uvc"

#define  MJPEG


#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define max(a, b) (((a) > (b)) ? (a) : (b))

#define clamp(val, min, max)                                                                                           \
        ({                                                                                                                 \
         typeof(val) __val = (val);                                                                                     \
         typeof(min) __min = (min);                                                                                     \
         typeof(max) __max = (max);                                                                                     \
         (void)(&__val == &__min);                                                                                      \
         (void)(&__val == &__max);                                                                                      \
         __val = __val < __min ? __min : __val;                                                                         \
         __val > __max ? __max : __val;                                                                                 \
         })

#define ARRAY_SIZE(a) ((sizeof(a) / sizeof(a[0])))
#define pixfmtstr(x) (x) & 0xff, ((x) >> 8) & 0xff, ((x) >> 16) & 0xff, ((x) >> 24) & 0xff

/*
 * The UVC webcam gadget kernel driver (g_webcam.ko) supports changing
 * the Brightness attribute of the Processing Unit (PU). by default. If
 * the underlying video capture device supports changing the Brightness
 * attribute of the image being acquired (like the Virtual Video, VIVI
 * driver), then we should route this UVC request to the respective
 * video capture device.
 *
 * Incase, there is no actual video capture device associated with the
 * UVC gadget and we wish to use this application as the final
 * destination of the UVC specific requests then we should return
 * pre-cooked (static) responses to GET_CUR(BRIGHTNESS) and
 * SET_CUR(BRIGHTNESS) commands to keep command verifier test tools like
 * UVC class specific test suite of USBCV, happy.
 *
 * Note that the values taken below are in sync with the VIVI driver and
 * must be changed for your specific video capture device. These values
 * also work well in case there in no actual video capture device.
 */
#define PU_BRIGHTNESS_MIN_VAL 0
#define PU_BRIGHTNESS_MAX_VAL 255
#define PU_BRIGHTNESS_STEP_SIZE 1
#define PU_BRIGHTNESS_DEFAULT_VAL 127

/* ---------------------------------------------------------------------------
 * Generic stuff
 */


/* Buffer representing one video frame */
struct buffer {
        struct v4l2_buffer buf;
        void *start;
        size_t length;
};

/* ---------------------------------------------------------------------------
 * UVC specific stuff
 */

struct uvc_frame_info {
        unsigned int width;
        unsigned int height;
        unsigned int intervals[8];
};

struct uvc_format_info {
        unsigned int fcc;
        const struct uvc_frame_info *frames;
};

static const struct uvc_frame_info uvc_frames_yuyv[] = {
        {
                640, 360,{666666, 10000000, 50000000, 0},
        },
        {
                1280, 720, {666666, 0},
        },
        {
                0, 0, {0,},
        },
};

static const struct uvc_frame_info uvc_frames_mjpeg[] = {

        {
                2048, 1536, {666666, 0},
        },
        {
                0, 0, {0,},
        },
};

static const struct uvc_format_info uvc_formats[] = {

        {V4L2_PIX_FMT_MJPEG, uvc_frames_mjpeg},
};

/* ---------------------------------------------------------------------------
 * V4L2 and UVC device instances
 */

/* Represents a V4L2 based video capture device */
struct v4l2_device {
        /* v4l2 device specific */
        int v4l2_fd;
        int is_streaming;
        char *v4l2_devname;

        /* v4l2 buffer specific */
        struct buffer *mem;
        unsigned int nbufs;

        /* v4l2 buffer queue and dequeue counters */
        unsigned long long int qbuf_count;
        unsigned long long int dqbuf_count;

        /* uvc device hook */
        struct uvc_device *udev;
};

/* Represents a UVC based video output device */
struct uvc_device {
        /* uvc device specific */
        int uvc_fd;
        int is_streaming;
        char *uvc_devname;

        /* uvc control request specific */

        struct uvc_streaming_control probe;
        struct uvc_streaming_control commit;
        int control;
        struct uvc_request_data request_error_code;
        unsigned int brightness_val;

        /* uvc buffer specific */
        struct buffer *mem;
        struct buffer *dummy_buf;
        unsigned int nbufs;
        unsigned int fcc;
        unsigned int width;
        unsigned int height;

        unsigned int bulk;
        uint8_t color;
        unsigned int imgsize;
        void *imgdata;

        /* USB speed specific */
        int mult;
        int burst;
        int maxpkt;
        enum usb_device_speed speed;

        /* uvc specific flags */
        int first_buffer_queued;
        int uvc_shutdown_requested;

        /* uvc buffer queue and dequeue counters */
        unsigned long long int qbuf_count;
        unsigned long long int dqbuf_count;

        /* v4l2 device hook */
        struct v4l2_device *vdev;
};

/************************************************************/
/************************************************************/
/************************************************************/
/***********************SDK_IMP******************************/
/************************************************************/
/************************************************************/
#define SDK
#ifdef SDK
extern struct chn_conf chn[FS_CHN_NUM];

static int burn_in_mode = 0;

static int GetFWParaConfig(const char *key, char *value)
{
	int ret = 0;
	char cmd[128] = {0};
	snprintf(cmd, sizeof(cmd), "fw_printenv factory %s", key);
	FILE *pf = popen(cmd, "r");
	char temp[512] = {0};
	if(fgets(temp, sizeof(temp), pf) != NULL)
	{
		strcpy(value,temp);
	}
	else
	{
		ret = -1;
	}
	pclose(pf);
	return ret;
}

static int SetFWParaConfigInt(const char *key, int value)
{
	char cmd[128] = {0};
	snprintf(cmd, sizeof(cmd), "fw_setenv factory %s %d", key,value);

    system(cmd);

	return 0;
}

static int FWParaConfigIncrease(const char *key)
{
    int times = 0;
    char buf[128] = {0};
    if(GetFWParaConfig(key,buf) == 0)
    {
        times = atoi(buf);
    }
    times++;
    return SetFWParaConfigInt(key,times); 
}

static int IMP_SDK_Init()
{
        int ret;

        /* Step.1 System init */
        ret = sample_system_init();
        if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_System_Init() failed\n");
                return -1;
        }

        /* Step.2 FrameSource init */
        ret = sample_framesource_init();
        if (ret < 0) {
                IMP_LOG_ERR(TAG, "FrameSource init failed\n");
                return -1;
        }

#ifdef MJPEG
        int i;
        /* Step.3 Encoder init */
        for (i = 0; i < FS_CHN_NUM; i++) {
                if (chn[i].enable) {
                        ret = IMP_Encoder_CreateGroup(chn[i].index);
                        if (ret < 0) {
                                IMP_LOG_ERR(TAG, "IMP_Encoder_CreateGroup(%d) error !\n", i);
                                return -1;
                        }
                }
        }

        if(burn_in_mode != 0 ) 
        {
                ret = sample_encoder_init();
                if (ret < 0) {
                        IMP_LOG_ERR(TAG, "Encoder init failed\n");
                        return -1;
                }
        }

        ret = sample_jpeg_init();
        if (ret < 0) {
                IMP_LOG_ERR(TAG, "Encoder init failed\n");
                return -1;
        }

        /* Step.4 Bind */
        for (i = 0; i < FS_CHN_NUM; i++) {
                if (chn[i].enable) {
                        ret = IMP_System_Bind(&chn[i].framesource_chn, &chn[i].imp_encoder);
                        if (ret < 0) {
                                IMP_LOG_ERR(TAG, "Bind FrameSource channel%d and Encoder failed\n",i);
                                return -1;
                        }
                }
        }
#endif

        ret = sample_framesource_streamon();
        if (ret < 0) {
                IMP_LOG_ERR(TAG, "ImpStreamOn failed\n");
                return -1;
        }

        return 0;
}

static int IMP_SDK_Deinit()
{
        int ret, i;

    ret = sample_framesource_streamoff();
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "FrameSource StreamOff failed\n");
        return -1;
    }

#ifdef MJPEG
    for (i = 0; i < FS_CHN_NUM; i++) {
        if (chn[i].enable) {
            ret = IMP_System_UnBind(&chn[i].framesource_chn, &chn[i].imp_encoder);
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "UnBind FrameSource channel%d and Encoder failed\n",i);
                return -1;
            }
        }
    }

    if(burn_in_mode != 0 ) 
    {
        ret = sample_encoder_exit();
        if (ret < 0) {
            IMP_LOG_ERR(TAG, "Encoder exit failed\n");
            return -1;
        }
    }

    ret = sample_jpeg_exit();
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "Encoder jpeg exit failed\n");
        return -1;
    }
#endif

    ret = sample_framesource_exit();
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "FrameSource exit failed\n");
        return -1;
    }

    ret = sample_system_exit();
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "sample_system_exit() failed\n");
        return -1;
    }
        return 0;
}

static int IMP_SDK_Streamon()
{
        int ret;
        int chn = CH0_INDEX;

#ifdef MJPEG
        ret = IMP_Encoder_StartRecvPic(chn + 4);
        if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_Encoder_StartRecvPic(%d) failed\n", 4 + chn);
                return -1;
        }
#else
        IMP_FrameSource_SetFrameDepth(chn, 4);
        if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_FrameSource_SetFrameDepth(%d,%d) failed\n", chn, 4);
                return -1;
        }
#endif

        return 0;

}

static int IMP_SDK_Streamoff()
{
        int chn = CH0_INDEX;
        int ret;
#ifdef MJPEG
        ret = IMP_Encoder_StopRecvPic(4 + chn);
        if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_Encoder_StopRecvPic() failed\n");
                return -1;
        }
#else
        IMP_FrameSource_SetFrameDepth(chn, 0);
#endif
        return 0;
}

void nv12toyuy2(char * image_in, char* image_out, int width, int height)
{
        int i, j;
        char *uv = image_in + width * height;
        for (j = 0; j < height;j++)
        {
                for (i = 0; i < width / 2;i++)
                {
                        image_out[0] = image_in[2 * i];//y
                        image_out[1] = uv[2 * i];//u
                        image_out[2] = image_in[2 * i + 1];//y
                        image_out[3] = uv[2 * i + 1];//v
                        image_out += 4;
                }
                image_in += width;
                if (j & 1)
                {
                        uv += width;
                }
        }
        return;
}

static int get_stream(char *buf, IMPEncoderStream *stream)
{
        int i, len = 0;
        int nr_pack = stream->packCount;

        //IMP_LOG_INFO(TAG, "pack count:%d\n", nr_pack);
        for (i = 0; i < nr_pack; i++) {

                IMPEncoderPack *pack = &stream->pack[i];
                if(pack->length){
                        uint32_t remSize = stream->streamSize - pack->offset;
                        if(remSize < pack->length){
                                memcpy(buf + len, (void *)(stream->virAddr + pack->offset), remSize);
                                memcpy(buf + remSize + len, (void *)stream->virAddr, pack->length - remSize);
                        }else {
                                memcpy((void *)(buf + len), (void *)(stream->virAddr + pack->offset), pack->length);
                        }
                        len += pack->length;
                }
        }
        //IMP_LOG_INFO(TAG, "len = %d\n",len);
        return len;
}

static int IMP_SDK_Get_Stream(struct uvc_device *dev, struct v4l2_buffer *buf)
{
        int ret,len;
        int chn = CH0_INDEX;
        IMPEncoderStream stream;
        IMPFrameInfo *frame_bak;
        static int times = 0;

        if((times++ % 30 == 0) && (!access("/tmp/dn", F_OK))) {
            FILE *fp = fopen("/tmp/dn", "r");
            char ch = fgetc(fp);
            IMPVI_NUM vinum = IMPVI_MAIN;
            IMPISPRunningMode pmode;

            if(ch == '0')
            {
                printf("### entry night mode IMPISP_RUNNING_MODE_NIGHT###\n");
                pmode = IMPISP_RUNNING_MODE_NIGHT;
            }
            else
            {
                printf("### entry night mode IMPISP_RUNNING_MODE_DAY###\n");
                pmode = IMPISP_RUNNING_MODE_DAY;
            }

            IMP_ISP_Tuning_SetISPRunningMode(vinum, &pmode);

            remove("/tmp/dn");
        }

        switch (dev->fcc) {
                case V4L2_PIX_FMT_YUYV:
                        ret = IMP_FrameSource_GetFrame(chn, &frame_bak);
                        if (ret < 0) {
                                IMP_LOG_ERR(TAG, "IMP_FrameSource_GetFrame(%d) failed\n", chn);
                                goto err_IMP_FrameSource_GetFrame;
                        }
                        nv12toyuy2((char *)frame_bak->virAddr, dev->mem[buf->index].start, dev->width , dev->height);
                        ret = IMP_FrameSource_ReleaseFrame(chn, frame_bak);
                        if (ret < 0) {
                                IMP_LOG_ERR(TAG, "IMP_FrameSource_ReleaseFrame(%d) failed\n", chn);
                        }
                        len = dev->width * dev->height *2;
                        break;

                case V4L2_PIX_FMT_MJPEG:
                        ret = IMP_Encoder_PollingStream(chn + 4, 1000);
                        if (ret < 0) {
                                IMP_LOG_ERR(TAG, "IMP_Encoder_PollingStream(%d) timeout\n", chn);
                        }
                        ret = IMP_Encoder_GetStream(chn + 4, &stream, 1);
                        if (ret < 0) {
                                IMP_LOG_ERR(TAG, "IMP_Encoder_GetStream(%d) failed\n", chn);
                                return 0;
                        }
                        len = get_stream(dev->mem[buf->index].start, &stream);
                        IMP_Encoder_ReleaseStream(chn + 4 , &stream);
                        break;
        }

        return len;

err_IMP_FrameSource_GetFrame:
        IMP_FrameSource_SetFrameDepth(chn, 0);
        return 0;
}

static volatile int error_int_num = 0;
int get_error_int_num()
{
	int fd = 0;
    char buf[512];
    char *p_int_num;
    int int_num,flag = 0;

    fd = open("/proc/jz/isp/isp-w02", O_RDONLY | O_SYNC);        
    if (fd < 0)    
    {  
        printf("open mem fd failed,%s\n", strerror(errno));      
        return -1;  
    }

    int ret = read(fd,buf,512);

    // printf("3333-%d\n%s\n",ret,buf);

    for(int i = 0; i < ret; i++)
    {
        if(buf[i] == '\n')
        {
            flag++;
            if(flag == 2)
            {
                p_int_num = buf + i + 1;
            }
        }

        if(flag == 2 && buf[i] == ',')
        {
            buf[i] = 0;
            break;
        }
    }
    
    int_num = atoi(p_int_num);
    // printf("%d\n",int_num);

	return int_num;
}

void *IMP_Get_Frame_Thread(void *args)
{
    int chn = CH0_INDEX;
    int camera_open_flag = 1;
    IMPEncoderStream stream;

    int	ret = IMP_Encoder_StartRecvPic(0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_StartRecvPic(%d) failed\n", 0);
		return ((void *)-1);
	}
    
    while(1)
    {
        static int times = 0;
        if(camera_open_flag == 0)
        {
            FWParaConfigIncrease("camera_open_times");
            if(IMP_SDK_Init() < 0)
            {
                FWParaConfigIncrease("camera_open_failure_times");
                sleep(1);
                continue;
            }

            camera_open_flag = 1;
            error_int_num = get_error_int_num();
            FWParaConfigIncrease("camera_read_failure_times");  
        }

        if((times++ % 99 == 0) && (!access("/tmp/dn", F_OK))) {
            FILE *fp = fopen("/tmp/dn", "r");
            char ch = fgetc(fp);
            IMPVI_NUM vinum = IMPVI_MAIN;
            IMPISPRunningMode pmode;

            if(ch == '0')
            {
                printf("### entry night mode IMPISP_RUNNING_MODE_NIGHT###\n");
                pmode = IMPISP_RUNNING_MODE_NIGHT;
            }
            else
            {
                printf("### entry night mode IMPISP_RUNNING_MODE_DAY###\n");
                pmode = IMPISP_RUNNING_MODE_DAY;
            }

            IMP_ISP_Tuning_SetISPRunningMode(vinum, &pmode);

            remove("/tmp/dn");
        }
        else if(times % 900 == 0) { //1 min
            printf("IMP_Get_Frame_Thread is work\n");
            int _error_int_num = get_error_int_num();
            if(error_int_num != _error_int_num)
            {
                IMP_SDK_Deinit();
                camera_open_flag = 0;
                continue;
            }
        }

        ret = IMP_Encoder_PollingStream(0, 1000);
        if (ret < 0) {
            IMP_LOG_ERR(TAG, "IMP_Encoder_PollingStream(%d) timeout\n", chn);
            continue;
        }
        ret = IMP_Encoder_GetStream(0, &stream, 1);
        if (ret < 0) {
            IMP_LOG_ERR(TAG, "IMP_Encoder_GetStream(%d) failed\n", chn);
            return 0;
        }

        IMP_Encoder_ReleaseStream(0 , &stream);
    }

    return NULL;
}

#endif

/************************************************************/
/************************************************************/
/************************************************************/
/************************************************************/
/************************************************************/
/************************************************************/
/* forward declarations */
static int
uvc_video_set_format(struct uvc_device *dev)
{
    struct v4l2_format fmt;
    int ret;

    CLEAR(fmt);

    fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    fmt.fmt.pix.width = dev->width;
    fmt.fmt.pix.height = dev->height;
    fmt.fmt.pix.pixelformat = dev->fcc;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    fmt.fmt.pix.sizeimage = dev->imgsize;
    ret = ioctl(dev->uvc_fd, VIDIOC_S_FMT, &fmt);
    if (ret < 0) {
        return ret;
    }


    return 0;
}

static int uvc_video_stream(struct uvc_device *dev, int enable)
{
        int type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        int ret;

        if (!enable) {
                ret = ioctl(dev->uvc_fd, VIDIOC_STREAMOFF, &type);
                if (ret < 0) {
                        IMP_LOG_INFO(TAG, "UVC: VIDIOC_STREAMOFF failed: %s (%d).\n", strerror(errno), errno);
                        return ret;
                }

#ifdef SDK
                ret = IMP_SDK_Streamoff();
                if (ret < 0) {
                        IMP_LOG_INFO(TAG, "IMP_SDK_Streamoff failed !!\n");
                        return ret;
                }
#endif

                IMP_LOG_INFO(TAG, "UVC: Stopping video stream.\n");

                return 0;
        }

        ret = ioctl(dev->uvc_fd, VIDIOC_STREAMON, &type);
        if (ret < 0) {
                IMP_LOG_INFO(TAG, "UVC: Unable to start streaming %s (%d).\n", strerror(errno), errno);
                return ret;
        }


        IMP_LOG_INFO(TAG, "UVC: Starting video stream.\n");

        dev->uvc_shutdown_requested = 0;

        return 0;
}

static int uvc_uninit_device(struct uvc_device *dev)
{
        unsigned int i;
        int ret;

        for (i = 0; i < dev->nbufs; ++i) {
                ret = munmap(dev->mem[i].start, dev->mem[i].length);
                if (ret < 0) {
                        IMP_LOG_INFO(TAG, "UVC: munmap failed\n");
                        return ret;
                }
        }

        free(dev->mem);

        return 0;
}
static int uvc_open(struct uvc_device **uvc, char *devname)
{
        struct uvc_device *dev;
        struct v4l2_capability cap;
        int fd;
        int ret = -EINVAL;

        fd = open(devname, O_RDWR | O_NONBLOCK);
        if (fd == -1) {
                IMP_LOG_INFO(TAG, "UVC: device open failed: %s (%d).\n", strerror(errno), errno);
                return ret;
        }

        ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
        if (ret < 0) {
                IMP_LOG_INFO(TAG, "UVC: unable to query uvc device: %s (%d)\n", strerror(errno), errno);
                goto err;
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_OUTPUT)) {
                IMP_LOG_INFO(TAG, "UVC: %s is no video output device\n", devname);
                goto err;
        }

        dev = calloc(1, sizeof *dev);
        if (dev == NULL) {
                ret = -ENOMEM;
                goto err;
        }

        IMP_LOG_INFO(TAG, "uvc device is %s on bus %s\n", cap.card, cap.bus_info);
        IMP_LOG_INFO(TAG, "uvc open succeeded, file descriptor = %d\n", fd);

        dev->uvc_fd = fd;
        *uvc = dev;

        return 0;

err:
        close(fd);
        return ret;
}

static void uvc_close(struct uvc_device *dev)
{
        close(dev->uvc_fd);
        free(dev->imgdata);
        free(dev);
}

/* ---------------------------------------------------------------------------
 * UVC streaming related
 */

static void uvc_video_fill_buffer(struct uvc_device *dev, struct v4l2_buffer *buf)
{
        int ret;
#ifdef SDK
        ret = IMP_SDK_Get_Stream(dev, buf);
        if(ret < 0){
                IMP_LOG_ERR(TAG, "IMP_SDK_Get_Stream err !!\n");
        }
#endif
        buf->bytesused = ret;
}

static int uvc_video_process(struct uvc_device *dev)
{
        struct v4l2_buffer ubuf;
        int ret;

        if (!dev->is_streaming)
                return 0;

        CLEAR(ubuf);

        ubuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        ubuf.memory = V4L2_MEMORY_MMAP;
        /* UVC stanalone setup. */
        ret = ioctl(dev->uvc_fd, VIDIOC_DQBUF, &ubuf);
        if (ret < 0)
                return ret;

        dev->dqbuf_count++;

        uvc_video_fill_buffer(dev, &ubuf);

        ret = ioctl(dev->uvc_fd, VIDIOC_QBUF, &ubuf);
        if (ret < 0)
                return ret;

        dev->qbuf_count++;

        return 0;
}

static int uvc_video_qbuf_mmap(struct uvc_device *dev)
{
        unsigned int i;
        int ret;

        for (i = 0; i < dev->nbufs; ++i) {
                memset(&dev->mem[i].buf, 0, sizeof(dev->mem[i].buf));

                dev->mem[i].buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
                dev->mem[i].buf.memory = V4L2_MEMORY_MMAP;
                dev->mem[i].buf.index = i;

                /* UVC standalone setup. */
                uvc_video_fill_buffer(dev, &(dev->mem[i].buf));

                ret = ioctl(dev->uvc_fd, VIDIOC_QBUF, &(dev->mem[i].buf));
                if (ret < 0) {
                        IMP_LOG_INFO(TAG, "UVC: VIDIOC_QBUF failed : %s (%d).[%d]\n", strerror(errno), errno,__LINE__);
                        return ret;
                }

                dev->qbuf_count++;
        }

        return 0;
}

static int uvc_video_qbuf(struct uvc_device *dev)
{
        int ret = 0;

        ret = uvc_video_qbuf_mmap(dev);

        return ret;
}

static int uvc_video_reqbufs_mmap(struct uvc_device *dev, int nbufs)
{
        struct v4l2_requestbuffers rb;
        unsigned int i;
        int ret;

        CLEAR(rb);

        rb.count = nbufs;
        rb.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        rb.memory = V4L2_MEMORY_MMAP;

        ret = ioctl(dev->uvc_fd, VIDIOC_REQBUFS, &rb);
        if (ret < 0) {
                if (ret == -EINVAL)
                        IMP_LOG_INFO(TAG, "UVC: does not support memory mapping\n");
                else
                        IMP_LOG_INFO(TAG, "UVC: Unable to allocate buffers: %s (%d).\n", strerror(errno), errno);
                goto err;
        }

        if (!rb.count)
                return 0;

        if (rb.count < 2) {
                IMP_LOG_INFO(TAG, "UVC: Insufficient buffer memory.\n");
                ret = -EINVAL;
                goto err;
        }

        /* Map the buffers. */
        dev->mem = calloc(rb.count, sizeof dev->mem[0]);
        if (!dev->mem) {
                IMP_LOG_INFO(TAG, "UVC: Out of memory\n");
                ret = -ENOMEM;
                goto err;
        }

        for (i = 0; i < rb.count; ++i) {
                memset(&dev->mem[i].buf, 0, sizeof(dev->mem[i].buf));

                dev->mem[i].buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
                dev->mem[i].buf.memory = V4L2_MEMORY_MMAP;
                dev->mem[i].buf.index = i;

                ret = ioctl(dev->uvc_fd, VIDIOC_QUERYBUF, &(dev->mem[i].buf));
                if (ret < 0) {
                        IMP_LOG_INFO(TAG,
                                        "UVC: VIDIOC_QUERYBUF failed for buf %d: "
                                        "%s (%d).\n",
                                        i, strerror(errno), errno);
                        ret = -EINVAL;
                        goto err_free;
                }
                dev->mem[i].start =
                        mmap(NULL /* start anywhere */,
                                        dev->mem[i].buf.length,
                                        PROT_READ | PROT_WRITE /* required */,
                                        MAP_SHARED /* recommended */,
                                        dev->uvc_fd, dev->mem[i].buf.m.offset);

                if (MAP_FAILED == dev->mem[i].start) {
                        IMP_LOG_INFO(TAG, "UVC: Unable to map buffer %u: %s (%d).\n", i, strerror(errno), errno);
                        dev->mem[i].length = 0;
                        ret = -EINVAL;
                        goto err_free;
                }

                dev->mem[i].length = dev->mem[i].buf.length;
                IMP_LOG_INFO(TAG, "UVC: Buffer %u mapped at address %p.\n", i, dev->mem[i].start);
        }

        dev->nbufs = rb.count;
        IMP_LOG_INFO(TAG, "UVC: %u buffers allocated.\n", rb.count);

        return 0;

err_free:
        free(dev->mem);
err:
        return ret;
}


static int uvc_video_reqbufs(struct uvc_device *dev, int nbufs)
{
        int ret = 0;

        ret = uvc_video_reqbufs_mmap(dev, nbufs);

        return ret;
}

static int uvc_handle_streamon_event(struct uvc_device *dev)
{
        int ret;

#ifdef SDK
        ret = IMP_SDK_Streamon();
        if(ret < 0){
                IMP_LOG_ERR(TAG, "IMP_SDK_Streamon err !!\n");
        }
#endif

        ret = uvc_video_reqbufs(dev, dev->nbufs);
        if (ret < 0)
                goto err;

        /* Queue buffers to UVC domain and start streaming. */
        ret = uvc_video_qbuf(dev);
        if (ret < 0)
                goto err;

        uvc_video_stream(dev, 1);
        dev->first_buffer_queued = 1;
        dev->is_streaming = 1;

        return 0;

err:
        return ret;
}

/* ---------------------------------------------------------------------------
 * UVC Request processing
 */

        static void
uvc_fill_streaming_control(struct uvc_device *dev, struct uvc_streaming_control *ctrl, int iframe, int iformat)
{
        const struct uvc_format_info *format;
        const struct uvc_frame_info *frame;
        unsigned int nframes;

        if (iformat < 0)
                iformat = ARRAY_SIZE(uvc_formats) + iformat;
        if (iformat < 0 || iformat >= (int)ARRAY_SIZE(uvc_formats))
                return;
        format = &uvc_formats[iformat];

        nframes = 0;
        while (format->frames[nframes].width != 0)
                ++nframes;

        if (iframe < 0)
                iframe = nframes + iframe;
        if (iframe < 0 || iframe >= (int)nframes)
                return;
        frame = &format->frames[iframe];

        memset(ctrl, 0, sizeof *ctrl);

        ctrl->bmHint = 1;
        ctrl->bFormatIndex = iformat + 1;
        ctrl->bFrameIndex = iframe + 1;
        ctrl->dwFrameInterval = frame->intervals[0];
        ctrl->dwMaxVideoFrameSize = dev->imgsize;

        /* TODO: the UVC maxpayload transfer size should be filled
         * by the driver.
         */
        if (!dev->bulk)
                ctrl->dwMaxPayloadTransferSize = (dev->maxpkt) * (dev->mult + 1) * (dev->burst + 1);
        else
                ctrl->dwMaxPayloadTransferSize = ctrl->dwMaxVideoFrameSize;

        ctrl->bmFramingInfo = 3;
        ctrl->bPreferedVersion = 1;
        ctrl->bMaxVersion = 1;
}

        static void
uvc_events_process_standard(struct uvc_device *dev, struct usb_ctrlrequest *ctrl, struct uvc_request_data *resp)
{
        IMP_LOG_INFO(TAG, "standard request\n");
        (void)dev;
        (void)ctrl;
        (void)resp;
}

static void uvc_events_process_control(
                struct uvc_device *dev, uint8_t req, uint8_t cs, uint8_t entity_id, uint8_t len, struct uvc_request_data *resp)
{
        switch (entity_id) {
                case 0:
                        switch (cs) {
                                case UVC_VC_REQUEST_ERROR_CODE_CONTROL:
                                        /* Send the request error code last prepared. */
                                        resp->data[0] = dev->request_error_code.data[0];
                                        resp->length = dev->request_error_code.length;
                                        break;

                                default:
                                        /*
                                         * If we were not supposed to handle this
                                         * 'cs', prepare an error code response.
                                         */
                                        dev->request_error_code.data[0] = 0x06;
                                        dev->request_error_code.length = 1;
                                        break;
                        }
                        break;

                        /* Camera terminal unit 'UVC_VC_INPUT_TERMINAL'. */
                case 1:
                        switch (cs) {
                                /*
                                 * We support only 'UVC_CT_AE_MODE_CONTROL' for CAMERA
                                 * terminal, as our bmControls[0] = 2 for CT. Also we
                                 * support only auto exposure.
                                 */
                                case UVC_CT_AE_MODE_CONTROL:
                                        switch (req) {
                                                case UVC_SET_CUR:
                                                        /* Incase of auto exposure, attempts to
                                                         * programmatically set the auto-adjusted
                                                         * controls are ignored.
                                                         */
                                                        resp->data[0] = 0x01;
                                                        resp->length = 1;
                                                        /*
                                                         * For every successfully handled control
                                                         * request set the request error code to no
                                                         * error.
                                                         */
                                                        dev->request_error_code.data[0] = 0x00;
                                                        dev->request_error_code.length = 1;
                                                        break;

                                                case UVC_GET_INFO:
                                                        /*
                                                         * TODO: We support Set and Get requests, but
                                                         * don't support async updates on an video
                                                         * status (interrupt) endpoint as of
                                                         * now.
                                                         */
                                                        resp->data[0] = 0x03;
                                                        resp->length = 1;
                                                        /*
                                                         * For every successfully handled control
                                                         * request set the request error code to no
                                                         * error.
                                                         */
                                                        dev->request_error_code.data[0] = 0x00;
                                                        dev->request_error_code.length = 1;
                                                        break;

                                                case UVC_GET_CUR:
                                                case UVC_GET_DEF:
                                                case UVC_GET_RES:
                                                        /* Auto Mode â€“ auto Exposure Time, auto Iris. */
                                                        resp->data[0] = 0x02;
                                                        resp->length = 1;
                                                        /*
                                                         * For every successfully handled control
                                                         * request set the request error code to no
                                                         * error.
                                                         */
                                                        dev->request_error_code.data[0] = 0x00;
                                                        dev->request_error_code.length = 1;
                                                        break;
                                                default:
                                                        /*
                                                         * We don't support this control, so STALL the
                                                         * control ep.
                                                         */
                                                        resp->length = -EL2HLT;
                                                        /*
                                                         * For every unsupported control request
                                                         * set the request error code to appropriate
                                                         * value.
                                                         */
                                                        dev->request_error_code.data[0] = 0x07;
                                                        dev->request_error_code.length = 1;
                                                        break;
                                        }
                                        break;

                                default:
                                        /*
                                         * We don't support this control, so STALL the control
                                         * ep.
                                         */
                                        resp->length = -EL2HLT;
                                        /*
                                         * If we were not supposed to handle this
                                         * 'cs', prepare a Request Error Code response.
                                         */
                                        dev->request_error_code.data[0] = 0x06;
                                        dev->request_error_code.length = 1;
                                        break;
                        }
                        break;

                        /* processing unit 'UVC_VC_PROCESSING_UNIT' */
                case 2:
                        switch (cs) {
                                /*
                                 * We support only 'UVC_PU_BRIGHTNESS_CONTROL' for Processing
                                 * Unit, as our bmControls[0] = 1 for PU.
                                 */
                                case UVC_PU_BRIGHTNESS_CONTROL:
                                        switch (req) {
                                                case UVC_SET_CUR:
                                                        resp->data[0] = 0x0;
                                                        resp->length = len;
                                                        /*
                                                         * For every successfully handled control
                                                         * request set the request error code to no
                                                         * error
                                                         */
                                                        dev->request_error_code.data[0] = 0x00;
                                                        dev->request_error_code.length = 1;
                                                        break;
                                                case UVC_GET_MIN:
                                                        resp->data[0] = PU_BRIGHTNESS_MIN_VAL;
                                                        resp->length = 2;
                                                        /*
                                                         * For every successfully handled control
                                                         * request set the request error code to no
                                                         * error
                                                         */
                                                        dev->request_error_code.data[0] = 0x00;
                                                        dev->request_error_code.length = 1;
                                                        break;
                                                case UVC_GET_MAX:
                                                        resp->data[0] = PU_BRIGHTNESS_MAX_VAL;
                                                        resp->length = 2;
                                                        /*
                                                         * For every successfully handled control
                                                         * request set the request error code to no
                                                         * error
                                                         */
                                                        dev->request_error_code.data[0] = 0x00;
                                                        dev->request_error_code.length = 1;
                                                        break;
                                                case UVC_GET_CUR:
                                                        resp->length = 2;
                                                        memcpy(&resp->data[0], &dev->brightness_val, resp->length);
                                                        /*
                                                         * For every successfully handled control
                                                         * request set the request error code to no
                                                         * error
                                                         */
                                                        dev->request_error_code.data[0] = 0x00;
                                                        dev->request_error_code.length = 1;
                                                        break;
                                                case UVC_GET_INFO:
                                                        /*
                                                         * We support Set and Get requests and don't
                                                         * support async updates on an interrupt endpt
                                                         */
                                                        resp->data[0] = 0x03;
                                                        resp->length = 1;
                                                        /*
                                                         * For every successfully handled control
                                                         * request, set the request error code to no
                                                         * error.
                                                         */
                                                        dev->request_error_code.data[0] = 0x00;
                                                        dev->request_error_code.length = 1;
                                                        break;
                                                case UVC_GET_DEF:
                                                        resp->data[0] = PU_BRIGHTNESS_DEFAULT_VAL;
                                                        resp->length = 2;
                                                        /*
                                                         * For every successfully handled control
                                                         * request, set the request error code to no
                                                         * error.
                                                         */
                                                        dev->request_error_code.data[0] = 0x00;
                                                        dev->request_error_code.length = 1;
                                                        break;
                                                case UVC_GET_RES:
                                                        resp->data[0] = PU_BRIGHTNESS_STEP_SIZE;
                                                        resp->length = 2;
                                                        /*
                                                         * For every successfully handled control
                                                         * request, set the request error code to no
                                                         * error.
                                                         */
                                                        dev->request_error_code.data[0] = 0x00;
                                                        dev->request_error_code.length = 1;
                                                        break;
                                                default:
                                                        /*
                                                         * We don't support this control, so STALL the
                                                         * default control ep.
                                                         */
                                                        resp->length = -EL2HLT;
                                                        /*
                                                         * For every unsupported control request
                                                         * set the request error code to appropriate
                                                         * code.
                                                         */
                                                        dev->request_error_code.data[0] = 0x07;
                                                        dev->request_error_code.length = 1;
                                                        break;
                                        }
                                        break;

                                default:
                                        /*
                                         * We don't support this control, so STALL the control
                                         * ep.
                                         */
                                        resp->length = -EL2HLT;
                                        /*
                                         * If we were not supposed to handle this
                                         * 'cs', prepare a Request Error Code response.
                                         */
                                        dev->request_error_code.data[0] = 0x06;
                                        dev->request_error_code.length = 1;
                                        break;
                        }

                        break;

                default:
                        /*
                         * If we were not supposed to handle this
                         * 'cs', prepare a Request Error Code response.
                         */
                        dev->request_error_code.data[0] = 0x06;
                        dev->request_error_code.length = 1;
                        break;
        }

        IMP_LOG_INFO(TAG, "control request (req %02x cs %02x)\n", req, cs);
}

static void uvc_events_process_streaming(struct uvc_device *dev, uint8_t req, uint8_t cs, struct uvc_request_data *resp)
{
        struct uvc_streaming_control *ctrl;

        IMP_LOG_INFO(TAG, "streaming request (req %02x cs %02x)\n", req, cs);

        if (cs != UVC_VS_PROBE_CONTROL && cs != UVC_VS_COMMIT_CONTROL)
                return;

        ctrl = (struct uvc_streaming_control *)&resp->data;
        resp->length = sizeof *ctrl;

        switch (req) {
                case UVC_SET_CUR:
                        dev->control = cs;
                        resp->length = 34;
                        break;

                case UVC_GET_CUR:
                        if (cs == UVC_VS_PROBE_CONTROL)
                                memcpy(ctrl, &dev->probe, sizeof *ctrl);
                        else
                                memcpy(ctrl, &dev->commit, sizeof *ctrl);
                        break;

                case UVC_GET_MIN:
                case UVC_GET_MAX:
                case UVC_GET_DEF:
                        uvc_fill_streaming_control(dev, ctrl, req == UVC_GET_MAX ? -1 : 0, req == UVC_GET_MAX ? -1 : 0);
                        break;

                case UVC_GET_RES:
                        CLEAR(ctrl);
                        break;

                case UVC_GET_LEN:
                        resp->data[0] = 0x00;
                        resp->data[1] = 0x22;
                        resp->length = 2;
                        break;

                case UVC_GET_INFO:
                        resp->data[0] = 0x03;
                        resp->length = 1;
                        break;
        }
}

        static void
uvc_events_process_class(struct uvc_device *dev, struct usb_ctrlrequest *ctrl, struct uvc_request_data *resp)
{
        if ((ctrl->bRequestType & USB_RECIP_MASK) != USB_RECIP_INTERFACE)
                return;

        switch (ctrl->wIndex & 0xff) {
                case UVC_INTF_CONTROL:
                        uvc_events_process_control(dev, ctrl->bRequest, ctrl->wValue >> 8, ctrl->wIndex >> 8, ctrl->wLength, resp);
                        break;

                case UVC_INTF_STREAMING:
                        uvc_events_process_streaming(dev, ctrl->bRequest, ctrl->wValue >> 8, resp);
                        break;

                default:
                        break;
        }
}
        static void
uvc_events_process_setup(struct uvc_device *dev, struct usb_ctrlrequest *ctrl, struct uvc_request_data *resp)
{
        dev->control = 0;

#ifdef ENABLE_USB_REQUEST_DEBUG
        IMP_LOG_INFO(TAG,
                        "\nbRequestType %02x bRequest %02x wValue %04x wIndex %04x "
                        "wLength %04x\n",
                        ctrl->bRequestType, ctrl->bRequest, ctrl->wValue, ctrl->wIndex, ctrl->wLength);
#endif
        switch (ctrl->bRequestType & USB_TYPE_MASK) {
                case USB_TYPE_STANDARD:
                        uvc_events_process_standard(dev, ctrl, resp);
                        break;

                case USB_TYPE_CLASS:
                        uvc_events_process_class(dev, ctrl, resp);
                        break;

                default:
                        break;
        }
}

        static int
uvc_events_process_control_data(struct uvc_device *dev, uint8_t cs, uint8_t entity_id, struct uvc_request_data *data)
{
        switch (entity_id) {
                /* Processing unit 'UVC_VC_PROCESSING_UNIT'. */
                case 2:
                        switch (cs) {
                                /*
                                 * We support only 'UVC_PU_BRIGHTNESS_CONTROL' for Processing
                                 * Unit, as our bmControls[0] = 1 for PU.
                                 */
                                case UVC_PU_BRIGHTNESS_CONTROL:
                                        memcpy(&dev->brightness_val, data->data, data->length);
                                        /* UVC - V4L2 integrated path. */
                                        break;

                                default:
                                        break;
                        }

                        break;

                default:
                        break;
        }

        IMP_LOG_INFO(TAG, "Control Request data phase (cs %02x entity %02x)\n", cs, entity_id);

        return 0;
}

static int uvc_events_process_data(struct uvc_device *dev, struct uvc_request_data *data)
{
        struct uvc_streaming_control *target;
        struct uvc_streaming_control *ctrl;
        const struct uvc_format_info *format;
        const struct uvc_frame_info *frame;
        const unsigned int *interval;
        unsigned int iformat, iframe;
        unsigned int nframes;
        unsigned int *val = (unsigned int *)data->data;
        int ret;

        switch (dev->control) {
                case UVC_VS_PROBE_CONTROL:
                        IMP_LOG_INFO(TAG, "setting probe control, length = %d\n", data->length);
                        target = &dev->probe;
                        break;

                case UVC_VS_COMMIT_CONTROL:
                        IMP_LOG_INFO(TAG, "setting commit control, length = %d\n", data->length);
                        target = &dev->commit;
                        break;

                default:
                        IMP_LOG_INFO(TAG, "setting unknown control, length = %d\n", data->length);

                        /*
                         * As we support only BRIGHTNESS control, this request is
                         * for setting BRIGHTNESS control.
                         * Check for any invalid SET_CUR(BRIGHTNESS) requests
                         * from Host. Note that we support Brightness levels
                         * from 0x0 to 0x10 in a step of 0x1. So, any request
                         * with value greater than 0x10 is invalid.
                         */
                        if (*val > PU_BRIGHTNESS_MAX_VAL) {
                                return -EINVAL;
                        } else {
                                ret = uvc_events_process_control_data(dev, UVC_PU_BRIGHTNESS_CONTROL, 2, data);
                                if (ret < 0)
                                        goto err;

                                return 0;
                        }
        }

        ctrl = (struct uvc_streaming_control *)&data->data;
        iformat = clamp((unsigned int)ctrl->bFormatIndex, 1U, (unsigned int)ARRAY_SIZE(uvc_formats));
        format = &uvc_formats[iformat - 1];

        nframes = 0;
        while (format->frames[nframes].width != 0)
                ++nframes;

        iframe = clamp((unsigned int)ctrl->bFrameIndex, 1U, nframes);
        frame = &format->frames[iframe - 1];
        interval = frame->intervals;

        while (interval[0] < ctrl->dwFrameInterval && interval[1])
                ++interval;

        target->bFormatIndex = iformat;
        target->bFrameIndex = iframe;
        if (dev->imgsize == 0)
                IMP_LOG_INFO(TAG, "WARNING: MJPEG requested and no image loaded.\n");
        target->dwMaxVideoFrameSize = dev->imgsize;
        target->dwFrameInterval = *interval;

        if (dev->control == UVC_VS_COMMIT_CONTROL) {
                dev->fcc = format->fcc;
                dev->width = frame->width;
                dev->height = frame->height;
        uvc_video_set_format(dev);
        }

        return 0;

err:
        return ret;
}

static void uvc_events_process(struct uvc_device *dev)
{
        struct v4l2_event v4l2_event;
        struct uvc_event *uvc_event = (void *)&v4l2_event.u.data;
        struct uvc_request_data resp;
        int ret;

        ret = ioctl(dev->uvc_fd, VIDIOC_DQEVENT, &v4l2_event);
        if (ret < 0) {
                IMP_LOG_INFO(TAG, "VIDIOC_DQEVENT failed: %s (%d)\n", strerror(errno), errno);
                return;
        }

        memset(&resp, 0, sizeof resp);
        resp.length = -EL2HLT;

        switch (v4l2_event.type) {
                case UVC_EVENT_CONNECT:
                        return;

                case UVC_EVENT_DISCONNECT:
                        dev->uvc_shutdown_requested = 1;
                        IMP_LOG_INFO(TAG,
                                        "UVC: Possible USB shutdown requested from "
                                        "Host, seen via UVC_EVENT_DISCONNECT\n");
                        return;

                case UVC_EVENT_SETUP:
                        uvc_events_process_setup(dev, &uvc_event->req, &resp);
                        break;

                case UVC_EVENT_DATA:
                        ret = uvc_events_process_data(dev, &uvc_event->data);
                        if (ret < 0)
                                break;
                        return;

                case UVC_EVENT_STREAMON:
                        if (!dev->bulk)
                                uvc_handle_streamon_event(dev);
                        return;

                case UVC_EVENT_STREAMOFF:
                        /* ... and now UVC streaming.. */
                        if (dev->is_streaming) {
                                uvc_video_stream(dev, 0);
                                uvc_uninit_device(dev);
                                uvc_video_reqbufs(dev, 0);
                                dev->is_streaming = 0;
                                dev->first_buffer_queued = 0;
                        }

                        return;
        }

        ret = ioctl(dev->uvc_fd, UVCIOC_SEND_RESPONSE, &resp);
        if (ret < 0) {
                IMP_LOG_INFO(TAG, "UVCIOC_S_EVENT failed: %s (%d)\n", strerror(errno), errno);
                return;
        }
}

static void uvc_events_init(struct uvc_device *dev)
{
        struct v4l2_event_subscription sub;
        unsigned int payload_size;

        payload_size = dev->imgsize;

        uvc_fill_streaming_control(dev, &dev->probe, 0, 0);
        uvc_fill_streaming_control(dev, &dev->commit, 0, 0);

        if (dev->bulk) {
                /* FIXME Crude hack, must be negotiated with the driver. */
                dev->probe.dwMaxPayloadTransferSize = dev->commit.dwMaxPayloadTransferSize = payload_size;
        }

        memset(&sub, 0, sizeof sub);
        sub.type = UVC_EVENT_SETUP;
        ioctl(dev->uvc_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
        sub.type = UVC_EVENT_DATA;
        ioctl(dev->uvc_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
        sub.type = UVC_EVENT_STREAMON;
        ioctl(dev->uvc_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
        sub.type = UVC_EVENT_STREAMOFF;
        ioctl(dev->uvc_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
}


/* ---------------------------------------------------------------------------
 * main
 */

int main()
{
        struct uvc_device *udev;
        char *uvc_devname = "/dev/video0";

        fd_set fdsu;
        int ret;
        int bulk_mode = 0;
        int nbufs = 2;
        int mult = 0;
        int burst = 0;
        enum usb_device_speed speed = USB_SPEED_SUPER; /* High-Speed */


#ifdef SDK
    char buf[512] = {0};
    if(GetFWParaConfig("burn_in_mode",buf) == 0)
    {
        burn_in_mode = atoi(buf);
    }

    ret = IMP_SDK_Init();
    printf("burn_in_mode:%d\n",burn_in_mode);
    if(burn_in_mode != 0)
    {
        FWParaConfigIncrease("camera_open_times");
        if(ret < 0){
            FWParaConfigIncrease("camera_open_failure_times");
            IMP_LOG_ERR(TAG, "IMP_SDK_Init ERR !!\n");
        }
        else
        {
            pthread_t tid;
            pthread_create(&tid, NULL, IMP_Get_Frame_Thread, NULL);
        }
    }
#endif

        /* Open the UVC device. */
        ret = uvc_open(&udev, uvc_devname);
        if (udev == NULL || ret < 0)
                return 1;

        udev->uvc_devname = uvc_devname;

        /* Set parameters as passed by user. */
#if defined SENSOR_WIDTH
        udev->width = SENSOR_WIDTH;
        udev->height = SENSOR_HEIGHT;
#elif defined FIRST_SENSOR_WIDTH
        udev->width = FIRST_SENSOR_WIDTH;
        udev->height = FIRST_SENSOR_HEIGHT;
#endif
#ifdef MJPEG
        udev->imgsize = udev->width * udev->height * 1.5;
        udev->fcc =  V4L2_PIX_FMT_MJPEG;
#else
        udev->imgsize = udev->width * udev->height * 2;
        udev->fcc =  V4L2_PIX_FMT_YUYV;
#endif
        udev->bulk = bulk_mode;
        udev->nbufs = nbufs;
        udev->mult = mult;
        udev->burst = burst;
        udev->speed = speed;
        udev->maxpkt = 1024;


        /* Init UVC events. */
        uvc_events_init(udev);


        while (1) {

                FD_ZERO(&fdsu);

                /* We want both setup and data events on UVC interface.. */
                FD_SET(udev->uvc_fd, &fdsu);

                fd_set efds = fdsu;
                fd_set dfds = fdsu;

                ret = select(udev->uvc_fd + 1, NULL, &dfds, &efds, NULL);

                if (-1 == ret) {
                        IMP_LOG_INFO(TAG, "select error %d, %s\n", errno, strerror(errno));
                        if (EINTR == errno)
                                continue;
                        break;
                }
                if (0 == ret) {
                        IMP_LOG_INFO(TAG, "select timeout\n");
                        break;
                }

                if (FD_ISSET(udev->uvc_fd, &efds))
                        uvc_events_process(udev);
                if (FD_ISSET(udev->uvc_fd, &dfds))
                        uvc_video_process(udev);

        }

        if (udev->is_streaming) {
                /* ... and now UVC streaming.. */
                uvc_video_stream(udev, 0);
                uvc_uninit_device(udev);
                uvc_video_reqbufs(udev, 0);
                udev->is_streaming = 0;
        }

#ifdef SDK
        ret = IMP_SDK_Deinit();
        if(ret < 0){
                IMP_LOG_ERR(TAG, "IMP_SDK_Deinit ERR !!\n");
                return -1;
        }
#endif
        uvc_close(udev);
        return 0;
}
