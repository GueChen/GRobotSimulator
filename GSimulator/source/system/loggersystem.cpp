#include "system/loggersystem.h"

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace GComponent{

LoggerSystem& LoggerSystem::getInstance()
{
	static LoggerSystem instance;
	return instance;
}

LoggerSystem::LoggerSystem()
{
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::trace);
    console_sink->set_pattern("[%^%l%$] %v");

    const spdlog::sinks_init_list sink_list = { console_sink };

    spdlog::init_thread_pool(8192, 1);

    logger_ = std::make_shared<spdlog::async_logger>("logger",
        sink_list.begin(),
        sink_list.end(),
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);
    logger_->set_level(spdlog::level::trace);

    spdlog::register_logger(logger_);
}

LoggerSystem::~LoggerSystem() {
    logger_->flush();
    spdlog::drop_all();
}

}