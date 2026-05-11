#ifndef AUDIOVISUALIZER_VIDEORECORDER_H
#define AUDIOVISUALIZER_VIDEORECORDER_H

#include "utils/Config.h"
#include <string>
#include <vector>

// Forward declarations for FFmpeg
extern "C" {
    struct AVFormatContext;
    struct AVCodecContext;
    struct AVStream;
    struct AVFrame;
    struct AVPacket;
    struct SwsContext;
}

namespace AudioVisualizer {
namespace Core {

class VideoRecorder {
public:
    explicit VideoRecorder(const Utils::Config& config);
    ~VideoRecorder();

    bool Initialize(int width, int height);
    void Shutdown();

    bool StartRecording(const std::string& outputPath);
    void StopRecording();
    bool IsRecording() const { return isRecording_; }

    void WriteFrame(const uint8_t* rgbaData);
    void WriteFrame(const float* rgbaData);

    // Configuration
    void SetCodec(const std::string& codec);
    void SetBitrate(int64_t bitrate);
    void SetFrameRate(int fps);
    void SetOutputFormat(const std::string& format);

private:
    bool InitializeEncoder();
    void CleanupEncoder();
    void EncodeFrame(AVFrame* frame);
    void FlushEncoder();

    Utils::Config config_;

    // Video parameters
    int width_;
    int height_;
    int fps_;
    int64_t bitrate_;
    std::string codecName_;
    std::string outputFormat_;
    std::string outputPath_;

    // FFmpeg contexts
    AVFormatContext* formatContext_;
    AVCodecContext* codecContext_;
    AVStream* videoStream_;
    AVFrame* frame_;
    AVFrame* rgbFrame_;
    AVPacket* packet_;
    SwsContext* swsContext_;

    // Frame tracking
    int64_t frameCount_;
    bool isInitialized_;
    bool isRecording_;

    // Temporary buffer
    std::vector<uint8_t> frameBuffer_;
};

} // namespace Core
} // namespace AudioVisualizer

#endif // AUDIOVISUALIZER_VIDEORECORDER_H
