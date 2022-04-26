#include "h264_stream.h"
#include <mp4v2/mp4v2.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

MP4TrackId video;  
MP4TrackId audio;  
MP4FileHandle fileHandle;
pthread_mutex_t mp4_mutex;

void *thread_a(void *arg)
{
    FILE* infile;
    uint8_t buf[456];
    infile = fopen("xiaoming.aac", "rb");
    
    while(1)
    {
        int rsz = fread(buf, 1, 455, infile);
        if(rsz != 455)
        {
            break;
        }

        pthread_mutex_lock(&mp4_mutex);
        for(int i = 0; i < 7 ; i++)
        {
            printf("%02x ",buf[i]);
        }
        int size;
        size |=((buf[3]&0x03)<<11);
		size |= buf[4]<<3;                //middle 8 bit
		size |= ((buf[5] & 0xe0)>>5); 
        printf("-------a:%d rsz:%d\n", size,rsz);
        MP4WriteSample(fileHandle, audio, &buf[7], size-7, MP4_INVALID_DURATION, 0, 1);
        pthread_mutex_unlock(&mp4_mutex);
    }

    return NULL;
}

void *thread_v(void *arg)
{
    int sps_flag =0;
    int pps_flag =0;
    FILE* infile;
    uint8_t *buf = (uint8_t *)malloc(100*1024*1024);
    uint8_t *p = buf;
    h264_stream_t* h = h264_new();
    size_t rsz = 0;
    size_t sz = 0;
    
    int nal_start, nal_end;
    
    infile = fopen("xiaoming.h264", "rb");
    while(1)
    {
        rsz = fread(buf + sz, 1, 100*1024*1024 - sz, infile);
        if(rsz == 0)
        {
            break;
        }
        sz += rsz;
        printf("sz = %ld\n", sz);
 
        int nal_size = 0;
        do
        {
            nal_size = find_nal_unit(p, sz, &nal_start, &nal_end);                        
            printf("nal_start : %d nal_end : %d nal_size : %d sz:%ld\n",nal_start,nal_end,nal_size,sz);
            
            if(nal_size <= 0)
            {
                break;
            }

            p += nal_start;
            read_nal_unit(h, p, nal_end - nal_start);

            pthread_mutex_lock(&mp4_mutex);
            if (sps_flag == 0 && h->nal->nal_unit_type == NAL_UNIT_TYPE_SPS )
            {
                //创建视频track //根据ISO/IEC 14496-10 可知sps的第二个，第三个，第四个字节分别是 AVCProfileIndication,profile_compat,AVCLevelIndication     其中90000/15  中的15>是fps
                video = MP4AddH264VideoTrack(fileHandle, 90000, 90000/15, 2560, 1440, p[1], p[2], p[3], 3);                  
                if(video == MP4_INVALID_TRACK_ID)  
                {  
                    MP4Close(fileHandle, 0);  
                    return NULL;  
                } 
                MP4AddH264SequenceParameterSet(fileHandle, video, p, nal_size);
                MP4SetVideoProfileLevel(fileHandle, 0x7F);
                sps_flag = 1;
                printf("SPS - nal_size : %d\n",nal_size);
            }
            else if(pps_flag == 0 && h->nal->nal_unit_type == NAL_UNIT_TYPE_PPS)
            {
                MP4AddH264PictureParameterSet(fileHandle, video, p, nal_size);
                pps_flag = 1;
                printf("PPS - nal_size : %d\n",nal_size);
            }
            else if(h->nal->nal_unit_type == NAL_UNIT_TYPE_CODED_SLICE_IDR)
            {
                uint8_t *t_p = p - 4;
                t_p[0] = (nal_size & 0xff000000) >> 24;  
                t_p[1] = (nal_size & 0x00ff0000) >> 16;  
                t_p[2] = (nal_size & 0x0000ff00) >> 8;  
                t_p[3] =  nal_size & 0x000000ff;  
                printf("I - nal_size : %d\n",nal_size);
                MP4WriteSample(fileHandle,video, t_p, nal_size + 4,MP4_INVALID_DURATION,0,true);                
            }
            else if(h->nal->nal_unit_type == NAL_UNIT_TYPE_CODED_SLICE_NON_IDR)
            {
                uint8_t *t_p = p - 4;
                t_p[0] = (nal_size & 0xff000000) >> 24;  
                t_p[1] = (nal_size & 0x00ff0000) >> 16;  
                t_p[2] = (nal_size & 0x0000ff00) >> 8;  
                t_p[3] =  nal_size & 0x000000ff; 
                printf("p - nal_size : %d\n",nal_size);
                MP4WriteSample(fileHandle,video, t_p, nal_size + 4,MP4_INVALID_DURATION,0,false);                
            }
            else
            {
                printf("%d - nal_size : %d\n",h->nal->nal_unit_type,nal_size);
            }
            pthread_mutex_unlock(&mp4_mutex);
            
            sz -= nal_end;
            if(nal_end >= sz)
            {
                memcpy(buf,p,nal_end - nal_start);
                p = buf;
                sz = nal_end - nal_start;
            }
            else
            {
                p += (nal_end - nal_start);                
            }

        }while(nal_size > 0);
    }

    free(buf);


    return NULL;
}

int main(int argc, char **argv)
{
    pthread_mutex_init(&mp4_mutex, NULL);

    fileHandle = MP4Create("test.mp4", 0);  
    if(fileHandle == MP4_INVALID_FILE_HANDLE)  
    {  
        return false;  
    }  
    MP4SetTimeScale(fileHandle, 90000);
    
    audio = MP4AddAudioTrack(fileHandle, 16000, 1024, MP4_MPEG2_AAC_LC_AUDIO_TYPE);  
    if(audio == MP4_INVALID_TRACK_ID)  
    {  
        MP4Close(fileHandle, 0);  
        return false;  
  
    }  
    MP4SetVideoProfileLevel(fileHandle, 0x7F);  
    MP4SetAudioProfileLevel(fileHandle, 0x02); 
    uint8_t ubuffer[2] = {20 , 8};
    MP4SetTrackESConfiguration(fileHandle, audio, &ubuffer[0], 2);  

    pthread_t thread_a_id = 0, thread_v_id = 0;
    int ret = pthread_create(&thread_a_id, NULL, &thread_a, NULL);
    if (ret != 0) 
    {
        printf("  [%s:%d] pthread_create thread_a_id failed\n", __FUNCTION__, __LINE__);
        exit(2);
    }
    
    ret = pthread_create(&thread_v_id, NULL, &thread_v, NULL);
    if (ret != 0) 
    {
        printf("  [%s:%d] pthread_create thread_v failed\n", __FUNCTION__, __LINE__);
        exit(3);
    }

    if( thread_a_id != 0 )
    {
        pthread_join(thread_a_id, NULL);
        thread_a_id = 0;
    }

    if( thread_v_id != 0 )
    {
        pthread_join(thread_v_id, NULL);
        thread_v_id = 0;
    }
    
    MP4Close(fileHandle,0);

    return 0;	
}