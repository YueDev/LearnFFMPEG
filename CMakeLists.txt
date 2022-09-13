cmake_minimum_required(VERSION 3.23)
project(LearnFFMPEG)

set(CMAKE_CXX_STANDARD 14)

include_directories(${PROJECT_SOURCE_DIR}/ffmpeg/include)
link_directories(${PROJECT_SOURCE_DIR}/ffmpeg/lib)

add_executable(LearnFFMPEG main.cpp)

target_link_libraries(
        LearnFFMPEG
        avcodec
        avdevice
        avfilter
        avformat
        avutil
        postproc
        swresample
        swscale
        )
