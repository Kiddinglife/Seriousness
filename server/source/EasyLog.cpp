#include "EasyLog.h"
#include "JackieApplication.h"
using namespace JACKIE_INET;

void RemoteLogger::handle(const el::LogDispatchData* data)
{
	{
		// NEVER DO LOG FROM HANDLER!
		if (japp != 0)
		{
			Command* cmd = japp->AllocCommand();
			cmd->command = Command::BCS_SEND;
			//@to-do fillup cmd with msg data
			japp->PostComand(cmd);
		}
	}
}
