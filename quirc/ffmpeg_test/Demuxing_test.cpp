#include "Demuxing.h"

int main(int argc, const char *argv[]) {

    Demuxing demuxing;

    if(argc != 2)
	{
		printf("Usage:Demuxing filename\n");
		exit(1);
	}

    const char *input_mp4= argv[1];
    const char *output_pcm= "./audio.pcm";
    const char *output_yuv= "./video.yuv";
    // demuxing.Initialize(input_mp4,output_yuv,output_pcm);
    // demuxing.Start();

    demuxing.Initialize(input_mp4);
    demuxing.Start();

    int s;
    char *d;
    while(1)
    {
        d = demuxing.ReadVideoFrameData(&s);
        printf("size:%d\n",s);
        demuxing.RemoveVideoFrameData();
    }

    // ffplay -f f32le -ac 1 -ar 44100 ./cmake-build-debug/audio.pcm
    // ffplay -f rawvideo -pix_fmt yuv420p -video_size 1000x416 ./cmake-build-debug/video.yuv
    return 0;
}
