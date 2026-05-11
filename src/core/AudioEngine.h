#ifndef AUDIOVISUALIZER_AUDIOENGINE_H
#define AUDIOVISUALIZER_AUDIOENGINE_H

#include "utils/Config.h"
#include <portaudio.h>
#include <vector>
#include <complex>
#include <mutex>

namespace AudioVisualizer {
namespace Core {

struct AudioData {
    std::vector<float> waveform;      // Time domain data
    std::vector<float> spectrum;      // Frequency domain data (FFT)
    std::vector<float> bands;         // Frequency bands (bass, mid, treble, etc.)
    float bass;                        // Bass intensity (0-1)
    float mid;                         // Mid frequency intensity (0-1)
    float treble;                      // Treble intensity (0-1)
    float volume;                      // Overall volume (0-1)
    float beat;                        // Beat detection (0-1)
};

class AudioEngine {
public:
    explicit AudioEngine(const Utils::Config& config);
    ~AudioEngine();

    bool Initialize();
    void Shutdown();

    void Start();
    void Pause();
    void Stop();

    void Update();
    AudioData GetAudioData();

    // Audio source selection
    bool SetInputDevice(int deviceIndex);
    bool LoadAudioFile(const std::string& filePath);
    std::vector<std::string> GetAvailableDevices();

    // Configuration
    void SetSampleRate(int sampleRate);
    void SetBufferSize(int bufferSize);
    void SetFFTSize(int fftSize);

private:
    void ProcessAudioBuffer(const float* input, int frameCount);
    void PerformFFT();
    void AnalyzeBands();
    void DetectBeat();

    static int AudioCallback(const void* inputBuffer,
                            void* outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void* userData);

    Utils::Config config_;
    
    // PortAudio
    PaStream* stream_;
    PaStreamParameters inputParams_;
    PaStreamParameters outputParams_;
    int sampleRate_;
    int bufferSize_;
    
    // Audio processing
    std::vector<float> inputBuffer_;
    std::vector<float> fftBuffer_;
    std::vector<std::complex<float>> fftOutput_;
    int fftSize_;
    
    // Audio data
    AudioData currentData_;
    std::mutex dataMutex_;
    
    // Audio capture for processing (avoid processing in callback)
    std::vector<float> captureBuffer_;
    size_t captureWritePos_;
    std::mutex captureMutex_;
    
    // File playback
    std::vector<float> fileAudioData_;
    size_t filePlaybackPosition_;
    int fileChannels_;
    int fileSampleRate_;
    bool isPlayingFromFile_;
    std::vector<float> callbackBuffer_;  // Pre-allocated buffer for callbacks
    
    // Beat detection
    float beatThreshold_;
    float beatDecay_;
    float lastBeatEnergy_;
    std::vector<float> energyHistory_;
    
    bool isInitialized_;
    bool isRunning_;
};

} // namespace Core
} // namespace AudioVisualizer

#endif // AUDIOVISUALIZER_AUDIOENGINE_H
