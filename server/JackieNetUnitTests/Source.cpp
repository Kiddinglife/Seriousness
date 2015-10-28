#include <iostream>

//#define _ELPP_STRICT_ROLLOUT
//#define ELPP_DISABLE_DEBUG_LOGS
#include "980easylogging++.h"
INITIALIZE_EASYLOGGINGPP

#include "JackieNet\WSAStartupSingleton.h"
//////////////////////////// GlobalFunctions_h starts ////////////////////////////////////////////
#include "JackieNet\GlobalFunctions.h"
static void test_superfastfunction_func()
{
	std::cout << "\nGlobalFunctions_h::test_superfastfunction_func() starts...\n";
	char* name = "jackie";
	std::cout << "name hash code = " << ( name, strlen(name) + 1, strlen(name) + 1 );
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
	std::cout << "size()= " << JACKIE_INET::JACKIE_INET_Address::size() << "\n";
}
static void test_ToHashCode_func()
{
	std::cout << "JACKIE_INET_Address::test_ToHashCode_func() starts...\n";
	JACKIE_INET::JACKIE_INET_Address addr;
	std::cout << "hash_code()= " << JACKIE_INET::JACKIE_INET_Address::ToHashCode(addr)
		<< "\n";
}
static void test_Ctor_ToString_FromString_funcs()
{
	printf_s("JACKIE_INET_Address::test_Ctor_ToString_FromString_funcs() starts...\n");

	JACKIE_INET::JACKIE_INET_Address default_ctor_addr;
	const char* str1 = default_ctor_addr.ToString();
	printf_s("default_ctor_addr_to_str = %s\n", str1);

	JACKIE_INET::JACKIE_INET_Address param_ctor_addr_localhost("localhost:12345");
	const char* str2 = param_ctor_addr_localhost.ToString();
	printf_s("param_ctor_addr_localhost = %s\n", str2);

	// THIS IS WRONG, so when you use domain name, you have to seprate two-params ctor
	//JACKIE_INET::JACKIE_INET_Address param_ctor_addr_domain("ZMD-SERVER:1234");

	JACKIE_INET::JACKIE_INET_Address param_ctor_addr_domain("ZMD-SERVER", 1234);
	const char* str3 = param_ctor_addr_domain.ToString();
	printf_s("param_ctor_addr_domain = %s\n", str3);
}
static void test_SetToLoopBack_func()
{
	printf_s("JACKIE_INET_Address::test_SetToLoopBack_func() starts...\n");

	JACKIE_INET::JACKIE_INET_Address addr("192.168.1.108", 12345);
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

	JACKIE_INET::JACKIE_INET_Address addr("LOCALHOST", 12345);
	const char* str = addr.ToString();
	addr.IsLoopback() ?
		printf_s("addr (%s) is loopback addr \n", str) :
		printf_s("addr (%s) is not loopback addr \n", str);

	JACKIE_INET::JACKIE_INET_Address addr1("192.168.1.103", 12345);
	const char* str1 = addr1.ToString();
	addr1.IsLoopback() ?
		printf_s("addr (%s) is loopback addr \n", str1) :
		printf_s("addr (%s) is not loopback addr \n", str1);
}
static void test_IsLANAddress_func()
{
	printf_s("JACKIE_INET_Address::test_IsLoopback_func() starts...\n");

	JACKIE_INET::JACKIE_INET_Address addr("localhost", 12345);
	const char* str = addr.ToString();
	addr.IsLoopback() ?
		printf_s("addr (%s) is LANA addr \n", str) :
		printf_s("addr (%s) is not LANA addr \n", str);

	JACKIE_INET::JACKIE_INET_Address addr1("www.baidu.com", 12345);
	const char* str1 = addr1.ToString();
	addr1.IsLoopback() ?
		printf_s("addr (%s) is LANA addr \n", str1) :
		printf_s("addr (%s) is not LANA addr \n", str1);
}

//////////////////////////// JACKIE_INet_GUID starts ////////////////////////////////////////////
static void test_JACKIE_INet_GUID_ToString_func()
{
	std::cout << "JACKIE_INet_GUID::test_ToString_func() starts...\n";

	printf_s("%s\n", JACKIE_INET::JACKIE_INet_GUID_Null.ToString());

	JACKIE_INET::JACKIE_INet_GUID gui(12);
	printf_s("%s\n", gui.ToString());

	printf_s("%d\n", JACKIE_INET::JACKIE_INet_GUID::ToUInt32(gui));
}

//////////////////////// JACKIE_INET_Address_GUID_Wrapper starts /////////////////////////
static void test_JACKIE_INET_Address_GUID_Wrapper_ToHashCodeString_func()
{
	std::cout << "JACKIE_INET_Address_GUID_Wrapper::test_ToHashCode_func() starts...\n";

	JACKIE_INET::JACKIE_INET_Address_GUID_Wrapper wrapper;
	printf_s("ToString(%s)\n", wrapper.ToString());
	printf_s("ToHashCode(%d)\n",
		JACKIE_INET::JACKIE_INET_Address_GUID_Wrapper::ToHashCode(wrapper));

	JACKIE_INET::JACKIE_INet_GUID gui(12);
	JACKIE_INET::JACKIE_INET_Address adrr("localhost", 123456);
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
	JackieTimeVal tv;
	// If you get an arror about an incomplete type, just delete this file
	JackieTimeZone tz;
	JackieGettimeofday(&tv, &tz);
	// time ( &rawtime );
	rawtime = tv.tv_sec;

	struct tm * timeinfo;
	timeinfo = localtime(&rawtime);
	strftime(buffer, 128, "%x %X", timeinfo);
	char buff[32];
	sprintf_s(buff, ":%i", tv.tv_usec / 1000);
	strcat_s(buffer, buff);

	printf_s("JackieGettimeofday(%s)\n", buffer);

	printf_s("GetTime(%i)\n", GetTime());
	JACKIE_Sleep(1000);
	printf_s("GetTime(%i)\n", GetTime());
}

///////////////////////////// JACKIE_INet_Socket_h /////////////////////////////
using namespace JACKIE_INET;
#include "JackieNet/JACKIE_INet_Socket.h"
static void test_GetMyIP_Wins_Linux_funcs()
{
	std::cout << "test_GetMyIP_Wins_Linux_funcs starts...\n";
	JACKIE_INET_Address addr[MAX_COUNT_LOCAL_IP_ADDR];
	JISBerkley::GetMyIPBerkley(addr);
	for( int i = 0; i < MAX_COUNT_LOCAL_IP_ADDR; i++ )
	{
		printf_s("(%s)\n", addr[i].ToString());
	}
}


class myhandler : public JISEventHandler
{
	public:
	virtual  void OnJISRecv(JISRecvParams *recvStruct) { }
	virtual  void DeallocJISRecvParams(JISRecvParams *s, const char *file, UInt32 line)
	{
	}
	virtual JISRecvParams *AllocJISRecvParams(const char *file, UInt32 line) { static JISRecvParams recv; return &recv; }
};
static void test_JISBerkley_All_funcs()
{
	std::cout << "test_JISBerkley_All_funcs starts...\n";

	JACKIE_INet_Socket* sock = JISAllocator::AllocJIS();
	sock->SetUserConnectionSocketIndex(0);

	JACKIE_INET_Address addresses[MAX_COUNT_LOCAL_IP_ADDR];
	sock->GetMyIP(addresses);
	for( int i = 0; i < MAX_COUNT_LOCAL_IP_ADDR; i++ )
	{
		if( addresses[i] == JACKIE_INET_Address_Null ) break;
		JACKIE_NET_DEBUG_PRINTF("my avaible IP (%s)\n", addresses[i].ToString());
	}

	if( sock->IsBerkleySocket() )
	{
		JISBerkley* bsock = ( (JISBerkley*) sock );

		myhandler handler;

		JISBerkleyBindParams bbp;
		bbp.port = 36005;
		bbp.hostAddress = "127.0.0.1";
		bbp.addressFamily = AF_INET;
		bbp.type = SOCK_DGRAM;
		bbp.protocol = 0;
		bbp.isBlocKing = false;
		bbp.isBroadcast = true;
		bbp.setIPHdrIncl = false;
		bbp.doNotFragment = false;
		bbp.pollingThreadPriority = 0;
		bbp.eventHandler = &handler;
		bbp.remotePortJackieNetWasStartedOn_PS3_PS4_PSP2 = 0;

		if( JISBerkley::IsPortInUse(bbp.port, bbp.hostAddress, bbp.addressFamily, bbp.type) )
		{
			printf_s("isportinuse(true)\n");
		} else
		{
			printf_s("isportinuse(false)\n");
		}

		JISBindResult br = bsock->Bind(&bbp, TRACE_FILE_AND_LINE_);
		printf_s("%s, bound addr (%s)\n", JISBindResultToString(br), bsock->GetBoundAddress().ToString());

		if( JISBerkley::IsPortInUse(bbp.port, bbp.hostAddress, bbp.addressFamily, bbp.type) )
		{
			printf_s("Isportinuse(true)\n");
		} else
		{
			printf_s("Isportinuse(false)\n");
		}

		switch( br )
		{
			case JISBindResult_FAILED_BIND_SOCKET:
				JISAllocator::DeallocJIS(sock);
				return;
				break;

			case JISBindResult_FAILED_SEND_RECV_TEST:
				JISAllocator::DeallocJIS(sock);
				return;
				break;

			default:
				JACKIE_ASSERT(br == JISBindResult_SUCCESS);
				break;
		}

		printf_s("Start CreateRecvPollingThread...\n");
		bsock->CreateRecvPollingThread(0);

		int ret;
		char* data = "JackieNet";
		JISSendParams sendParams;
		sendParams.data = data;
		sendParams.length = strlen(data) + 1;
		sendParams.receiverINetAddress = bsock->GetBoundAddress();
		do { ret = bsock->Send(&sendParams, TRACE_FILE_AND_LINE_); } while( ret < 0 );

		JISRecvParams* recvParams = handler.AllocJISRecvParams(TRACE_FILE_AND_LINE_);
		recvParams->socket = bsock;
		ret = bsock->RecvFrom(recvParams);
		if( ret >= 0 ) printf_s("recv(%s)\n", recvParams->data);
		printf_s("Start Polling Recv in another thread...\n");

		bsock->RecvFromLoop(bsock);

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
	for( int i = 0; i < 100000; i++ )
	{
		TestMemoryPool* test = memoryPool.Allocate();
		test->allocationId = i;
		printf_s("allocationId(%d)\n", test->allocationId);
		memoryPool.Release(test);
	}
}
//////////////////////////////////////////////////////////////////////////

/////////////////////// test_Queue_funcs ////////////////////////////
#include "JackieNet/RingBufferQueue.h"
#include "JackieNet/LockFreeQueue.h"
static void test_Queue_funcs()
{
	JINFO << "test_Queue_funcs STARTS...";
	DataStructures::RingBufferQueue<int> queue;

	TIMED_BLOCK(hello, "hello")
	{
		for( int i = 1; i < 100; i++ )
		{
			queue.PushTail(i, __FILE__, __LINE__);
		}
		queue.PushHead(12, 3, __FILE__, __LINE__);
		queue.Contains(12);
		queue.IsEmpty();
		queue.PopTail();
		queue.PopHead();
		queue.RemoveAtIndex(12);
		queue.Shrink2MiniSzie(__FILE__, __LINE__);
		queue.Resize(1000, __FILE__, __LINE__);
		queue.Clear(__FILE__, __LINE__);
	}

	DataStructures::LockFreeQueue lockfree(12);
	int a = 12;
	lockfree.PushTail((unsigned char*) &a, sizeof(a));
	//unsigned char b[8] = { 0 };
	UInt32 b;
	lockfree.PopHead((unsigned char*) &b, sizeof(int));
	JINFO << "b" << b;
}
//////////////////////////////////////////////////////////////////////////
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

static int testcase = CircularArrayQueueSingleThread;
static int testfunc = Test_Queue_funcs;


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
	defaultConf.setGlobally(el::ConfigurationType::Format, "[%level][%datetime][%logger] InFile[%fbase] AtLine[%line]\n%msg\n====================================================");

	// set individual option, @NOTICE you have to set this after setGlobally() to make it work
	//defaultConf.set(el::Level::Info, el::ConfigurationType::Format, "%datetime %level %msg");
	defaultConf.set(el::Level::Debug, el::ConfigurationType::Format, "[%level][%datetime][%logger]\nInFile[%fbase] Call[%func] AtLine[%line]\n%msg\n====================================================");
	defaultConf.set(el::Level::Trace, el::ConfigurationType::Format, "[%level][%datetime][%logger] InFile[%fbase] AtLine[%line]\n====================================================");


	/// reconfigureLogger will create new logger if ir does not exists
	/// just simply add wahtever logger you want, best way is using class name as logger name
	el::Loggers::reconfigureLogger("default", defaultConf);
	el::Loggers::reconfigureLogger(JackieNetName, defaultConf);

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
	switch( testcase )
	{
		case GlobalFunctions_h:
			switch( testfunc )
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
			switch( testfunc )
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
			switch( testfunc )
			{
				case JACKIE_INet_GUID_ToString_func:
					test_JACKIE_INet_GUID_ToString_func();
					break;
				default:
					test_JACKIE_INet_GUID_ToString_func();
					break;
			}
			break;

		case CLASS_JACKIE_INET_Address_GUID_Wrapper:
			switch( testfunc )
			{
				case JACKIE_INET_Address_GUID_Wrapper_ToHashCodeString_func:
					test_JACKIE_INET_Address_GUID_Wrapper_ToHashCodeString_func();
					break;
				default:
					test_JACKIE_INET_Address_GUID_Wrapper_ToHashCodeString_func();
					break;
			}
			break;

		case NetTime_h:
			switch( testfunc )
			{
				default:
					test_NetTime_h_All_funcs();
					break;
			}
			break;

		case JACKIE_INet_Socket_h:
			switch( testfunc )
			{
				case ClassJISBerkley:
					test_JISBerkley_All_funcs();
					break;
				default:
					test_JISBerkley_All_funcs();
					break;
			}
			break;
		case MemoryPool_h:
			switch( testfunc )
			{
				default:
					test_MemoryPool_funcs();
					break;
			}
			break;
		case CircularArrayQueueSingleThread:
			switch( testfunc )
			{
				case Test_Queue_funcs:
					test_Queue_funcs();
					break;
				default:
					test_Queue_funcs();
					break;
			}
			break;
		default:
			break;
	}
	return 0;
}
