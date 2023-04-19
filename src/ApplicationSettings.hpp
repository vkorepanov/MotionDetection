#pragma once

#include <Poco/Net/SocketAddress.h>
#include <Poco/Util/AbstractConfiguration.h>

#include <array>
#include <optional>
#include <string>
#include <variant>

namespace vehlwn {
struct ApplicationSettings {
    Poco::Net::SocketAddress http_server_host_and_port;

    struct VideoCapture {
        std::string filename;
        std::optional<std::string> file_format;
        std::optional<std::string> video_size;
        std::optional<std::string> framerate;
        std::optional<std::string> input_format;
    } video_capture;

    struct OutputFiles {
        std::string prefix;
        std::string extension;
        std::optional<std::string> video_bitrate;
        std::optional<std::string> audio_bitrate;
    } output_files;

    struct Logging {
        std::string log_level;
    } logging;

    struct BackgroundSubtractor {
        struct Knn {
            int history;
            double dist_2_threshold;
            bool detect_shadows;
        };
        struct Mog2 {
            int history;
            double var_threshold;
            bool detect_shadows;
        };
        std::variant<Knn, Mog2> algorithm;
    } background_subtractor;

    struct Preprocess {
        std::optional<double> resize_factor;
        struct NormalizedBox {
            int kernel_size;
        };
        struct Gaussian {
            int kernel_size;
            double sigma;
        };
        struct Median {
            int kernel_size;
        };
        std::optional<std::variant<NormalizedBox, Gaussian, Median>> smoothing;
    } preprocess;
};

ApplicationSettings
    read_settings(const Poco::Util::AbstractConfiguration& config) noexcept;
} // namespace vehlwn
