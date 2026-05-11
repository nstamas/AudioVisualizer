#include "AudioEngine.h"
#include "utils/Logger.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace AudioVisualizer {
namespace Core {

AudioEngine::AudioEngine(const Utils::Config& config)
    : config_(config)
    , stream_(nullptr)
    , sampleRate_(44100)
    , bufferSize_(512)
    , fftSize_(2048)
    , captureWritePos_(0)
    , filePlaybackPosition_(0)
    , fileChannels_(0)
    , fileSampleRate_(0)
    , isPlayingFromFile_(false)
    , beatThreshold_(1.5f)
    , beatDecay_(0.95f)
    , lastBeatEnergy_(0.0f)
    , isInitialized_(false)
    , isRunning_(false)
{
    sampleRate_ = config_.GetInt("audio.sample_rate", 44100);
    bufferSize_ = config_.GetInt("audio.buffer_size", 512);
    fftSize_ = config_.GetInt("audio.fft_size", 2048);
    
    energyHistory_.resize(43); // 1 second at ~43 fps
}

AudioEngine::~AudioEngine() {
    Shutdown();
}

bool AudioEngine::Initialize() {
    Utils::Logger::Info("Initializing audio engine...");

    // Initialize PortAudio
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        Utils::Logger::Error("PortAudio initialization failed: " + std::string(Pa_GetErrorText(err)));
        return false;
    }

    // Set up input parameters
    inputParams_.device = Pa_GetDefaultInputDevice();
    if (inputParams_.device == paNoDevice) {
        Utils::Logger::Error("No default input device found");
        Pa_Terminate();
        return false;
    }

    const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(inputParams_.device);
    Utils::Logger::Info("Using audio device: " + std::string(deviceInfo->name));

    inputParams_.channelCount = 2;
    inputParams_.sampleFormat = paFloat32;
    inputParams_.suggestedLatency = deviceInfo->defaultLowInputLatency;
    inputParams_.hostApiSpecificStreamInfo = nullptr;

    // Set up output parameters for playback
    outputParams_.device = Pa_GetDefaultOutputDevice();
    if (outputParams_.device == paNoDevice) {
        Utils::Logger::Warning("No default output device found - audio playback disabled");
    } else {
        const PaDeviceInfo* outputDeviceInfo = Pa_GetDeviceInfo(outputParams_.device);
        Utils::Logger::Info("Using output device: " + std::string(outputDeviceInfo->name));
        
        outputParams_.channelCount = 2;
        outputParams_.sampleFormat = paFloat32;
        outputParams_.suggestedLatency = outputDeviceInfo->defaultLowOutputLatency;
        outputParams_.hostApiSpecificStreamInfo = nullptr;
    }

    // Open audio stream with both input and output
    err = Pa_OpenStream(
        &stream_,
        &inputParams_,
        (outputParams_.device != paNoDevice) ? &outputParams_ : nullptr,
        sampleRate_,
        bufferSize_,
        paClipOff,
        AudioCallback,
        this
    );

    if (err != paNoError) {
        Utils::Logger::Error("Failed to open audio stream: " + std::string(Pa_GetErrorText(err)));
        Pa_Terminate();
        return false;
    }

    // Initialize buffers
    inputBuffer_.resize(bufferSize_ * 2); // Stereo
    callbackBuffer_.resize(bufferSize_ * 2); // Pre-allocated callback buffer
    captureBuffer_.resize(sampleRate_ * 2); // 1 second capture buffer
    fftBuffer_.resize(fftSize_);
    fftOutput_.resize(fftSize_);
    
    currentData_.waveform.resize(bufferSize_);
    currentData_.spectrum.resize(fftSize_ / 2);
    currentData_.bands.resize(8); // 8 frequency bands

    isInitialized_ = true;
    Utils::Logger::Info("Audio engine initialized successfully");
    return true;
}

void AudioEngine::Shutdown() {
    if (!isInitialized_) {
        return;
    }

    Stop();

    if (stream_) {
        Pa_CloseStream(stream_);
        stream_ = nullptr;
    }

    Pa_Terminate();
    isInitialized_ = false;
}

void AudioEngine::Start() {
    if (!isInitialized_ || isRunning_) {
        return;
    }

    PaError err = Pa_StartStream(stream_);
    if (err == paNoError) {
        isRunning_ = true;
        Utils::Logger::Info("Audio stream started");
    } else {
        Utils::Logger::Error("Failed to start audio stream: " + std::string(Pa_GetErrorText(err)));
    }
}

void AudioEngine::Pause() {
    if (!isRunning_) {
        return;
    }

    Pa_StopStream(stream_);
    isRunning_ = false;
    Utils::Logger::Info("Audio stream paused");
}

void AudioEngine::Stop() {
    if (!isRunning_) {
        return;
    }

    Pa_StopStream(stream_);
    isRunning_ = false;
    
    // Clear buffers
    std::fill(inputBuffer_.begin(), inputBuffer_.end(), 0.0f);
    std::fill(fftBuffer_.begin(), fftBuffer_.end(), 0.0f);
    
    Utils::Logger::Info("Audio stream stopped");
}

void AudioEngine::Update() {
    if (!isRunning_) {
        return;
    }

    // Process captured audio data (not in real-time callback to avoid blocking)
    if (isPlayingFromFile_) {
        std::lock_guard<std::mutex> lock(captureMutex_);
        if (captureWritePos_ >= bufferSize_ * 2) {
            // Process the most recent buffer
            size_t readPos = (captureWritePos_ >= bufferSize_ * 2) ? 
                            (captureWritePos_ - bufferSize_ * 2) : 0;
            
            std::vector<float> tempBuffer(bufferSize_ * 2);
            for (int i = 0; i < bufferSize_ * 2; ++i) {
                tempBuffer[i] = captureBuffer_[(readPos + i) % captureBuffer_.size()];
            }
            
            ProcessAudioBuffer(tempBuffer.data(), bufferSize_);
        }
    }
}

AudioData AudioEngine::GetAudioData() {
    std::lock_guard<std::mutex> lock(dataMutex_);
    return currentData_;
}

int AudioEngine::AudioCallback(const void* inputBuffer,
                               void* outputBuffer,
                               unsigned long framesPerBuffer,
                               const PaStreamCallbackTimeInfo* timeInfo,
                               PaStreamCallbackFlags statusFlags,
                               void* userData)
{
    auto* engine = static_cast<AudioEngine*>(userData);
    float* output = static_cast<float*>(outputBuffer);
    
    if (engine->isPlayingFromFile_) {
        // Write file data directly to output buffer
        for (unsigned long frame = 0; frame < framesPerBuffer; ++frame) {
            if (engine->filePlaybackPosition_ < engine->fileAudioData_.size()) {
                float sample = engine->fileAudioData_[engine->filePlaybackPosition_];
                
                // Convert mono to stereo (duplicate to both channels)
                if (engine->fileChannels_ == 1) {
                    output[frame * 2] = sample;      // Left
                    output[frame * 2 + 1] = sample;  // Right
                    engine->filePlaybackPosition_++;
                } else {
                    // Stereo file
                    output[frame * 2] = sample;
                    if (engine->filePlaybackPosition_ + 1 < engine->fileAudioData_.size()) {
                        output[frame * 2 + 1] = engine->fileAudioData_[engine->filePlaybackPosition_ + 1];
                        engine->filePlaybackPosition_ += 2;
                    } else {
                        output[frame * 2 + 1] = sample;
                        engine->filePlaybackPosition_++;
                    }
                }
            } else {
                // Loop back to start
                engine->filePlaybackPosition_ = 0;
                output[frame * 2] = 0.0f;
                output[frame * 2 + 1] = 0.0f;
            }
        }
        
        // Quickly copy to capture buffer for later processing (avoid heavy processing in callback)
        {
            std::lock_guard<std::mutex> lock(engine->captureMutex_);
            for (unsigned long frame = 0; frame < framesPerBuffer; ++frame) {
                size_t writePos = (engine->captureWritePos_ + frame * 2) % engine->captureBuffer_.size();
                engine->captureBuffer_[writePos] = output[frame * 2];
                engine->captureBuffer_[writePos + 1] = output[frame * 2 + 1];
            }
            engine->captureWritePos_ = (engine->captureWritePos_ + framesPerBuffer * 2) % engine->captureBuffer_.size();
        }
    } else {
        // Use microphone input
        const float* input = static_cast<const float*>(inputBuffer);
        if (input) {
            engine->ProcessAudioBuffer(input, framesPerBuffer);
            
            // Pass through the microphone input to output (optional - can be disabled)
            if (output) {
                std::memcpy(output, input, framesPerBuffer * 2 * sizeof(float));
            }
        } else if (output) {
            // No input - output silence
            std::memset(output, 0, framesPerBuffer * 2 * sizeof(float));
        }
    }

    return paContinue;
}

void AudioEngine::ProcessAudioBuffer(const float* input, int frameCount) {
    std::lock_guard<std::mutex> lock(dataMutex_);

    // Copy input buffer
    std::memcpy(inputBuffer_.data(), input, frameCount * 2 * sizeof(float));

    // Convert stereo to mono for waveform
    currentData_.waveform.resize(frameCount);
    for (int i = 0; i < frameCount; ++i) {
        currentData_.waveform[i] = (input[i * 2] + input[i * 2 + 1]) * 0.5f;
    }

    // Copy to FFT buffer (pad with zeros if needed)
    std::fill(fftBuffer_.begin(), fftBuffer_.end(), 0.0f);
    for (int i = 0; i < std::min(frameCount, fftSize_); ++i) {
        fftBuffer_[i] = currentData_.waveform[i];
    }

    // Apply Hanning window
    for (int i = 0; i < fftSize_; ++i) {
        float window = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (fftSize_ - 1)));
        fftBuffer_[i] *= window;
    }

    // Perform FFT
    PerformFFT();

    // Analyze frequency bands
    AnalyzeBands();

    // Detect beat
    DetectBeat();

    // Calculate overall volume
    float sum = 0.0f;
    for (float sample : currentData_.waveform) {
        sum += std::abs(sample);
    }
    currentData_.volume = std::min(sum / frameCount, 1.0f);
}

void AudioEngine::PerformFFT() {
    // Simple DFT (in production, use FFTW3 or similar)
    // This is a placeholder implementation
    for (int k = 0; k < fftSize_ / 2; ++k) {
        float real = 0.0f;
        float imag = 0.0f;
        
        for (int n = 0; n < fftSize_; ++n) {
            float angle = 2.0f * M_PI * k * n / fftSize_;
            real += fftBuffer_[n] * std::cos(angle);
            imag -= fftBuffer_[n] * std::sin(angle);
        }
        
        fftOutput_[k] = std::complex<float>(real, imag);
        float magnitude = std::sqrt(real * real + imag * imag) / fftSize_;
        currentData_.spectrum[k] = std::min(magnitude * 2.0f, 1.0f);
    }
}

void AudioEngine::AnalyzeBands() {
    // Divide spectrum into frequency bands
    // Band ranges (Hz): 0-60 (sub-bass), 60-250 (bass), 250-500 (low mid),
    //                   500-2k (mid), 2k-4k (upper mid), 4k-6k (presence),
    //                   6k-12k (brilliance), 12k+ (air)
    
    const std::vector<int> bandRanges = {60, 250, 500, 2000, 4000, 6000, 12000, 22000};
    const int spectrumSize = currentData_.spectrum.size();
    
    for (size_t i = 0; i < currentData_.bands.size(); ++i) {
        int startHz = (i == 0) ? 0 : bandRanges[i - 1];
        int endHz = bandRanges[i];
        
        int startBin = (startHz * spectrumSize) / (sampleRate_ / 2);
        int endBin = (endHz * spectrumSize) / (sampleRate_ / 2);
        
        float sum = 0.0f;
        int count = 0;
        
        for (int j = startBin; j < endBin && j < spectrumSize; ++j) {
            sum += currentData_.spectrum[j];
            count++;
        }
        
        currentData_.bands[i] = (count > 0) ? (sum / count) : 0.0f;
    }
    
    // Extract bass, mid, treble
    currentData_.bass = (currentData_.bands[0] + currentData_.bands[1]) * 0.5f;
    currentData_.mid = (currentData_.bands[2] + currentData_.bands[3] + currentData_.bands[4]) / 3.0f;
    currentData_.treble = (currentData_.bands[5] + currentData_.bands[6] + currentData_.bands[7]) / 3.0f;
}

void AudioEngine::DetectBeat() {
    // Simple beat detection based on energy in bass frequencies
    float instantEnergy = 0.0f;
    for (int i = 0; i < 2; ++i) { // Use first two bands (sub-bass and bass)
        instantEnergy += currentData_.bands[i];
    }
    
    // Add to history
    energyHistory_.push_back(instantEnergy);
    if (energyHistory_.size() > 43) {
        energyHistory_.erase(energyHistory_.begin());
    }
    
    // Calculate average energy
    float avgEnergy = 0.0f;
    for (float energy : energyHistory_) {
        avgEnergy += energy;
    }
    avgEnergy /= energyHistory_.size();
    
    // Beat detection
    if (instantEnergy > beatThreshold_ * avgEnergy && instantEnergy > lastBeatEnergy_) {
        currentData_.beat = 1.0f;
        lastBeatEnergy_ = instantEnergy;
    } else {
        currentData_.beat *= beatDecay_;
        lastBeatEnergy_ *= beatDecay_;
    }
    
    currentData_.beat = std::max(0.0f, std::min(currentData_.beat, 1.0f));
}

std::vector<std::string> AudioEngine::GetAvailableDevices() {
    std::vector<std::string> devices;
    int deviceCount = Pa_GetDeviceCount();
    
    for (int i = 0; i < deviceCount; ++i) {
        const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
        if (deviceInfo->maxInputChannels > 0) {
            devices.push_back(deviceInfo->name);
        }
    }
    
    return devices;
}

bool AudioEngine::SetInputDevice(int deviceIndex) {
    if (isRunning_) {
        Stop();
    }
    
    inputParams_.device = deviceIndex;
    
    // Reopen stream with new device
    if (stream_) {
        Pa_CloseStream(stream_);
    }
    
    PaError err = Pa_OpenStream(
        &stream_,
        &inputParams_,
        nullptr,
        sampleRate_,
        bufferSize_,
        paClipOff,
        AudioCallback,
        this
    );
    
    return (err == paNoError);
}

void AudioEngine::SetSampleRate(int sampleRate) {
    sampleRate_ = sampleRate;
}

void AudioEngine::SetBufferSize(int bufferSize) {
    bufferSize_ = bufferSize;
}

void AudioEngine::SetFFTSize(int fftSize) {
    fftSize_ = fftSize;
}

bool AudioEngine::LoadAudioFile(const std::string& filePath) {
    Utils::Logger::Info("Loading audio file: " + filePath);
    
    // Simple WAV file loader
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        Utils::Logger::Error("Failed to open file: " + filePath);
        return false;
    }
    
    // Read WAV header
    char riff[4], wave[4], fmt[4], data[4];
    uint32_t chunkSize, subchunk1Size, subchunk2Size;
    uint16_t audioFormat, numChannels, bitsPerSample;
    uint32_t sampleRate, byteRate;
    uint16_t blockAlign;
    
    file.read(riff, 4);
    if (std::string(riff, 4) != "RIFF") {
        Utils::Logger::Error("Not a valid WAV file (missing RIFF header)");
        return false;
    }
    
    file.read(reinterpret_cast<char*>(&chunkSize), 4);
    file.read(wave, 4);
    if (std::string(wave, 4) != "WAVE") {
        Utils::Logger::Error("Not a valid WAV file (missing WAVE header)");
        return false;
    }
    
    file.read(fmt, 4);
    file.read(reinterpret_cast<char*>(&subchunk1Size), 4);
    file.read(reinterpret_cast<char*>(&audioFormat), 2);
    file.read(reinterpret_cast<char*>(&numChannels), 2);
    file.read(reinterpret_cast<char*>(&sampleRate), 4);
    file.read(reinterpret_cast<char*>(&byteRate), 4);
    file.read(reinterpret_cast<char*>(&blockAlign), 2);
    file.read(reinterpret_cast<char*>(&bitsPerSample), 2);
    
    // Skip any extra format bytes
    if (subchunk1Size > 16) {
        file.seekg(subchunk1Size - 16, std::ios::cur);
    }
    
    // Find data chunk
    file.read(data, 4);
    while (std::string(data, 4) != "data" && !file.eof()) {
        file.read(reinterpret_cast<char*>(&subchunk2Size), 4);
        file.seekg(subchunk2Size, std::ios::cur);
        file.read(data, 4);
    }
    
    if (file.eof()) {
        Utils::Logger::Error("No data chunk found in WAV file");
        return false;
    }
    
    file.read(reinterpret_cast<char*>(&subchunk2Size), 4);
    
    Utils::Logger::Info("WAV file info - Sample rate: " + std::to_string(sampleRate) + 
                       ", Channels: " + std::to_string(numChannels) +
                       ", Bits: " + std::to_string(bitsPerSample));
    
    // Only support PCM format for now
    if (audioFormat != 1) {
        Utils::Logger::Error("Only PCM WAV files are supported");
        return false;
    }
    
    // Read audio data
    size_t numSamples = subchunk2Size / (bitsPerSample / 8);
    fileAudioData_.resize(numSamples);
    
    if (bitsPerSample == 16) {
        std::vector<int16_t> tempData(numSamples);
        file.read(reinterpret_cast<char*>(tempData.data()), subchunk2Size);
        
        // Convert to float [-1, 1]
        for (size_t i = 0; i < numSamples; ++i) {
            fileAudioData_[i] = tempData[i] / 32768.0f;
        }
    } else if (bitsPerSample == 8) {
        std::vector<uint8_t> tempData(numSamples);
        file.read(reinterpret_cast<char*>(tempData.data()), subchunk2Size);
        
        // Convert to float [-1, 1]
        for (size_t i = 0; i < numSamples; ++i) {
            fileAudioData_[i] = (tempData[i] - 128) / 128.0f;
        }
    } else {
        Utils::Logger::Error("Unsupported bit depth: " + std::to_string(bitsPerSample));
        return false;
    }
    
    fileChannels_ = numChannels;
    fileSampleRate_ = sampleRate;
    filePlaybackPosition_ = 0;
    isPlayingFromFile_ = true;
    
    // Check for sample rate mismatch
    if (sampleRate != static_cast<uint32_t>(sampleRate_)) {
        Utils::Logger::Warning("File sample rate (" + std::to_string(sampleRate) + 
                              ") differs from stream sample rate (" + std::to_string(sampleRate_) + 
                              ") - playback speed may be incorrect");
    }
    
    Utils::Logger::Info("Successfully loaded " + std::to_string(numSamples) + " samples");
    return true;
}

} // namespace Core
} // namespace AudioVisualizer
