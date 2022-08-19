#ifndef FFMPEG_DECODER_DEMUXING_H
#define FFMPEG_DECODER_DEMUXING_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/pixdesc.h>
#include <libavutil/error.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavutil/timestamp.h>
};

#include <fstream>
#include <iostream>

class Demuxing {

public:
    Demuxing();

    virtual ~Demuxing();

    /**
     * 初始化设置参数
     * @param inputFile  输入文件路径
     * @param outputVideo 输出视频文件路径
     * @param outputAudio 输入音频文件路径
     */
    void Initialize(std::string input_file, std::string output_video, std::string output_audio);

    void Initialize(std::string input_file);

    void Start();

    char * ReadVideoFrameData(int *size);
    void RemoveVideoFrameData();

private:
    int mode;
    std::string input_file_;
    std::string output_video_;
    std::string output_audio_;

    std::fstream output_audio_stream_;
    std::fstream output_video_stream_;

    AVFormatContext *format_context_ = nullptr;
    AVCodecContext *audio_codec_context_ = nullptr;
    AVCodecContext *video_codec_context_ = nullptr;

    int audio_stream_index_ = -1;
    int video_stream_index_ = -1;

    int audio_frame_counter = 0;
    int video_frame_counter = 1;

    uint8_t *image_data_[4] = {nullptr};
    int image_data_line_size_[4] = {0};
    int image_dst_buffer_size = 0;

    AVPacket *av_packet_ = nullptr;
    AVFrame *av_frame_ = nullptr;

    void OpenCodecContext(AVMediaType type, AVCodecContext **codec_context, int *stream_index) const;

    int DecodePacket(AVCodecContext *codec_context);
    char *DecodePacket(AVCodecContext *codec_context,int *ddddd);

    int OutputAudioFrame(AVFrame *frame);

    int OutputVideoFrame(AVFrame *frame);

    void AllocImage();
};
#endif //FFMPEG_DECODER_DEMUXING_H
