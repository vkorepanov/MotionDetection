#pragma once

#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/rational.h>
}

#include <boost/core/span.hpp>

#include "ErrorWithContext.hpp"
#include "ffmpeg/ScopedAvDictionary.hpp"
#include "ffmpeg/detail/AvError.hpp"
#include "ffmpeg/detail/AvPacketAdapters.hpp"

namespace vehlwn::ffmpeg::detail {
class ScopedAvFormatInput {
    AVFormatContext* m_raw = nullptr;

public:
    ScopedAvFormatInput(
        const char* const url,
        const std::optional<std::string>& file_format,
        ScopedAvDictionary& options)
    {
        m_raw = nullptr;
        const AVInputFormat* format = nullptr;
        if(file_format) {
            format = av_find_input_format(file_format.value().data());
            if(!format) {
                throw std::runtime_error(
                    "av_find_input_format couldn't find specified format");
            }
        }
        const int errnum
            = avformat_open_input(&m_raw, url, format, options.double_ptr());
        if(errnum < 0)
            throw ErrorWithContext("avformat_open_input failed: ", AvError(errnum));
    }
    ScopedAvFormatInput(const ScopedAvFormatInput&) = delete;
    ScopedAvFormatInput(ScopedAvFormatInput&& rhs) noexcept
    {
        swap(rhs);
    }
    ~ScopedAvFormatInput()
    {
        std::cerr << "~ScopedAvFormatInput" << std::endl;
        avformat_close_input(&m_raw);
    }
    void swap(ScopedAvFormatInput& rhs) noexcept
    {
        std::swap(m_raw, rhs.m_raw);
    }

    using StreamsView = boost::span<AVStream*>;
    StreamsView streams() const
    {
        return StreamsView(m_raw->streams, m_raw->nb_streams);
    }

    const AVInputFormat* iformat() const
    {
        return m_raw->iformat;
    }
    void dump_format() const
    {
        av_dump_format(m_raw, 0, m_raw->url, 0);
    }
    void find_stream_info() const
    {
        const int errnum = avformat_find_stream_info(m_raw, nullptr);
        if(errnum < 0)
            throw ErrorWithContext(
                "avformat_find_stream_info failed: ",
                AvError(errnum));
    }
    OwningAvPacket read_packet()
    {
        OwningAvPacket ret;
        const int errnum = av_read_frame(m_raw, ret.raw());
        if(errnum < 0)
            throw ErrorWithContext("av_read_frame failed: ", AvError(errnum));
        return ret;
    }

    AVRational guess_frame_rate(AVStream* const stream) const
    {
        return av_guess_frame_rate(m_raw, stream, nullptr);
    }
};
} // namespace vehlwn::ffmpeg::detail
