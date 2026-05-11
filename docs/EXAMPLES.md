# Example Configurations

## High-Quality Recording Setup

For recording high-quality videos suitable for YouTube, streaming, etc.

**config_hq_recording.json**
```json
{
  "audio": {
    "input_device": "default",
    "sample_rate": 48000,
    "buffer_size": 512,
    "fft_size": 4096
  },
  "visualization": {
    "width": 1920,
    "height": 1080,
    "fps": 60,
    "preset_path": "assets/presets/",
    "preset_duration": 45.0,
    "smooth_transitions": true,
    "beat_sensitivity": 1.2,
    "aspect_correction": true
  },
  "recording": {
    "codec": "libx264",
    "bitrate": "20M",
    "output_format": "mp4",
    "fps": 60
  },
  "effects": {
    "bloom": true,
    "vignette": true
  }
}
```

## Performance Mode

For lower-end systems or when performance is critical.

**config_performance.json**
```json
{
  "audio": {
    "sample_rate": 44100,
    "buffer_size": 256,
    "fft_size": 1024
  },
  "visualization": {
    "width": 1280,
    "height": 720,
    "fps": 30,
    "preset_duration": 20.0,
    "smooth_transitions": false,
    "beat_sensitivity": 1.0
  },
  "recording": {
    "bitrate": "5M",
    "fps": 30
  },
  "window": {
    "msaa_samples": 0
  }
}
```

## 4K Ultra Setup

For high-end systems with powerful GPUs.

**config_4k.json**
```json
{
  "visualization": {
    "width": 3840,
    "height": 2160,
    "fps": 60,
    "preset_duration": 60.0,
    "beat_sensitivity": 1.5
  },
  "recording": {
    "codec": "libx265",
    "bitrate": "50M",
    "output_format": "mp4",
    "fps": 60
  },
  "effects": {
    "bloom": true,
    "smearing": true,
    "blur": false
  },
  "window": {
    "msaa_samples": 8
  }
}
```

## Live Performance

Optimized for real-time visualization during live performances.

**config_live.json**
```json
{
  "audio": {
    "input_device": "line-in",
    "buffer_size": 128,
    "fft_size": 2048
  },
  "visualization": {
    "width": 1920,
    "height": 1080,
    "fps": 60,
    "preset_duration": 30.0,
    "smooth_transitions": true,
    "beat_sensitivity": 1.8
  },
  "recording": {
    "fps": 60
  },
  "window": {
    "fullscreen": true,
    "vsync": true
  }
}
```

## Audio File Processing

For processing pre-recorded audio files.

**config_file_processing.json**
```json
{
  "audio": {
    "input_device": "file",
    "sample_rate": 44100,
    "buffer_size": 1024,
    "fft_size": 4096
  },
  "visualization": {
    "width": 1920,
    "height": 1080,
    "fps": 30,
    "preset_duration": 60.0
  },
  "recording": {
    "codec": "libx264",
    "bitrate": "15M",
    "output_format": "mp4",
    "fps": 30
  }
}
```

## Usage

To use a custom configuration:

```bash
AudioVisualizer config_hq_recording.json
```

Or copy the desired configuration to `config.json` in the executable directory.
