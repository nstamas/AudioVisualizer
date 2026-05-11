#include "VideoRecorder.h"
#include "utils/Logger.h"
#include <cstring>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

namespace AudioVisualizer {
namespace Core {

VideoRecorder::VideoRecorder(const Utils::Config& config)
    : config_(config)
    , width_(1920)
    , height_(1080)
    , fps_(60)
    , bitrate_(10000000) // 10 Mbps
    , codecName_("libx264")
    , outputFormat_("mp4")
    , formatContext_(nullptr)
    , codecContext_(nullptr)
    , videoStream_(nullptr)
    , frame_(nullptr)
    , rgbFrame_(nullptr)
    , packet_(nullptr)
    , swsContext_(nullptr)
    , frameCount_(0)
    , isInitialized_(false)
    , isRecording_(false)
{
    fps_ = config_.GetInt("recording.fps", 60);
    codecName_ = config_.GetString("recording.codec", "libx264");
    outputFormat_ = config_.GetString("recording.output_format", "mp4");
    
    std::string bitrateStr = config_.GetString("recording.bitrate", "10M");
    // Parse bitrate string (e.g., "10M" -> 10000000)
    if (bitrateStr.back() == 'M' || bitrateStr.back() == 'm') {
        bitrate_ = std::stoll(bitrateStr.substr(0, bitrateStr.size() - 1)) * 1000000;
    } else if (bitrateStr.back() == 'K' || bitrateStr.back() == 'k') {
        bitrate_ = std::stoll(bitrateStr.substr(0, bitrateStr.size() - 1)) * 1000;
    } else {
        bitrate_ = std::stoll(bitrateStr);
    }
}

VideoRecorder::~VideoRecorder() {
    Shutdown();
}

bool VideoRecorder::Initialize(int width, int height) {
    Utils::Logger::Info("Initializing video recorder...");

    width_ = width;
    height_ = height;

    frameBuffer_.resize(width_ * height_ * 4); // RGBA

    isInitialized_ = true;
    Utils::Logger::Info("Video recorder initialized");
    return true;
}

void VideoRecorder::Shutdown() {
    if (!isInitialized_) {
        return;
    }

    if (isRecording_) {
        StopRecording();
    }

    isInitialized_ = false;
}

bool VideoRecorder::StartRecording(const std::string& outputPath) {
    if (isRecording_ || !isInitialized_) {
        return false;
    }

    outputPath_ = outputPath;
    Utils::Logger::Info("Starting video recording: " + outputPath_);

    if (!InitializeEncoder()) {
        Utils::Logger::Error("Failed to initialize encoder");
        return false;
    }

    frameCount_ = 0;
    isRecording_ = true;
    Utils::Logger::Info("Recording started");
    return true;
}

void VideoRecorder::StopRecording() {
    if (!isRecording_) {
        return;
    }

    Utils::Logger::Info("Stopping video recording...");

    FlushEncoder();
    CleanupEncoder();

    isRecording_ = false;
    Utils::Logger::Info("Recording stopped. Frames written: " + std::to_string(frameCount_));
}

bool VideoRecorder::InitializeEncoder() {
    int ret;

    // Allocate output format context
    avformat_alloc_output_context2(&formatContext_, nullptr, 
                                   outputFormat_.c_str(), 
                                   outputPath_.c_str());
    if (!formatContext_) {
        Utils::Logger::Error("Could not allocate output format context");
        return false;
    }

    // Find encoder
    const AVCodec* codec = avcodec_find_encoder_by_name(codecName_.c_str());
    if (!codec) {
        Utils::Logger::Error("Codec not found: " + codecName_);
        return false;
    }

    // Create video stream
    videoStream_ = avformat_new_stream(formatContext_, nullptr);
    if (!videoStream_) {
        Utils::Logger::Error("Could not create video stream");
        return false;
    }

    // Allocate codec context
    codecContext_ = avcodec_alloc_context3(codec);
    if (!codecContext_) {
        Utils::Logger::Error("Could not allocate codec context");
        return false;
    }

    // Set codec parameters
    codecContext_->codec_id = codec->id;
    codecContext_->codec_type = AVMEDIA_TYPE_VIDEO;
    codecContext_->width = width_;
    codecContext_->height = height_;
    codecContext_->time_base = AVRational{1, fps_};
    codecContext_->framerate = AVRational{fps_, 1};
    codecContext_->pix_fmt = AV_PIX_FMT_YUV420P;
    codecContext_->bit_rate = bitrate_;
    codecContext_->gop_size = 12;
    codecContext_->max_b_frames = 2;

    // Set H.264 specific options
    if (codec->id == AV_CODEC_ID_H264) {
        av_opt_set(codecContext_->priv_data, "preset", "medium", 0);
        av_opt_set(codecContext_->priv_data, "crf", "23", 0);
    }

    if (formatContext_->oformat->flags & AVFMT_GLOBALHEADER) {
        codecContext_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    // Open codec
    ret = avcodec_open2(codecContext_, codec, nullptr);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        Utils::Logger::Error("Could not open codec: " + std::string(errbuf));
        return false;
    }

    // Copy codec parameters to stream
    ret = avcodec_parameters_from_context(videoStream_->codecpar, codecContext_);
    if (ret < 0) {
        Utils::Logger::Error("Could not copy codec parameters");
        return false;
    }

    videoStream_->time_base = codecContext_->time_base;

    // Allocate frames
    frame_ = av_frame_alloc();
    if (!frame_) {
        Utils::Logger::Error("Could not allocate frame");
        return false;
    }

    frame_->format = codecContext_->pix_fmt;
    frame_->width = width_;
    frame_->height = height_;

    ret = av_frame_get_buffer(frame_, 0);
    if (ret < 0) {
        Utils::Logger::Error("Could not allocate frame buffer");
        return false;
    }

    // Allocate RGB frame for conversion
    rgbFrame_ = av_frame_alloc();
    if (!rgbFrame_) {
        Utils::Logger::Error("Could not allocate RGB frame");
        return false;
    }

    rgbFrame_->format = AV_PIX_FMT_RGBA;
    rgbFrame_->width = width_;
    rgbFrame_->height = height_;

    ret = av_frame_get_buffer(rgbFrame_, 0);
    if (ret < 0) {
        Utils::Logger::Error("Could not allocate RGB frame buffer");
        return false;
    }

    // Initialize SWS context for color space conversion
    swsContext_ = sws_getContext(
        width_, height_, AV_PIX_FMT_RGBA,
        width_, height_, AV_PIX_FMT_YUV420P,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );

    if (!swsContext_) {
        Utils::Logger::Error("Could not initialize SWS context");
        return false;
    }

    // Allocate packet
    packet_ = av_packet_alloc();
    if (!packet_) {
        Utils::Logger::Error("Could not allocate packet");
        return false;
    }

    // Open output file
    if (!(formatContext_->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&formatContext_->pb, outputPath_.c_str(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, errbuf, sizeof(errbuf));
            Utils::Logger::Error("Could not open output file: " + std::string(errbuf));
            return false;
        }
    }

    // Write header
    ret = avformat_write_header(formatContext_, nullptr);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        Utils::Logger::Error("Error writing header: " + std::string(errbuf));
        return false;
    }

    Utils::Logger::Info("Encoder initialized successfully");
    return true;
}

void VideoRecorder::CleanupEncoder() {
    if (packet_) {
        av_packet_free(&packet_);
    }

    if (swsContext_) {
        sws_freeContext(swsContext_);
        swsContext_ = nullptr;
    }

    if (rgbFrame_) {
        av_frame_free(&rgbFrame_);
    }

    if (frame_) {
        av_frame_free(&frame_);
    }

    if (codecContext_) {
        avcodec_free_context(&codecContext_);
    }

    if (formatContext_) {
        if (!(formatContext_->oformat->flags & AVFMT_NOFILE)) {
            avio_closep(&formatContext_->pb);
        }
        avformat_free_context(formatContext_);
        formatContext_ = nullptr;
    }
}

void VideoRecorder::WriteFrame(const uint8_t* rgbaData) {
    if (!isRecording_) {
        return;
    }

    // Copy RGBA data to RGB frame
    for (int y = 0; y < height_; ++y) {
        std::memcpy(
            rgbFrame_->data[0] + y * rgbFrame_->linesize[0],
            rgbaData + y * width_ * 4,
            width_ * 4
        );
    }

    // Convert RGBA to YUV420P
    sws_scale(
        swsContext_,
        rgbFrame_->data, rgbFrame_->linesize,
        0, height_,
        frame_->data, frame_->linesize
    );

    frame_->pts = frameCount_;
    EncodeFrame(frame_);
    frameCount_++;
}

void VideoRecorder::WriteFrame(const float* rgbaData) {
    if (!isRecording_) {
        return;
    }

    // Convert float RGBA to uint8_t RGBA
    for (int i = 0; i < width_ * height_ * 4; ++i) {
        frameBuffer_[i] = static_cast<uint8_t>(rgbaData[i] * 255.0f);
    }

    WriteFrame(frameBuffer_.data());
}

void VideoRecorder::EncodeFrame(AVFrame* frame) {
    int ret;

    // Send frame to encoder
    ret = avcodec_send_frame(codecContext_, frame);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        Utils::Logger::Error("Error sending frame: " + std::string(errbuf));
        return;
    }

    // Receive encoded packets
    while (ret >= 0) {
        ret = avcodec_receive_packet(codecContext_, packet_);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, errbuf, sizeof(errbuf));
            Utils::Logger::Error("Error receiving packet: " + std::string(errbuf));
            return;
        }

        // Rescale packet timestamp
        av_packet_rescale_ts(packet_, codecContext_->time_base, videoStream_->time_base);
        packet_->stream_index = videoStream_->index;

        // Write packet
        ret = av_interleaved_write_frame(formatContext_, packet_);
        if (ret < 0) {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, errbuf, sizeof(errbuf));
            Utils::Logger::Error("Error writing packet: " + std::string(errbuf));
        }

        av_packet_unref(packet_);
    }
}

void VideoRecorder::FlushEncoder() {
    // Flush remaining frames
    EncodeFrame(nullptr);

    // Write trailer
    av_write_trailer(formatContext_);
}

void VideoRecorder::SetCodec(const std::string& codec) {
    codecName_ = codec;
}

void VideoRecorder::SetBitrate(int64_t bitrate) {
    bitrate_ = bitrate;
}

void VideoRecorder::SetFrameRate(int fps) {
    fps_ = fps;
}

void VideoRecorder::SetOutputFormat(const std::string& format) {
    outputFormat_ = format;
}

} // namespace Core
} // namespace AudioVisualizer
