#include "Demuxing.h"

// #include "string_util.h"
#include <ios>

Demuxing::Demuxing() = default;

Demuxing::~Demuxing() {

    av_freep(this->image_data_);

    av_free(this->av_packet_);
    av_free(this->av_frame_);

    avcodec_free_context(&this->video_codec_context_);
    avcodec_free_context(&this->audio_codec_context_);
    avformat_close_input(&format_context_);
    avformat_free_context(format_context_);

    this->output_video_stream_.close();
    this->output_audio_stream_.close();
};

void Demuxing::Initialize(std::string input_file, std::string output_video, std::string output_audio) {
    this->input_file_ = std::move(input_file);
    this->output_audio_ = std::move(output_audio);
    this->output_video_ = std::move(output_video);

    format_context_ = avformat_alloc_context();
    avformat_open_input(&format_context_, this->input_file_.c_str(), nullptr, nullptr);
    avformat_find_stream_info(format_context_, nullptr);

    OpenCodecContext(AVMEDIA_TYPE_VIDEO,
                     &this->video_codec_context_, &this->video_stream_index_);
    // OpenCodecContext(AVMEDIA_TYPE_AUDIO,
    //                  &this->audio_codec_context_, &this->audio_stream_index_);

    this->output_audio_stream_.open(this->output_audio_.c_str(), std::ios::binary | std::ios::out);
    this->output_video_stream_.open(this->output_video_.c_str(), std::ios::binary | std::ios::out);

    if (this->output_video_stream_.bad()) {
        throw std::runtime_error("Open output video file failed.");
    }
    if (this->output_audio_stream_.bad()) {
        throw std::runtime_error("Open output audio file failed.");
    }

    mode = 0;
}

void Demuxing::Initialize(std::string input_file) {
    this->input_file_ = std::move(input_file);

    format_context_ = avformat_alloc_context();
    avformat_open_input(&format_context_, this->input_file_.c_str(), nullptr, nullptr);
    avformat_find_stream_info(format_context_, nullptr);

    OpenCodecContext(AVMEDIA_TYPE_VIDEO,
                     &this->video_codec_context_, &this->video_stream_index_);
    // OpenCodecContext(AVMEDIA_TYPE_AUDIO,
    //                  &this->audio_codec_context_, &this->audio_stream_index_);

    mode = 1;
}

void Demuxing::Start() {
    av_dump_format(this->format_context_, 0, this->input_file_.c_str(), 0);

    this->av_packet_ = av_packet_alloc();
    this->av_frame_ = av_frame_alloc();
    AllocImage();
    if (this->av_packet_ == nullptr) {
        throw std::runtime_error("alloc packet failure.");
    }
    if (this->av_frame_ == nullptr) {
        throw std::runtime_error("alloc avframe failure.");
    }

    if(mode != 0) 
    {
        return;
    }

    while (av_read_frame(this->format_context_, this->av_packet_) >= 0) {
        int ret = 0;
        if (this->av_packet_->stream_index == this->audio_stream_index_) {
            // decode audio data
            DecodePacket(this->audio_codec_context_);
        } else if (this->av_packet_->stream_index == this->video_stream_index_) {
            // decode video data
            DecodePacket(this->video_codec_context_);
        }
        av_packet_unref(this->av_packet_);
        if (ret < 0) {
            break;
        }
    }
}

char * Demuxing::ReadVideoFrameData(int *size)
{
    while (av_read_frame(this->format_context_, this->av_packet_) >= 0) {
        int ret = 0;
        if (this->av_packet_->stream_index == this->video_stream_index_) {
            // decode video data
            return DecodePacket(this->video_codec_context_,size);
        }
        av_packet_unref(this->av_packet_);
        if (ret < 0) {
            break;
        }
    }

    return NULL;
}

void Demuxing::RemoveVideoFrameData()
{
    av_frame_unref(this->av_frame_);
    av_packet_unref(this->av_packet_);
}

int Demuxing::DecodePacket(AVCodecContext *codec_context) {
    int ret = 0;

    ret = avcodec_send_packet(codec_context, this->av_packet_);

    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        return 0;
    }
    if (ret == AVERROR(EINVAL)) {
        // throw std::runtime_error(vformat("avcodec_send_packet failure. error:%s", av_err2str(ret)));
        throw std::runtime_error("avcodec_send_packet failure. error");
    }
    if (ret == AVERROR_INPUT_CHANGED) {
        // throw std::runtime_error(vformat("avcodec_send_packet failure. error:%s", av_err2str(ret)));
        throw std::runtime_error("avcodec_send_packet failure. error:%s");
    }
    if (ret < 0) {
        // throw std::runtime_error(vformat("avcodec_send_packet failure. error:%s", av_err2str(ret)));
        throw std::runtime_error("avcodec_send_packet failure. error");
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(codec_context, this->av_frame_);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;
        }
        if (ret == AVERROR(EINVAL)) {
            // throw std::runtime_error(vformat("avcodec_receive_failure. error:%s", av_err2str(ret)));
            throw std::runtime_error("avcodec_receive_failure. error:s");
        }
        if (ret == AVERROR_INPUT_CHANGED) {
            // throw std::runtime_error(vformat("avcodec_receive_failure failure. error:%s", av_err2str(ret)));
            throw std::runtime_error("avcodec_receive_failure failure. error");
        }
        if (ret < 0) {
            return ret;
        }
        if (codec_context->codec_type == AVMEDIA_TYPE_AUDIO) {
            if(mode == 0)
            {
                ret = OutputAudioFrame(this->av_frame_);
            }
        } else if (codec_context->codec_type == AVMEDIA_TYPE_VIDEO) {
            if(mode == 0)
            {
                ret = OutputVideoFrame(this->av_frame_);
            }
        }
        av_frame_unref(this->av_frame_);
        // if dump frame failed , return ret;
        if (ret < 0) {
            return ret;
        }
    }

    return 0;
}

char * Demuxing::DecodePacket(AVCodecContext *codec_context,int *ddddd) {
    int ret = 0;

    ret = avcodec_send_packet(codec_context, this->av_packet_);

    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        return 0;
    }
    if (ret == AVERROR(EINVAL)) {
        // throw std::runtime_error(vformat("avcodec_send_packet failure. error:%s", av_err2str(ret)));
        throw std::runtime_error("avcodec_send_packet failure. error");
    }
    if (ret == AVERROR_INPUT_CHANGED) {
        // throw std::runtime_error(vformat("avcodec_send_packet failure. error:%s", av_err2str(ret)));
        throw std::runtime_error("avcodec_send_packet failure. error:%s");
    }
    if (ret < 0) {
        // throw std::runtime_error(vformat("avcodec_send_packet failure. error:%s", av_err2str(ret)));
        throw std::runtime_error("avcodec_send_packet failure. error");
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(codec_context, this->av_frame_);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;
        }
        if (ret == AVERROR(EINVAL)) {
            // throw std::runtime_error(vformat("avcodec_receive_failure. error:%s", av_err2str(ret)));
            throw std::runtime_error("avcodec_receive_failure. error:s");
        }
        if (ret == AVERROR_INPUT_CHANGED) {
            // throw std::runtime_error(vformat("avcodec_receive_failure failure. error:%s", av_err2str(ret)));
            throw std::runtime_error("avcodec_receive_failure failure. error");
        }
        if (ret < 0) {
            return NULL;
        }
        if (codec_context->codec_type == AVMEDIA_TYPE_AUDIO) {
            if(mode == 0)
            {
                ret = OutputAudioFrame(this->av_frame_);
            }
        } else if (codec_context->codec_type == AVMEDIA_TYPE_VIDEO) {
            ret = OutputVideoFrame(this->av_frame_);
            *ddddd = this->image_dst_buffer_size;
            return reinterpret_cast<char *>(this->image_data_[0]);
        }
        av_frame_unref(this->av_frame_);
        // if dump frame failed , return ret;
        if (ret < 0) {
            return NULL;
        }
    }

    return NULL;
}

int Demuxing::OutputAudioFrame(AVFrame *frame) {
    int unpadded_line_size = frame->nb_samples * av_get_bytes_per_sample(AVSampleFormat(frame->format));
    // std::cout << vformat("Write audio frame %d,size=%d", this->audio_frame_counter++, unpadded_line_size) << std::endl;
    output_audio_stream_.write(reinterpret_cast<const char *>(frame->extended_data[0]), unpadded_line_size);

    return 0;
}


int Demuxing::OutputVideoFrame(AVFrame *frame) {
    if (frame->width != this->video_codec_context_->width
        || frame->height != this->video_codec_context_->height
        || frame->format != this->video_codec_context_->pix_fmt) {
        throw std::runtime_error("The video frame width,height and fmt must same .");
    }
    av_image_copy(image_data_,
                  image_data_line_size_,
                  const_cast<const uint8_t ** > ( reinterpret_cast< uint8_t **>(frame->data)),
                  frame->linesize,
                  this->video_codec_context_->pix_fmt,
                  frame->width,
                  frame->height
    );
    // std::cout << vformat("Write video frame %d,size=%d,width=%d,height=%d,fmt=%s",
    //                      this->video_frame_counter++,
    //                      this->image_dst_buffer_size,
    //                      frame->width,
    //                      frame->height,
    //                      av_get_pix_fmt_name(AVPixelFormat(frame->format)))
    //           << std::endl;
    if(mode == 0)
    {
        output_video_stream_.write(reinterpret_cast<const char *>(this->image_data_[0]), this->image_dst_buffer_size);
    }

    return 0;
}

/**
 *
 * @param type 音频/视频
 */
void Demuxing::OpenCodecContext(AVMediaType type, AVCodecContext **codec_context, int *stream_index) const {
    // find video
    *stream_index = av_find_best_stream(format_context_,
                                        type, -1,
                                        -1, nullptr, 0);

    if (*stream_index < 0) {
        // stream not found
        // throw std::runtime_error(vformat("Find stream %s error :%s",
        //                                  av_get_media_type_string(type),
        //                                  av_err2str(*stream_index)));
        throw std::runtime_error("Find stream error");
    }

    AVStream *stream = format_context_->streams[*stream_index];

    if (stream == nullptr) {
        throw std::runtime_error("Find video stream failure.");
    }

    const AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (codec == nullptr) {
        throw std::runtime_error("Find video codec failure.");
    }
    *codec_context = avcodec_alloc_context3(codec);
    if (avcodec_parameters_to_context(*codec_context, stream->codecpar) < 0) {
        throw std::runtime_error("Fill parameters failure.");
    }
    if (avcodec_open2(*codec_context, codec, nullptr) < 0) {
        throw std::runtime_error("Open avcodec failure.");
    }

}

void Demuxing::AllocImage() {

    this->image_dst_buffer_size = av_image_alloc(this->image_data_,
                                                 this->image_data_line_size_,
                                                 this->video_codec_context_->width,
                                                 this->video_codec_context_->height,
                                                 this->video_codec_context_->pix_fmt,
                                                 1);
    if (this->image_dst_buffer_size < 0) {
        // throw std::runtime_error(vformat("Alloc image error,message:%s", av_err2str(this->image_dst_buffer_size)));
        throw std::runtime_error("Alloc image error,message");
    }
}
