#include <iostream>
//#define _ELPP_STRICT_ROLLOUT
//#define ELPP_DISABLE_DEBUG_LOGS
#define ELPP_THREAD_SAFE 
//#define ELPP_FORCE_USE_STD_THREAD
//#define ELPP_DISABLE_INFO_LOGS
#include "980easylogging++.h"
INITIALIZE_EASYLOGGINGPP

#include "JackieNet\WSAStartupSingleton.h"
#include "JackieNet\NetTypes.h"
#include "JackieNet\GlobalFunctions.h"

using namespace JACKIE_INET;

//////////////////////////// GlobalFunctions_h starts ////////////////////////////////////////////
static void test_superfastfunction_func()
{
	std::cout << "\nGlobalFunctions_h::test_superfastfunction_func() starts...\n";
	char* name = "jackie";
	std::cout << "name hash code = " << (name, strlen(name) + 1, strlen(name) + 1);
}
static void test_isDomainIPAddr_func()
{
	std::cout << "\nGlobalFunctions_h::test_isDomainIPAddr_func() starts...\n";
	const char* host_domain = "www.baidu.com";
	const char* host_Num = "192.168.1.168";
	std::cout << "host_domain = www.baidu.com " << isDomainIPAddr(host_domain);
	std::cout << "\nhost_domain = 192.168.1.168 " << isDomainIPAddr(host_Num) << "\n";
}
static void test_itoa_func()
{
	std::cout << "\nGlobalFunctions_h::test_itoa_func() starts...\n";
	char result[3];
	std::cout << "\nvalue(int 12)= " << Itoa(12, result, 10) << "\n";
}

static void test_DomainNameToIP_func()
{
	// needed for getaddrinfo
	std::cout << "\nGlobalFunctions_h::test_DomainNameToIP_func() starts...\n";
	char ip[65] = { 0 };

	DomainNameToIP("", ip);
	std::cout << "\nvalue("")=ip " << ip << "\n"; //=lan ip address192.168.1.107

	DomainNameToIP("localhost", ip);
	std::cout << "\nvalue(localhost)=ip " << ip << "\n"; //=loopback ip addr 127.0.0.1

	DomainNameToIP("192.168.1.108", ip);
	std::cout << "\nvalue(localhost)=ip " << ip << "\n"; // = return 192.168.1.108

	DomainNameToIP("www.baidu.com", ip);
	std::cout << "\nvalue(www.baidu.com)=ip " << ip << "\n"; // = return 61.135.169.121

	DomainNameToIP("ZMD-SERVER", ip);
	std::cout << "\nvalue(ZMD-SERVER)=ip " << ip << "\n"; // = return 192.168.1.107
}

//////////////////////////// JACKIE_INET_Address_h starts ////////////////////////////////////////////
#include "JackieNet\NetTypes.h"
static void test_size_func()
{
	std::cout << "JACKIE_INET_Address::test_size_func() starts...\n";
	std::cout << "size()= " << JACKIE_INET::JackieAddress::size() << "\n";
}
static void test_ToHashCode_func()
{
	std::cout << "JACKIE_INET_Address::test_ToHashCode_func() starts...\n";
	JACKIE_INET::JackieAddress addr;
	std::cout << "hash_code()= " << JACKIE_INET::JackieAddress::ToHashCode(addr)
		<< "\n";
}
static void test_Ctor_ToString_FromString_funcs()
{
	printf_s("JACKIE_INET_Address::test_Ctor_ToString_FromString_funcs() starts...\n");

	JACKIE_INET::JackieAddress default_ctor_addr;
	const char* str1 = default_ctor_addr.ToString();
	printf_s("default_ctor_addr_to_str = %s\n", str1);

	JACKIE_INET::JackieAddress param_ctor_addr_localhost("localhost:12345");
	const char* str2 = param_ctor_addr_localhost.ToString();
	printf_s("param_ctor_addr_localhost = %s\n", str2);

	// THIS IS WRONG, so when you use domain name, you have to seprate two-params ctor
	//JACKIE_INET::JACKIE_INET_Address param_ctor_addr_domain("ZMD-SERVER:1234");

	JACKIE_INET::JackieAddress param_ctor_addr_domain("ZMD-SERVER", 1234);
	const char* str3 = param_ctor_addr_domain.ToString();
	printf_s("param_ctor_addr_domain = %s\n", str3);
}
static void test_SetToLoopBack_func()
{
	printf_s("JACKIE_INET_Address::test_SetToLoopBack_func() starts...\n");

	JACKIE_INET::JackieAddress addr("192.168.1.108", 12345);
	const char* str = addr.ToString();
	printf_s("addr = %s\n", str);

	addr.SetToLoopBack(4);
	str = addr.ToString();
	printf_s("After SetToLoopBack(4), addr = %s\n", str);

	/// if you do not define ipv6, this will use ipv4
	addr.SetToLoopBack(6);
	str = addr.ToString();
	printf_s("After SetToLoopBack(6), addr = %s\n", str);
}
static void test_IsLoopback_func()
{
	printf_s("JACKIE_INET_Address::test_IsLoopback_func() starts...\n");

	JACKIE_INET::JackieAddress addr("LOCALHOST", 12345);
	const char* str = addr.ToString();
	addr.IsLoopback() ?
		printf_s("addr (%s) is loopback addr \n", str) :
		printf_s("addr (%s) is not loopback addr \n", str);

	JACKIE_INET::JackieAddress addr1("192.168.1.103", 12345);
	const char* str1 = addr1.ToString();
	addr1.IsLoopback() ?
		printf_s("addr (%s) is loopback addr \n", str1) :
		printf_s("addr (%s) is not loopback addr \n", str1);
}
static void test_IsLANAddress_func()
{
	printf_s("JACKIE_INET_Address::test_IsLoopback_func() starts...\n");

	JACKIE_INET::JackieAddress addr("localhost", 12345);
	const char* str = addr.ToString();
	addr.IsLoopback() ?
		printf_s("addr (%s) is LANA addr \n", str) :
		printf_s("addr (%s) is not LANA addr \n", str);

	JACKIE_INET::JackieAddress addr1("www.baidu.com", 12345);
	const char* str1 = addr1.ToString();
	addr1.IsLoopback() ?
		printf_s("addr (%s) is LANA addr \n", str1) :
		printf_s("addr (%s) is not LANA addr \n", str1);
}

//////////////////////////// JACKIE_INet_GUID starts ////////////////////////////////////////////
static void test_JACKIE_INet_GUID_ToString_func()
{
	std::cout << "JACKIE_INet_GUID::test_ToString_func() starts...\n";

	printf_s("%s\n", JACKIE_INET::JACKIE_NULL_GUID.ToString());

	JACKIE_INET::JackieGUID gui(12);
	printf_s("%s\n", gui.ToString());

	printf_s("%d\n", JACKIE_INET::JackieGUID::ToUInt32(gui));
}

//////////////////////// JACKIE_INET_Address_GUID_Wrapper starts /////////////////////////
static void test_JACKIE_INET_Address_GUID_Wrapper_ToHashCodeString_func()
{
	std::cout << "JACKIE_INET_Address_GUID_Wrapper::test_ToHashCode_func() starts...\n";

	JACKIE_INET::JACKIE_INET_Address_GUID_Wrapper wrapper;
	printf_s("ToString(%s)\n", wrapper.ToString());
	printf_s("ToHashCode(%d)\n",
		JACKIE_INET::JACKIE_INET_Address_GUID_Wrapper::ToHashCode(wrapper));

	JACKIE_INET::JackieGUID gui(12);
	JACKIE_INET::JackieAddress adrr("localhost", (UInt16)123456);
	JACKIE_INET::JACKIE_INET_Address_GUID_Wrapper wrapper1(gui);
	printf_s("ToString(%s)\n", wrapper1.ToString());
	printf_s("ToHashCode(%d)\n",
		JACKIE_INET::JACKIE_INET_Address_GUID_Wrapper::ToHashCode(wrapper1));
}

//////////////////////////////////////////////////////////////////////////
#include "JackieNet/NetTime.h"
static void test_NetTime_h_All_funcs()
{
	std::cout << "Test_NetTime_h_All_funcs starts...\n";

	char buffer[128];
	time_t rawtime;
	struct timeval tv;
	// If you get an arror about an incomplete type, just delete this file
	struct timezone tz;
	gettimeofday(&tv, &tz);
	// time ( &rawtime );
	rawtime = tv.tv_sec;

	struct tm * timeinfo;
	timeinfo = localtime(&rawtime);
	strftime(buffer, 128, "%x %X", timeinfo);
	char buff[32];
	sprintf_s(buff, ":%i", tv.tv_usec / 1000);
	strcat_s(buffer, buff);

	printf_s("JackieGettimeofday(%s)\n", buffer);

	printf_s("GetTime(%i)\n", GetTimeMS());
	JackieSleep(1000);
	printf_s("GetTime(%i)\n", GetTimeMS());
}

///////////////////////////// JACKIE_INet_Socket_h /////////////////////////////
using namespace JACKIE_INET;
#include "JackieNet/JackieINetSocket.h"
static void test_GetMyIP_Wins_Linux_funcs()
{
	std::cout << "test_GetMyIP_Wins_Linux_funcs starts...\n";
	JackieAddress addr[MAX_COUNT_LOCAL_IP_ADDR];
	JISBerkley::GetMyIPBerkley(addr);
	for (int i = 0; i < MAX_COUNT_LOCAL_IP_ADDR; i++)
	{
		printf_s("(%s)\n", addr[i].ToString());
	}
}

//////////////////////////////////////////////////////////////////////////

////////////////////////// test_MemoryPool_funcs ///////////////////////////
#include "JackieNet/MemoryPool.h"
struct TestMemoryPool
{
	int allocationId;
};
static void test_MemoryPool_funcs()
{
	std::cout << "test_MemoryPool_funcs starts...\n";
	DataStructures::MemoryPool<TestMemoryPool> memoryPool;
	for (int i = 0; i < 100000; i++)
	{
		TestMemoryPool* test = memoryPool.Allocate();
		test->allocationId = i;
		//printf_s("allocationId(%d)\n", test->allocationId);
		memoryPool.Reclaim(test);
	}
}
//////////////////////////////////////////////////////////////////////////

/////////////////////// test_Queue_funcs ////////////////////////////
#include "JackieNet/ArraryQueue.h"
#include "JackieNet/LockFreeQueue.h"
#include "JackieNet/EasyLog.h"

JACKIE_THREAD_DECLARATION(lockfreeproducer)
{
	TIMED_FUNC();
	for (int i = 0; i < 10; i++)
	{
		((DataStructures::LockFreeQueue<int, 4 * 100000>*)arguments)->PushTail(i);
	}
	return 0;
}

JACKIE_THREAD_DECLARATION(lockfreeconsumer)
{

	for (int i = 0; i < ((DataStructures::LockFreeQueue<int, 4 * 100000>*)arguments)->Size()
		; i++)
	{
		int t;
		((DataStructures::LockFreeQueue<int, 4 * 100000>*)arguments)->PopHead(t);
	}
	return 0;
}

static void test_Queue_funcs()
{
	JINFO << "test_Queue_funcs STARTS...";

	DataStructures::LockFreeQueue<int, 4 * 100000> lockfree;
	TIMED_BLOCK(LockFreeQueueTimer, "LockFreeQueue")
	{
		for (int i = 0; i < 10; i++)
		{
			lockfree.PushTail(i);
		}
		for (int i = 0; i < 10; i++)
		{
			int t;
			lockfree.PopHead(t);
		}
		for (unsigned int i = 0; i < lockfree.Size(); i++)
		{
			printf_s("%d, ", lockfree[i]);
		}
	}

	DataStructures::ArraryQueue<int, 100001> queuee;
	TIMED_BLOCK(RingBufferQueueTimer, "RingBufferQueueTimer")
	{
		for (int i = 0; i < 100000; i++)
		{
			queuee.PushTail(i);
		}
		for (int i = 0; i < 100000; i++)
		{
			int t;
			queuee.PopHead(t);
		}
	}
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
#include "JackieNet/ServerApplication.h"
static void test_ServerApplication_funcs()
{
	JINFO << "test_ServerApplication_funcs STARTS...";

	JACKIE_INET::BindSocket socketDescriptor;
	socketDescriptor.blockingSocket = USE_NON_BLOBKING_SOCKET; // USE_NON_BLOBKING_SOCKET; //USE_BLOBKING_SOCKET
	socketDescriptor.port = 32000;
	socketDescriptor.socketFamily = AF_INET;

	JACKIE_INET::ServerApplication* app = JACKIE_INET::ServerApplication::GetInstance();
	app->Start(4, &socketDescriptor, 1);

	app->Connect("127.0.0.1", 32000);
	//int ret;
	//char* data = "JackieNet";
	//JISSendParams sendParams;
	//sendParams.data = data;
	//sendParams.length = strlen(data) + 1;
	//sendParams.receiverINetAddress = app->bindedSockets[0]->GetBoundAddress();
	//do { ret = ( ( JACKIE_INET::JISBerkley* )app->bindedSockets[0] )->Send(&sendParams, TRACE_FILE_AND_LINE_); } while( ret < 0 );



	Packet* packet = 0;
	//// Loop for input
	while (1)
	{

		//Command* c = app->AllocCommand();
		//c->command = Command::BCS_SEND;
		//app->PostComand(c);

		// This sleep keeps RakNet responsive
		for (packet = app->GetPacketOnce(); packet != 0;
			app->ReclaimPacket(packet), packet = 0)
		{

			/// user logics goes here
			//Command* c = app->AllocCommand();
			//c->command = Command::BCS_SEND;
			//app->ExecuteComand(c);

		}

		/// another way to use
		//packet = app->GetPacketOnce();
		//Command* c = app->AllocCommand();
		//c->command = Command::BCS_SEND;
		//app->ExecuteComand(c);
		//if( packet != 0 ) app->ReclaimOnePacket(packet);

		JackieSleep(1500);
		//break;
	}

	Sleep(1001);
	app->StopRecvThread();
	Sleep(1001);
	app->StopNetworkUpdateThread();
	Sleep(1001);
}
//////////////////////////////////////////////////////////////////////////

#include "JackieNet/JackieBits.h"
static void test_JackieStream__funcs()
{
	TIMED_FUNC(test);
	//for( unsigned int Index = 0; Index < 10000; Index++ )
	//{
	//	char strr[ ] = "1";
	//	JackieBits s5((UInt8*) strr, sizeof(strr), false);
	//	JackieBits s4;
	//	s4.WriteFrom(s5);
	//	s4.Reuse();
	//}
	JINFO << "Test s6.WriteMiniFrom()";
	short strr = -5;
	JackieBits s5((UInt8*)&strr, sizeof(strr), false);
	s5.PrintBit();

	JackieBits s6;
	s6.WriteMini((UInt8*)&strr, sizeof(strr) * 8, false);
	s6.PrintBit();

	JINFO << "Test NumberOfLeadingZeroes() " << JackieBits::GetLeadingZeroSize(1);

	JINFO << "Test ReverseBytes() ";
	Int8 str[256];
	int a = 1;
	JackieBits::PrintBit(str, 32, (UInt8*)&a);
	JINFO << str;
	UInt8 b[4];
	JackieBits::ReverseBytes((UInt8*)&a, b, 4);
	JackieBits::PrintBit(str, 32, b);
	JINFO << str;

	JINFO << "Test s7.ReadBits()";
	JackieBits s7((UInt8*)&strr, sizeof(strr), true);
	UInt8 readto[256] = { 0 };
	s7.ReadPosBits(3);
	s7.ReadBits(readto, 13, false);
	s5.PrintBit();
	JackieBits::PrintBit(str, 16, readto);
	JINFO << str;

	int reverse = 123;
	JackieBits::PrintBit(str, 32, (UInt8*)&reverse);
	JINFO << str;
	int dest;
	JackieBits::ReverseBytes((UInt8*)&reverse, (UInt8*)&dest, 4);
	JackieBits::PrintBit(str, 32, (UInt8*)&dest);
	JINFO << str;

	int as = 12;
	s7 << as << reverse;

	JINFO << "Test s8 alll write and read";
	JackieBits s8;
	UInt32 v1 = 128;
	s8.Serialize(true, v1);
	UInt32 v2 = 0;
	s8.Serialize(false, v2);
	JINFO << "v2 " << v2;


	Int8 u1 = -25;
	s8.Write(12);
	s8.WriteMini(u1);
	s8.Read(as);
	s8.PadZeroAfterAlignedWRPos(123);
	Int8 u2;
	s8.ReadMini(u2);
	JINFO << "u2 " << (int)u2 << "a  " << as;
}
enum
{
	//////////////////////////////////////////////////////////////////////////
	GlobalFunctions_h,
	superfastfunction_func,
	isDomainIPAddr_func,
	DomainNameToIP_func,
	Itoa_func,
	//////////////////////////////////////////////////////////////////////////
	JACKIE_INET_Address_h,
	size_func,
	ToHashCode_func,
	Ctor_ToString_FromString_funcs,
	SetToLoopBack_func,
	IsLoopback_func,
	IsLANAddress_func,
	//////////////////////////////////////////////////////////////////////////
	JACKIE_INet_GUID_h,
	JACKIE_INet_GUID_ToString_func,
	//////////////////////////////////////////////////////////////////////////
	CLASS_JACKIE_INET_Address_GUID_Wrapper,
	JACKIE_INET_Address_GUID_Wrapper_ToHashCodeString_func,
	//////////////////////////////////////////////////////////////////////////
	NetTime_h,
	//////////////////////////////////////////////////////////////////////////
	ClassJISAllocator,
	//////////////////////////////////////////////////////////////////////////
	JACKIE_INet_Socket_h,
	ClassJISBerkley,
	//////////////////////////////////////////////////////////////////////////
	MemoryPool_h,
	//////////////////////////////////////////////////////////////////////////
	CircularArrayQueueSingleThread,
	Test_Queue_funcs,
	//////////////////////////////////////////////////////////////////////////
	ServerApplication_H,
	JackieStream_H,
	Start,
	AllClass,
	AllFuncs
};


//static int testcase = GlobalFunctions_h;
//static int testfunc = AllFuncs;

//static int testcase = JACKIE_INET_Address_h;
//static int testfunc = AllFuncs;

//static int testcase = JACKIE_INet_GUID_h;
//static int testfunc = AllFuncs;

//static int testcase = JACKIE_INET_Address_GUID_Wrapper;
//static int testfunc = AllFuncs;

//static int testcase = NetTime_h;
//static int testfunc = AllFuncs;

//static int testcase = JACKIE_INet_Socket_h;
//static int testfunc = ClassJISBerkley;

//static int testcase = MemoryPool_h;
//static int testfunc = AllFuncs;
//
//
//static int testcase = CircularArrayQueueSingleThread;
//static int testfunc = Test_Queue_funcs;
//
//static int testcase = ServerApplication_H;
//static int testfunc = AllFuncs;

static int testcase = JackieStream_H;
static int testfunc = AllFuncs;


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

	//LOG(INFO) << "Log using default file";
	//LOG(ERROR) << "Log using default file";
	//LOG(WARNING) << "Log using default file";
	////LOG(FATAL) << "Log using default file";
	//LOG(DEBUG) << "Log using default file";
	//JDEBUG << "test debug";

	START_EASYLOGGINGPP(argc, argv);
	switch (testcase)
	{
	case GlobalFunctions_h:
		switch (testfunc)
		{
		case superfastfunction_func:
			test_superfastfunction_func();
			break;
		case isDomainIPAddr_func:
			test_isDomainIPAddr_func();
			break;
		case Itoa_func:
			test_itoa_func();
			break;
		case DomainNameToIP_func:
			test_DomainNameToIP_func();
			break;
		default:
			test_superfastfunction_func();
			test_isDomainIPAddr_func();
			test_itoa_func();
			test_DomainNameToIP_func();
			break;
		}
		break;
	case JACKIE_INET_Address_h:
		switch (testfunc)
		{
		case size_func:
			test_size_func();
			break;
		case ToHashCode_func:
			test_ToHashCode_func();
			break;
		case Ctor_ToString_FromString_funcs:
			test_Ctor_ToString_FromString_funcs();
			break;
		case SetToLoopBack_func:
			test_SetToLoopBack_func();
			break;
		case IsLoopback_func:
			test_IsLoopback_func();
			break;
		case IsLANAddress_func:
			test_IsLANAddress_func();
			break;
		default:
			test_size_func();
			test_ToHashCode_func();
			test_Ctor_ToString_FromString_funcs();
			test_SetToLoopBack_func();
			test_IsLoopback_func();
			test_IsLANAddress_func();
			break;
		}
		break;
	case JACKIE_INet_GUID_h:
		test_JACKIE_INet_GUID_ToString_func();
		break;
	case CLASS_JACKIE_INET_Address_GUID_Wrapper:
		test_JACKIE_INET_Address_GUID_Wrapper_ToHashCodeString_func();
		break;
	case NetTime_h:
		test_NetTime_h_All_funcs();
		break;
	case MemoryPool_h:
		test_MemoryPool_funcs();
		break;
	case CircularArrayQueueSingleThread:
		test_Queue_funcs();
		break;
	case ServerApplication_H:
		test_ServerApplication_funcs();
		break;
	case JackieStream_H:
		test_JackieStream__funcs();
		break;
	default:
		break;
	}
	return 0;
}
