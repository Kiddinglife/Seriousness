#ifndef easylogging980_H_
#define easylogging980_H_

#include "980easylogging++.h"
#include "DLLExport.h"
#include <assert.h>

#define JackieNetName  "JackieNet"
#define JDEBUG CLOG(DEBUG, JackieNetName)
#define JINFO CLOG(INFO, JackieNetName)
#define JWARNING CLOG(WARNING, JackieNetName)
#define JERROR CLOG(ERROR, JackieNetName)
#define JFATAL CLOG(FATAL, JackieNetName)
#define JTRACEENTER CLOG(TRACE, JackieNetName)
#define JTRACELEAVE CLOG(TRACE, JackieNetName)

#define RINF CLOG(INFO, "RemoteLogger")
#define RDBG CLOG(DEBUG, "RemoteLogger")
#define RERR CLOG(ERROR, "RemoteLogger")
#define RWAR CLOG(WARNING, "RemoteLogger")
#define RFATAL CLOG(FATAL, "RemoteLogger")

namespace JACKIE_INET {

	class  JackieApplication;
	class  JACKIE_EXPORT RemoteLogger : public el::LogDispatchCallback
	{
	private:
		JackieApplication* japp;

	public:
		RemoteLogger()
		{
			japp = 0;
			el::Loggers::getLogger("RemoteLogger"); // register
		}
		void handle(const el::LogDispatchData* data);

		/// cb when recvived ID_CONNECTION_REQUEST_ACCEPTED in main loop
		void OnActive(JACKIE_INET::JackieApplication* app)
		{
			japp = app;
			assert(japp != 0);
		}
	};
}
#endif