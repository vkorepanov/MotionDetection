#pragma once

#include <stdexcept>
#include <string>

extern "C" {
#include <libavformat/avformat.h>
}
#include <boost/core/span.hpp>

#include "../ErrorWithContext.hpp"
#include "AvError.hpp"
#include "AvPacketAdapters.hpp"

namespace vehlwn::ffmpeg::detail {
class ScopedAvFormatOutput {
    AVFormatContext* m_raw = nullptr;

public:
    explicit ScopedAvFormatOutput(const char* const filename)
    {
        const int errnum
            = avformat_alloc_output_context2(&m_raw, nullptr, nullptr, filename);
        if(errnum < 0) {
            throw ErrorWithContext(
                "Could not create output context: ",
                AvError(errnum));
        }
    }
    ScopedAvFormatOutput(const ScopedAvFormatOutput&) = delete;
    ScopedAvFormatOutput(ScopedAvFormatOutput&& rhs) noexcept
    {
        swap(rhs);
    }
    ~ScopedAvFormatOutput()
    {
        avformat_free_context(m_raw);
    }
    ScopedAvFormatOutput& operator=(const ScopedAvFormatOutput&) = delete;
    ScopedAvFormatOutput& operator=(ScopedAvFormatOutput&& rhs) noexcept
    {
        swap(rhs);
        return *this;
    }
    void swap(ScopedAvFormatOutput& rhs) noexcept
    {
        std::swap(m_raw, rhs.m_raw);
    }

    [[nodiscard]] unsigned oformat_flags() const
    {
        return static_cast<unsigned>(m_raw->oformat->flags);
    }

    [[nodiscard]] AVStream* new_stream() const
    {
        AVStream* const ret = avformat_new_stream(m_raw, nullptr);
        if(ret == nullptr) {
            throw std::runtime_error("avformat_new_stream failed");
        }
        return ret;
    }

    using StreamsView = boost::span<AVStream*>;
    [[nodiscard]] StreamsView streams() const
    {
        return {m_raw->streams, m_raw->nb_streams};
    }

    void dump_format() const
    {
        av_dump_format(m_raw, 0, m_raw->url, 1);
    }

    void avio_open() const
    {
        const int errnum = ::avio_open(&m_raw->pb, m_raw->url, AVIO_FLAG_WRITE);
        if(errnum < 0) {
            throw ErrorWithContext("Could not open output file: ", AvError(errnum));
        }
    }

    void write_header() const
    {
        const int errnum = avformat_write_header(m_raw, nullptr);
        if(errnum < 0) {
            throw ErrorWithContext(
                "Error occurred when opening output file: ",
                AvError(errnum));
        }
    }

    void write_trailer() const
    {
        av_write_trailer(m_raw);
    }

    void interleaved_write_packet(OwningAvPacket& packet) const
    {
        const int errnum = av_interleaved_write_frame(m_raw, packet.raw());
        if(errnum < 0) {
            throw ErrorWithContext(
                "av_interleaved_write_frame failed: ",
                AvError(errnum));
        }
    }
};
} // namespace vehlwn::ffmpeg::detail
