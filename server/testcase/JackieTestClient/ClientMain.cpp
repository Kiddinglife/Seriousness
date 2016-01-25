#include <iostream>
//#define _ELPP_STRICT_ROLLOUT
//#define ELPP_DISABLE_DEBUG_LOGS
#define ELPP_THREAD_SAFE 
//#define ELPP_FORCE_USE_STD_THREAD
//#define ELPP_DISABLE_INFO_LOGS
#include "EasyLog.h"
INITIALIZE_EASYLOGGINGPP

#include "JackieIPlugin.h"
#include "JackieApplication.h"

#if ENABLE_SECURE_HAND_SHAKE == 1
#include "SecurityHandShake.h"
#endif

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
	el::Loggers::reconfigureLogger("RemoteLogger", defaultConf);
	el::Helpers::installLogDispatchCallback<RemoteLogger>("RemoteLogger");

	// Clears everything because configurations uses heap so we want to retain it.
	// otherwise it is retained by internal memory management at the end of program
	// execution
	defaultConf.clear();

	RINF << "Starting Client.." << (UInt32)-1;

	JACKIE_INET::JackieApplication* client = JACKIE_INET::JackieApplication::GetInstance();
	JackieIPlugin plugin;
	client->SetPlugin(&plugin);

	// default use wild address and random port and blobking mode
	JACKIE_INET::JackieBindingSocket socketDescriptor;
	if (client->Start(&socketDescriptor) == StartupResult::START_SUCCEED)
	{

#if ENABLE_SECURE_HAND_SHAKE == 1
		{
			char serverPublicKey[cat::EasyHandshake::PUBLIC_KEY_BYTES];
			FILE *fp = fopen("..\\publickey.pk", "rb");
			fread(serverPublicKey, sizeof(serverPublicKey), 1, fp);
			fclose(fp);
			JACKIE_INET::JackieSHSKey shsKeys;
			shsKeys.remoteServerPublicKey = serverPublicKey;
			shsKeys.publicKeyMode = SecureConnectionMode::USE_KNOWN_PUBLIC_KEY;
			char uname[] = "admin";
			ConnectionAttemptResult connectResult = client->Connect("127.0.0.1", 38000, uname, sizeof(uname), &shsKeys);
			//ConnectionAttemptResult connectResult = client->Connect("127.0.0.1", 38000, uname, sizeof(uname));
			assert(connectResult == ConnectionAttemptResult::CONNECTION_ATTEMPT_POSTED);
		}
#else
		assert(client->Connect("127.0.0.1", 38000, "root", strlen("root")) == ConnectionAttemptResult::CONNECTION_ATTEMPT_POSTED);
#endif

		JINFO << "\nMy IP addresses:";
		unsigned int i;
		for (i = 0; i < client->GetLocalIPAddrCount(); i++)
		{
			JINFO << i + 1 << ". " << client->GetLocalIPAddr(i);
		}

		JINFO << " My GUID " << client->GetGuidFromSystemAddress(JACKIE_NULL_ADDRESS).g;

		JackiePacket* packet = 0;

		//// Loop for input
		while (1)
		{
			for (packet = client->GetPacketOnce(); packet != 0;
				client->ReclaimPacket(packet), packet = client->GetPacketOnce())
			{
				/// user logics goes here
				//Command* c = app->AllocCommand();
				//c->command = Command::BCS_SEND;
				//app->ExecuteComand(c);

			}
		}

		client->StopRecvThread();
		client->StopNetworkUpdateThread();
		JackieApplication::DestroyInstance(client);
	}
	else
	{
	}
	return 0;
}