#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <iostream>
#include <string>
#include "stb_image_write.h"

extern "C" {
#include <libavcodec//avcodec.h>
#include <libavformat/avformat.h>
}

using std::cout;
using std::endl;
using std::string;

const auto VIDEO_PATH = "../video/small_bunny_1080p_60fps.mp4";

int decode_packet(AVCodecContext *pCodecContext, AVPacket *pPacket, AVFrame *pFrame);

void saveImage(const string &outFile, int width, int height, int comp, unsigned char *imageData);

int main() {

    cout << "video: " << VIDEO_PATH << endl;

    //allocate an AVFormatContext
    auto *pFormatContext = avformat_alloc_context();
    if (!pFormatContext) {
        std::cout << "ERROR could not allocate memory for Format Context" << std::endl;
        return -1;
    }
    //open input stream
    if (avformat_open_input(&pFormatContext, VIDEO_PATH, nullptr, nullptr) != 0) {
        std::cout << "ERROR avformat_open_input error: could not open video file" << std::endl;
        return -1;
    }

    //read AVFormatContext's message.
    cout << "========================" << endl;
    cout << "AVFormatContext" << endl;
    cout << "format: " << pFormatContext->iformat->name << endl;
    cout << "duration: " << pFormatContext->duration << endl;
    cout << "bit_rate: " << pFormatContext->bit_rate << endl;


    //video's codec
    const AVCodec *pCodec = nullptr;
    AVCodecParameters *pCodecParameters = nullptr;
    int video_stream_index = -1;

    for (int i = 0; i < pFormatContext->nb_streams; ++i) {
        //get AVStream and it's codec parameters.
        auto *pStream = pFormatContext->streams[i];
        auto *pParameters = pStream->codecpar;
        //read AVStream's message, it's usually more accurate than AVFormatContext's message.
        cout << "========================" << endl;
        cout << "AVSTREAM " << i << endl;
        if (pParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            cout << "video resolution: " << pParameters->width << " * " << pParameters->height << endl;
            if (video_stream_index == -1) {
                video_stream_index = i;
                pCodec = avcodec_find_decoder(pParameters->codec_id);
                pCodecParameters = pParameters;
                pCodec = avcodec_find_decoder(pParameters->codec_id);
                pCodecParameters = pParameters;
            }
        } else if (pParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
            cout << "audio: channels: " << pParameters->ch_layout.nb_channels << endl;
            cout << "audio: sample rate: " << pParameters->sample_rate << endl;
        } else {
            cout << "not video or audio: " << pParameters->codec_type << endl;
        }
        cout << "duration: " << pStream->duration << endl;
        cout << "bit_rate: " << pParameters->bit_rate << endl;
        cout << "frame_rate: " << pStream->r_frame_rate.num << " / " << pStream->r_frame_rate.den << endl;
        cout << "start_time: " << pStream->start_time << endl;
        cout << "base_time: " << pStream->time_base.num << " / " << pStream->time_base.den << endl;


    }

    cout << "========================" << endl;

    if (video_stream_index == -1) {
        cout << "the File does not contain a video stream" << endl;
        return -1;
    }

    if (pCodec == nullptr || pCodecParameters == nullptr) {
        cout << "ERROR unsupported codec" << endl;
        return -1;
    }

    //allocated memory for AVCodecContext
    auto pCodecContext = avcodec_alloc_context3(pCodec);
    if (pCodecContext == nullptr) {
        cout << "failed to allocated memory for AVCodecContext" << endl;
        return -1;
    }

    // Fill the codec context based on the values from pCodecParameters
    if (avcodec_parameters_to_context(pCodecContext, pCodecParameters) < 0) {
        cout << "failed to copy codec params to pCodecContext" << endl;
        return -1;
    }

    // Initialize the pCodecContext to use pCodec
    if (avcodec_open2(pCodecContext, pCodec, nullptr) < 0) {
        cout << "failed to open codec" << endl;
        return -1;
    }

    //Now we will read dates form stream. First allocate memory for AVPacket and AVFrame
    auto pPacket = av_packet_alloc();
    auto pFrame = av_frame_alloc();
    if (pPacket == nullptr || pFrame == nullptr) {
        cout << "failed to alloc AVPacket or AVFrame" << endl;
        return -1;
    }

    int max_response = 8;

    while (av_read_frame(pFormatContext, pPacket) >= 0) {
        if (pPacket->stream_index == video_stream_index) {
            cout << "==================" << endl;
            cout << "AVPacket->pts: " << pPacket->pts << endl;
            if (decode_packet(pCodecContext, pPacket, pFrame) < 0) break;
            max_response--;
            if (max_response <= 0) break;
        }
        av_packet_unref(pPacket);
    }

    cout << "releasing resources" << endl;

    avformat_close_input(&pFormatContext);
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
    avcodec_free_context(&pCodecContext);

    return 0;
}


int decode_packet(AVCodecContext *pCodecContext, AVPacket *pPacket, AVFrame *pFrame) {
    cout << "----decode packet----" << endl;
    int result = avcodec_send_packet(pCodecContext, pPacket);

    if (result < 0) {
        cout << "send packet error" << endl;
        cout << av_err2str(result) << endl;
        return result;
    }

    while (true) {
        result = avcodec_receive_frame(pCodecContext, pFrame);
        if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
            cout << "eagain or EOF" << endl;
            break;
        } else if (result < 0) {
            cout << "Error receive frame " << endl;
            string string1 = av_err2str(result);
            cout << string1 << endl;
            return result;
        }
        cout << "Frame_num: " << pCodecContext->frame_number << endl;
        cout << "type: " << av_get_picture_type_char(pFrame->pict_type) << endl;
        cout << "size: " << pFrame->pkt_size << "bytes" << endl;
        cout << "format: " << pFrame->format << endl;
        cout << "pts : " << pFrame->pts << endl;
        cout << "key_frame : " << pFrame->key_frame << endl;
        cout << "DTS : " << pFrame->coded_picture_number << endl;

        //save frame to png
        auto *pFrameData = pFrame->data[0];
        int comp = 3;
        long imageDataSize = pFrame->width * pFrame->height;
        auto imageData = new unsigned char[imageDataSize * comp]{};
        for (int i = 0; i < imageDataSize; ++i) {
            auto y = *(pFrameData + i);
            imageData[i * comp] = y;
            imageData[i * comp + 1] = y;
            imageData[i * comp + 2] = y;
        }


        string fileName = "../out/frame_"
                          + std::to_string(pCodecContext->frame_number)
                          + "_"
                          + std::to_string(pFrame->pts)
                          +".jpg";

        saveImage(fileName, pFrame->width, pFrame->height, comp, imageData);

        cout << "write picture" << endl;

        delete[] imageData;
    }

    return 0;
}

void saveImage(const string &outFile, int width, int height, int comp, unsigned char *imageData) {
//    stbi_write_png(outFile.c_str(), width, height, comp, imageData, width * comp);
    stbi_write_jpg(outFile.c_str(), width, height, comp, imageData, 75);
}