#include <iostream>

extern "C" {
#include <libavformat/avformat.h>
}

int main() {
    std::cout << avformat_configuration() << std::endl;
    return 0;
}
