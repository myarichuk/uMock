#pragma once
#include "ComDefinitions.h"
#include <memory>
#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/RollingFileAppender.hh>
#include <log4cpp/PatternLayout.hh>
#include "RegistryHelper.hpp"
#include "ComUtils.hpp"

#define LOG_FILENAME "uMock.RewriteProfiler.log"
#define LAYOUT_PATTERN "%d [%p] %m%n"
#define MAX_LOG_SIZE_KB 5000000
#define MAX_BACKUP_INDEX 2
#ifdef _DEBUG
#define LOGGING_PRIORITY log4cpp::Priority::DEBUG
#else
#define LOGGING_PRIORITY log4cpp::Priority::WARN
#endif

using namespace log4cpp;
using namespace std;
class CLogger
{
public:	
	CLogger()
	{		
		static Appender *m_FileAppender;
		static Layout *m_Layout;
		static bool isLoggerInitialized;		

		if(!isLoggerInitialized) //guarantee constructor code will run only once
		{
			bool hasSucceededInitialization = false;
			wstring modulePath;
			CRegistryHelper::GetCOMClassLocation(__uuidof(CRewriteProfiler),&modulePath);	

			string logFilePath = CComUtils::WStringToString(modulePath);
			logFilePath = logFilePath.substr(0,logFilePath.length() - 1);
			
			try
			{				
				//TODO : check in stackoverflow if this would not give memory leak
				auto category = &Category::getInstance("");

				m_FileAppender = 
					new log4cpp::RollingFileAppender(
								string("rootAppender"),
								logFilePath + LOG_FILENAME,
								MAX_LOG_SIZE_KB,
								MAX_BACKUP_INDEX);
				
				m_Layout = new log4cpp::PatternLayout();
				
				((log4cpp::PatternLayout*)m_Layout)->setConversionPattern(LAYOUT_PATTERN);
				m_FileAppender->setLayout(m_Layout);

				category->setPriority(LOGGING_PRIORITY);				
				category->addAppender(*m_FileAppender);
				
				hasSucceededInitialization = true;
			}
			catch(exception e)
			{
				cerr << "Error initializing logger. Reason:" << e.what() << endl;
			}
			
			Category::getInstance("").info("--= Initialized logger... =--");
			isLoggerInitialized = hasSucceededInitialization;
		}
	}

	static void Shutdown()
	{		
		Category::getInstance("").info("--= Shutting down logger... =--");
		log4cpp::Category::shutdown();
	}

	void Info(const std::string &message)
	{
		Category::getInstance("").info(message);
	}

	void Info(const std::wstring &message)
	{
		Category::getInstance("").info(CComUtils::WStringToString(message));
	}

	void Warn(const std::string &message)
	{
		Category::getInstance("").warn(message);
	}

	void Warn(const std::wstring &message)
	{
		Category::getInstance("").warn(CComUtils::WStringToString(message));
	}

	void Debug(const std::string &message)
	{
		Category::getInstance("").debug(message);
	}

	void Debug(const std::wstring &message)
	{
		Category::getInstance("").debug(CComUtils::WStringToString(message));
	}

	void Error(const std::string &message)
	{
		Category::getInstance("").error(message);
	}

	void Error(const std::wstring &message)
	{
		Category::getInstance("").error(CComUtils::WStringToString(message));
	}
};
