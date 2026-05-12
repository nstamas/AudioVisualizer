# Contributing to Audio Visualizer

Thank you for your interest in contributing! This document provides guidelines for contributing to the project.

## Getting Started

1. Fork the repository
2. Clone your fork: `git clone https://github.com/yourusername/AudioVisualizer.git`
3. Create a feature branch: `git checkout -b feature/my-new-feature`
4. Make your changes
5. Commit with clear messages: `git commit -am 'Add new feature'`
6. Push to your fork: `git push origin feature/my-new-feature`
7. Create a Pull Request

## Development Setup

Follow the instructions in `docs/SETUP.md` to set up your development environment.

## Code Style

### C++ Style Guidelines

- Use 4 spaces for indentation (no tabs)
- Opening braces on the same line
- Class names use PascalCase: `AudioEngine`
- Function names use PascalCase: `Initialize()`
- Variable names use camelCase: `isPlaying_`
- Member variables end with underscore: `width_`
- Constants use UPPER_SNAKE_CASE: `MAX_BUFFER_SIZE`
- Use C++17 features when appropriate
- Prefer smart pointers over raw pointers
- Use const-correctness

Example:
```cpp
class MyClass {
public:
    MyClass();
    ~MyClass();

    void DoSomething(int value);
    int GetValue() const { return value_; }

private:
    int value_;
    std::unique_ptr<Resource> resource_;
};
```

### File Organization

- Header files: `.h` extension
- Implementation files: `.cpp` extension
- One class per file pair (header + implementation)
- Use header guards: `#ifndef AUDIOVISUALIZER_CLASSNAME_H`
- Include order:
  1. Corresponding header
  2. C++ standard library
  3. Third-party libraries
  4. Project headers

### Comments

- Use `//` for single-line comments
- Use `/* */` for multi-line comments
- Document public APIs with clear descriptions
- Explain "why" not "what" in implementation comments
- Add TODO comments for future improvements: `// TODO: Implement this feature`

## Areas for Contribution

### High Priority

- [ ] Complete ProjectM integration
- [ ] Implement FFT using FFTW3
- [ ] Add file dialog support (Windows/Linux/macOS)
- [ ] Implement custom shader system
- [ ] Add audio file loading (MP3, WAV, FLAC)
- [ ] Optimize rendering pipeline
- [ ] Add preset creation/editing tools

### Medium Priority

- [ ] Implement all visual effects (masking, outline, shadow, etc.)
- [ ] Add video sprite support
- [ ] Implement GIF loading and animation
- [ ] Add beat detection improvements
- [ ] Create preset sequencer
- [ ] Add MIDI input support
- [ ] Implement plugin system

### Low Priority

- [ ] Add scripting support (Lua or Python)
- [ ] Create web interface for remote control
- [ ] Add VR/AR support
- [ ] Implement machine learning beat detection
- [ ] Add multi-monitor support
- [ ] Create mobile companion app

## Testing

- Write unit tests for new features
- Test on multiple platforms (Windows, Linux, macOS)
- Test with different audio devices
- Verify performance on low-end hardware
- Check memory leaks with Valgrind or similar tools

## Pull Request Guidelines

### Before Submitting

- [ ] Code follows the style guidelines
- [ ] All tests pass
- [ ] New code has appropriate comments
- [ ] Documentation is updated if needed
- [ ] No compiler warnings
- [ ] Code builds on multiple platforms (if possible)

### PR Description

Include:
- Clear description of changes
- Motivation for the change
- Screenshots/videos for visual changes
- Breaking changes (if any)
- Related issues

### Review Process

1. Automated checks must pass (CI/CD)
2. Code review by maintainers
3. Address review comments
4. Merge when approved

## Bug Reports

When reporting bugs, include:
- Operating system and version
- GPU model and driver version
- Steps to reproduce
- Expected behavior
- Actual behavior
- Screenshots/logs if applicable
- Configuration file (if relevant)

Use the GitHub issue template.

## Feature Requests

When requesting features:
- Describe the feature clearly
- Explain the use case
- Provide examples or mockups
- Consider implementation complexity
- Check if similar features exist

## Performance Optimization

When optimizing:
- Profile before optimizing
- Document performance improvements with benchmarks
- Consider trade-offs (speed vs. memory vs. quality)
- Test on various hardware configurations

## Documentation

- Update README.md for major changes
- Add/update API documentation
- Create tutorials for new features
- Update setup guides
- Add examples

## Community

- Be respectful and constructive
- Help other contributors
- Share knowledge and best practices
- Report security vulnerabilities privately

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

## Questions?

- Open a GitHub Discussion
- Check existing issues and documentation
- Contact maintainers

Thank you for contributing to Audio Visualizer! 🎵🎨
