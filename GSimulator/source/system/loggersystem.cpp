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

void LoggerSystem::Log(LoggerObject obj, const std::string& str)
{
    switch (obj) {
        using enum LoggerObject;
    case Cmd:
        logger_->info(str);
        break;
    case Ui:
        emit LoggerDisplayRequest(str);
        break;
    case File:
        file_logger_->info(str);
    }
}

LoggerSystem::LoggerSystem()
{
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::trace);
    console_sink->set_pattern("[%^%l%$] %v");

    const spdlog::sinks_init_list sink_list = { console_sink };

    spdlog::init_thread_pool(8192, 2);
    
    logger_ = std::make_shared<spdlog::async_logger>("logger",
        sink_list.begin(),
        sink_list.end(),
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);

    logger_->set_level(spdlog::level::trace);
    
    spdlog::register_logger(logger_);
    
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("log.txt", 1024 * 1024 * 10);

    file_logger_ = std::make_shared<spdlog::async_logger>("file_logger", 
        file_sink, 
        spdlog::thread_pool(), 
        spdlog::async_overflow_policy::block);
    file_logger_->set_level(spdlog::level::info);
    spdlog::register_logger(file_logger_);

}

LoggerSystem::~LoggerSystem() {
    logger_->flush();
    spdlog::drop_all();
}

}