#include <iostream>
//#define _ELPP_STRICT_ROLLOUT
//#define ELPP_DISABLE_DEBUG_LOGS
#define ELPP_THREAD_SAFE 
//#define ELPP_FORCE_USE_STD_THREAD
//#define ELPP_DISABLE_INFO_LOGS
#include "../JackieNet/EasyLog.h"
INITIALIZE_EASYLOGGINGPP

#include "JackieNet/DefaultNetDefines.h"

#include "JackieNet/ServerApplication.h"
#include "JackieNet/MessageID.h"
#include "JackieNet/IPlugin.h"

using namespace JACKIE_INET;

int main(int argc, char** argv)
{

	el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);
	el::Loggers::addFlag(el::LoggingFlag::CreateLoggerAutomatically);

	el::Configurations defaultConf;

	// set to default config
	//defaultConf.setToDefault(); 

	//// To set GLOBAL configurations you may use including all levels 
	//defaultConf.setGlobally(el::ConfigurationType::ToStandardOutput, "false");
	//defaultConf.setGlobally(el::ConfigurationType::MaxLogFileSize, "1");
	defaultConf.setGlobally(el::ConfigurationType::Format, "[%level][%datetime][%logger][tid %thread] InFile[%fbase] AtLine[%line]\n%msg\n");

	// set individual option, @NOTICE you have to set this after setGlobally() to make it work
	//defaultConf.set(el::Level::Info, el::ConfigurationType::Format, "%datetime %level %msg");
	defaultConf.set(el::Level::Debug, el::ConfigurationType::Format, "[%level][%datetime][%logger][tid %thread] AtLine[%line]\n%msg\n");
	defaultConf.set(el::Level::Trace, el::ConfigurationType::Format, "[%level][%logger][tid %thread] AtLine[%line]\n%msg\n");
	defaultConf.set(el::Level::Info, el::ConfigurationType::Format, "[%level][%logger][tid %thread][line %line]\n%msg\n");


	/// reconfigureLogger will create new logger if ir does not exists
	/// just simply add wahtever logger you want, best way is using class name as logger name
	el::Loggers::reconfigureLogger("default", defaultConf);
	el::Loggers::reconfigureLogger(JackieNetName, defaultConf);
	el::Loggers::reconfigureLogger("performance", defaultConf);

	// Clears everything because configurations uses heap so we want to retain it.
	// otherwise it is retained by internal memory management at the end of program
	// execution
	defaultConf.clear();


	JINFO << "Start Client..";

	JACKIE_INET::ServerApplication* app = JACKIE_INET::ServerApplication::GetInstance();
	IPlugin plugin;
	app->AttachOnePlugin(&plugin);

	JACKIE_INET::BindSocket socketDescriptor("", 0);
	app->Start(4, &socketDescriptor, 1);

	app->Connect("127.0.0.1", 38000);

	Packet* packet = 0;
	//// Loop for input
	while (1)
	{
		// This sleep keeps RakNet responsive
		for (packet = app->GetPacketOnce(); packet != 0;
			app->ReclaimPacket(packet), packet = 0)
		{
			/// user logics goes here
			//Command* c = app->AllocCommand();
			//c->command = Command::BCS_SEND;
			//app->ExecuteComand(c);

		}
	}

	app->StopRecvThread();
	app->StopNetworkUpdateThread();
	return 0;
}