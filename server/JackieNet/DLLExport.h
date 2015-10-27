/*
 *  Export.h
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 */
#ifndef DLL_EXPORT_H_
#define DLL_EXPORT_H_

//#include"DefaultNetDefines.h"

#if defined(_WIN32) && !(defined(__GNUC__)  || defined(__GCCXML__)) && !defined(JACIE_HAS_STATIC_LIB) && defined(JACIE_HAS_DYNAMIC_LIB)
#define JACKIE_EXPORT __declspec(dllexport)
#else
#define JACKIE_EXPORT  
#endif

#define STATIC_FACTORY_DECLARATIONS(x) \
static x* GetInstance(void); \
static void DestroyInstance( x *i);

#define STATIC_FACTORY_DEFINITIONS(x,y) \
x* x::GetInstance(void) {return JACKIE_INET::OP_NEW<y>( _FILE_AND_LINE_ );} \
void x::DestroyInstance( x *i) {JACKIE_INET::OP_DELETE(( y* ) i, _FILE_AND_LINE_);}

#ifndef DO_NOT_USE_EASY_LOGGING
#define JackieNetName  "JackieNet"
#define JDEBUG CLOG(DEBUG, JackieNetName)
#define JINFO CLOG(INFO, JackieNetName)
#define JWARNING CLOG(WARNING, JackieNetName)
#define JERROR CLOG(ERROR, JackieNetName)
#define JFATAL CLOG(FATAL, JackieNetName)
#define JTRACEENTER CLOG(TRACE, JackieNetName)
#define JTRACELEAVE CLOG(TRACE, JackieNetName)
#endif

#endif
