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

class LoggerSystem final : public QObject
{
	Q_OBJECT

public:
	enum class LoggerObject {
		Cmd,
		Ui,
		File
	};

public:
	static LoggerSystem& getInstance();

	//template<typename... Args>
	//void Log(LoggerObject obj, Args&&... args) {
	//	switch (obj) {
	//	case LoggerObject::Cmd:
	//		//logger_->info(std::forward<Args>(args)...);

	//		break;
	//	case LoggerObject::Ui:
	//		//emit LoggerDisplayRequest(std::format(std::forward<Args>(args)...));
	//		break;
	//	case LoggerObject::File:

	//	}
	//}


protected:
	LoggerSystem();
	~LoggerSystem();

signals:
	void LoggerDisplayRequest(const std::string& context);

private:
	std::shared_ptr<spdlog::logger> logger_;

};

} // !namespace GComponent

#endif // !__GLOGGER_SYSTEM_H
