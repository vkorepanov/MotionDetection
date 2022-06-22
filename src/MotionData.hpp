#pragma once

#include <opencv2/core/mat.hpp>

class MotionData {
public:
    MotionData(cv::Mat frame, cv::Mat fgmask)
        : m_frame{std::move(frame)}
        , m_fgmask{std::move(fgmask)}
    {}

    const cv::Mat& frame() const
    {
        return m_frame;
    }

    const cv::Mat& fgmask() const
    {
        return m_fgmask;
    }

private:
    cv::Mat m_frame;
    cv::Mat m_fgmask;
};
