#ifndef easylogging980_H_
#define easylogging980_H_

#include "980easylogging++.h"
#define JackieNetName  "JackieNet"
#define JDEBUG CLOG(DEBUG, JackieNetName)
#define JINFO CLOG(INFO, JackieNetName)
#define JWARNING CLOG(WARNING, JackieNetName)
#define JERROR CLOG(ERROR, JackieNetName)
#define JFATAL CLOG(FATAL, JackieNetName)
#define JTRACEENTER CLOG(TRACE, JackieNetName)
#define JTRACELEAVE CLOG(TRACE, JackieNetName)

#endif