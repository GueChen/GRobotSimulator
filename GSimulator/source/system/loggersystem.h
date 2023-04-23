/**
 *  @file  	loggersystem.h
 *  @brief 	This class used for record all neccessary message to logger board or file for testing data.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Mar 31th, 2023
 **/
#ifndef __GLOGGER_SYSTEM_H
#define __GLOGGER_SYSTEM_H

#include "base/singleton.h"

#include <spdlog/spdlog.h>
#include <format>

#include <QtCore/QObject>

namespace GComponent {
enum class LoggerObject {
	Cmd,
	Ui,
	File
};
class LoggerSystem final : public QObject
{
	Q_OBJECT
public:


public:
	static LoggerSystem& getInstance();

	void Log(LoggerObject obj, const std::string& str);


protected:
	LoggerSystem();
	~LoggerSystem();

signals:
	void LoggerDisplayRequest(const std::string& context);

private:
	std::shared_ptr<spdlog::logger> logger_;
	std::shared_ptr<spdlog::logger> file_logger_;

};

} // !namespace GComponent

#endif // !__GLOGGER_SYSTEM_H
