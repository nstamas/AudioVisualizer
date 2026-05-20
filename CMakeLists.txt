cmake_minimum_required(VERSION 3.20)
project(AudioVisualizer VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(ENABLE_TESTING "Enable unit tests" ON)
option(ENABLE_PROFILING "Enable performance profiling" OFF)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(nlohmann_json REQUIRED)

# Find FFmpeg libraries
# Try vcpkg's FindFFMPEG first (Windows-friendly)
find_package(FFMPEG QUIET)

if(NOT FFMPEG_FOUND)
    # Fall back to pkg-config for Linux/macOS
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(FFMPEG 
            libavcodec 
            libavformat 
            libavutil 
            libswscale
            libswresample
        )
    endif()
endif()

if(NOT FFMPEG_FOUND AND NOT FFMPEG_LIBRARIES)
    message(WARNING "FFmpeg not found. Video recording features will be disabled.")
    set(FFMPEG_INCLUDE_DIRS "")
    set(FFMPEG_LIBRARIES "")
endif()

# Find PortAudio
find_package(portaudio QUIET)
if(NOT portaudio_FOUND)
    pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)
endif()

# Find or build ProjectM
find_package(projectM QUIET)
if(NOT projectM_FOUND)
    message(STATUS "ProjectM not found, will use bundled version")
    # Add ProjectM as subdirectory if included
    # add_subdirectory(third_party/projectm)
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/third_party/imgui
    ${CMAKE_SOURCE_DIR}/third_party/stb
    ${FFMPEG_INCLUDE_DIRS}
)

# Dear ImGui
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/third_party/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# Source files
set(SOURCES
    src/main.cpp
    
    # Core
    src/core/Application.cpp
    src/core/AudioEngine.cpp
    src/core/VisualizationEngine.cpp
    
    # Graphics
    src/graphics/Renderer.cpp
    src/graphics/SpriteManager.cpp
    src/graphics/EffectsProcessor.cpp
    src/graphics/ShaderManager.cpp
    
    # UI
    src/ui/MainWindow.cpp
    src/ui/ControlPanel.cpp
    src/ui/PresetManager.cpp
    src/ui/LogWindow.cpp
    
    # Utils
    src/utils/Config.cpp
    src/utils/FileIO.cpp
    src/utils/Logger.cpp
)

# Header files
set(HEADERS
    include/AudioVisualizer.h
    
    # Core
    src/core/Application.h
    src/core/AudioEngine.h
    src/core/VisualizationEngine.h
    
    # Graphics
    src/graphics/Renderer.h
    src/graphics/SpriteManager.h
    src/graphics/EffectsProcessor.h
    src/graphics/ShaderManager.h
    
    # UI
    src/ui/MainWindow.h
    src/ui/ControlPanel.h
    src/ui/PresetManager.h
    src/ui/LogWindow.h
    
    # Utils
    src/utils/Config.h
    src/utils/FileIO.h
    src/utils/Logger.h
)

# Windows resource file for icon
if(WIN32)
    set(RESOURCE_FILES resources.rc)
endif()

# Create executable
if(WIN32)
    add_executable(${PROJECT_NAME} WIN32
        ${SOURCES} 
        ${HEADERS}
        ${IMGUI_SOURCES}
        ${RESOURCE_FILES}
    )
    # Allow main() instead of WinMain() while hiding console
    target_link_options(${PROJECT_NAME} PRIVATE "/ENTRY:mainCRTStartup")
else()
    add_executable(${PROJECT_NAME} 
        ${SOURCES} 
        ${HEADERS}
        ${IMGUI_SOURCES}
    )
endif()

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenGL::GL
    GLEW::GLEW
    glfw
    glm::glm
    nlohmann_json::nlohmann_json
)

# Conditionally add VideoRecorder if FFmpeg is available
if(FFMPEG_FOUND OR FFMPEG_LIBRARIES)
    target_sources(${PROJECT_NAME} PRIVATE src/core/VideoRecorder.cpp)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_FFMPEG=1)
endif()

# Link FFmpeg if available
if(FFMPEG_FOUND OR FFMPEG_LIBRARIES)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARIES})
    if(FFMPEG_INCLUDE_DIRS)
        target_include_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_INCLUDE_DIRS})
    endif()
endif()

# Link PortAudio
if(portaudio_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE portaudio)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE ${PORTAUDIO_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${PORTAUDIO_INCLUDE_DIRS})
endif()

# Link ProjectM if found
if(projectM_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE projectM::projectM)
endif()

# Platform-specific settings
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
    )
    # Copy DLLs to output directory
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND_EXPAND_LISTS
    )
elseif(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE dl pthread)
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE 
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
endif()

# Compiler warnings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX-)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE 
        -Wall -Wextra -Wpedantic
        $<$<CONFIG:Release>:-O3>
    )
endif()

# Copy assets to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_SOURCE_DIR}/icon.png
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/icon.png
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_SOURCE_DIR}/icon.png
    ${CMAKE_BINARY_DIR}/icon.png
)

# Install rules
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY assets/
    DESTINATION share/audiovisualizer/assets
)

# Print configuration summary
message(STATUS "")
message(STATUS "AudioVisualizer Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")

