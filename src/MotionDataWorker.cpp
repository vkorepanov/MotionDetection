#include "MotionDataWorker.hpp"

#include <Poco/Format.h>
#include <Poco/Logger.h>
#include <cstdlib>
#include <memory>

namespace vehlwn {
MotionDataWorker::MotionDataWorker(
    std::shared_ptr<BackgroundSubtractorFactory> back_subtractor_factory,
    std::shared_ptr<VideoCaptureFactory> video_capture_factory,
    std::shared_ptr<PreprocessImageFactory> preprocess_image_factory,
    Poco::Logger& logger)
    : m_back_subtractor_factory{std::move(back_subtractor_factory)}
    , m_video_capture_factory{std::move(video_capture_factory)}
    , m_preprocess_image_factory{std::move(preprocess_image_factory)}
    , m_motion_data{std::make_shared<Mutex<MotionData>>()}
    , m_stopped{false}
    , m_logger{logger}
{
    poco_information(m_logger, "constructor MotionDataWorker");
}

MotionDataWorker::~MotionDataWorker()
{
    poco_information(m_logger, "destructor ~MotionDataWorker");
    if(!m_stopped)
        stop();
}

std::shared_ptr<const Mutex<MotionData>> MotionDataWorker::get_motion_data() const
{
    return m_motion_data;
}

void MotionDataWorker::start()
{
    m_stopped = false;
    auto back_subtractor = m_back_subtractor_factory->create();
    auto video_capture = m_video_capture_factory->create();
    m_fps = video_capture->get_fps();
    auto preprocess_filter = m_preprocess_image_factory->create();
    m_working_thread = std::thread{[=] {
        while(!m_stopped) {
            std::optional<cv::Mat> opt_frame = video_capture->read();
            if(!opt_frame) {
                poco_fatal(m_logger, "Cannot read more frames from a capture file");
                std::exit(1);
            }
            cv::Mat frame = std::move(*opt_frame);
            cv::Mat processed = preprocess_filter->apply(frame);
            cv::Mat fgmask = back_subtractor->apply(processed);
            (*m_motion_data->lock())
                .set_frame(std::move(frame))
                .set_fgmask(std::move(fgmask));
        }
    }};
}

void MotionDataWorker::stop()
{
    poco_information(m_logger, "Stopping...");
    m_stopped = true;
    if(m_working_thread.joinable()) {
        poco_information(m_logger, "Joining working thread...");
        m_working_thread.join();
        poco_information(m_logger, "Joined");
    }
}

double MotionDataWorker::get_fps() const
{
    return m_fps;
}
} // namespace vehlwn
