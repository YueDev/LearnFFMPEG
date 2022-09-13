#include <iostream>
#include <string>

extern "C" {
#include <libavformat/avformat.h>
}

using std::cout;
using std::endl;
using std::string;

const auto VIDEO_PATH = "../video/small_bunny_1080p_60fps.mp4";

int main() {
    cout << "video: " << VIDEO_PATH << endl;

    auto *pFormatContext = avformat_alloc_context();

    if (!pFormatContext) {
        std::cout << "ERROR could not allocate memory for Format Context" << std::endl;
        return -1;
    }

    if (avformat_open_input(&pFormatContext, VIDEO_PATH, nullptr, nullptr) != 0) {
        std::cout << "ERROR avformat_open_input error: could not open video file" << std::endl;
        return -1;
    }

    cout << "========================" << endl;
    cout << "AVFormatContext" << endl;
    cout << "format: " << pFormatContext->iformat->name << endl;
    cout << "duration: " << pFormatContext->duration << endl;
    cout << "bit_rate: " << pFormatContext->bit_rate << endl;

    for (int i = 0; i < pFormatContext->nb_streams; ++i) {
        auto *pStream = pFormatContext->streams[i];
        auto *pParameters = pStream->codecpar;
        cout << "========================" << endl;
        cout << "AVSTREAM " << i<< endl;
        if (pParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            cout << "video resolution: " << pParameters->width << " * " << pParameters->height << endl;
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

        auto *pCodec = avcodec_find_decoder(pParameters->codec_id);
        if (pCodec == nullptr) {
            cout << "ERROR unsupported codec" << endl;
            return -1;
        }


    }

    return 0;
}
