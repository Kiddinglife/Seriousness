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

static void test_JACKIE_INet_GUID_ToString_func()
{
	std::cout << "JACKIE_INet_GUID::test_ToString_func() starts...\n";

	printf_s("%s\n", JACKIE_INET::JACKIE_NULL_GUID.ToString());

	JACKIE_INET::JackieGUID gui(12);
	printf_s("%s\n", gui.ToString());

	printf_s("%d\n", JACKIE_INET::JackieGUID::ToUInt32(gui));
}

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

using namespace JACKIE_INET;
#include "JackieNet/JackieINetSocket.h"
static void test_GetMyIP_Wins_Linux_funcs()
{
	std::cout << "test_GetMyIP_Wins_Linux_funcs starts...\n";
	JackieAddress addr[MAX_COUNT_LOCAL_IP_ADDR];
	JACKIE_INET::JISBerkley::GetMyIPBerkley(addr);
	for (int i = 0; i < MAX_COUNT_LOCAL_IP_ADDR; i++)
	{
		printf_s("(%s)\n", addr[i].ToString());
	}
}

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

#include "JackieNet/ArraryQueue.h"
#include "JackieNet/LockFreeQueue.h"
#include "JackieNet/EasyLog.h"
#include "JackieNet/Array.h"
#include "JackieNet/OrderArray.h"
#include "JackieNet/OrderListMap.h"
#include "JackieNet/DoubleLinkedList.h"

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
			//printf_s("%d, ", lockfree[i]);
		}
	}

	DataStructures::ArraryQueue<int, 100001> queuee;
	TIMED_BLOCK(RingBufferQueueTimer, "RingBufferQueueTimer")
	{
		for (int i = 0; i < 100000; i++)
		{
			queuee.Enqueue(i);
		}
		for (int i = 0; i < 100000; i++)
		{
			int t;
			queuee.Dequeue(t);
		}
	}

	DataStructures::Array<int, 100001> list;
	TIMED_BLOCK(ListTimer, "ListTimer")
	{
		for (int i = 0; i < 5000; i++)
		{
			list.PushAtLast(i);
		}

		list.InsertAtIndex(1, 6);
		list.RemoveAtIndex(24);
		list.ReplaceAtIndex(23, 0, 23);

		for (int i = 4; i < 100; i++)
		{
			list.RemoveAtIndex(i);
		}
		int t = list.PopAtLast();
	}

	DataStructures::OrderArray<int, int> olist;
	TIMED_BLOCK(olistTimer, "olist")
	{
		for (int i = 0; i < 10; i++)
		{
			olist.Insert(i, i, true);
		}

		olist.Insert(-3, -3, true);
		olist.RemoveAtIndex(4);

		for (int i = 0; i < 10; i++)
		{
			//JINFO << olist[i];
		}
	}

	DataStructures::OrderListMap<std::string, int> omap;
	TIMED_BLOCK(omapTimer, "omap")
	{
		omap.Set("set", 1);
		omap.SetExisting("set", 2);
		omap.SetNew("SetNew", 3);
		int setval = omap.Get("set");
		//JINFO << "set = " << setval;
		setval = omap.Get("SetNew");
		//JINFO << "set = " << setval;
		omap.Delete("set");
	}

	DataStructures::CircularList<int> linkedlist;
	TIMED_BLOCK(LinkedlisttIMER, "linkedlist")
	{
		linkedlist.Beginning();
		linkedlist.Add(4);
		linkedlist.Add(3);
		linkedlist.Insert(2);
		linkedlist.Add(1);
		linkedlist.Sort();

		for (size_t i = 0; i < linkedlist.Size(); i++)
		{
			printf("%d ", linkedlist.Peek());
			linkedlist++;
		}
		printf("\n");
		DCHECK(linkedlist.Has(3));
		DCHECK(linkedlist.Find(3));
		linkedlist.Remove();
		DCHECK(linkedlist.Has(3) == false);
		linkedlist.Find(4);
		linkedlist.Replace(-1);
		linkedlist.Beginning();
		for (size_t i = 0; i < linkedlist.Size(); i++)
		{
			printf("%d ", linkedlist.Peek());
			linkedlist++;
		}
		printf("\n");
	}

	DataStructures::ArrayCircularList<int, 1000> arrayCircularList;
	TIMED_BLOCK(ArrayCircularListTimer, "ArrayCircularListTimer")
	{
		int val;
		arrayCircularList.Add(2);
		arrayCircularList.Add(3);
		arrayCircularList.Add(8);

		arrayCircularList.Print();

		bool flag = arrayCircularList.Has(8);
		DCHECK(flag == true);
		flag = arrayCircularList.Has(13);
		DCHECK(flag == false);

		arrayCircularList.Remove(8);
		flag = arrayCircularList.Has(8);
		DCHECK(flag == false);

		val = arrayCircularList.Pop();
		DCHECK(val == 2);
		flag = arrayCircularList.Has(2);
		DCHECK(flag == false);

		arrayCircularList.Print();

		arrayCircularList.Add(2);
		arrayCircularList.Add(8);

		arrayCircularList.Print();

		flag = arrayCircularList.Has(8);
		DCHECK(flag == true);
		flag = arrayCircularList.Has(3);
		DCHECK(flag == true);
	}
}

#include "JackieNet/JackieBits.h"
static bool IncomeDatagramEventHandler(JISRecvParams *param)
{
	JINFO << "recv from  " << param->senderINetAddress.ToString() << ", bytes " << param->bytesRead;

	JackieBits jb((UInt8*)param->data, param->bytesRead, false);

	MessageID msgid;
	jb.ReadMini(msgid);
	JINFO << "ID_OPEN_CONNECTION_REQUEST_1 = " << (int)msgid;

	jb.ReadSkipBytes(16);

	jb.ReadMini(msgid);
	JINFO << "JACKIE_INET_PROTOCOL_VERSION = " << (int)msgid;

	return true;
}
#include "JackieNet/ServerApplication.h"
static void test_ServerApplication_funcs()
{
	JINFO << "test_ServerApplication_funcs STARTS...";

	JACKIE_INET::BindSocket socketDescriptor;
	socketDescriptor.blockingSocket = USE_BLOBKING_SOCKET; // USE_NON_BLOBKING_SOCKET; 
	socketDescriptor.port = 32000;
	socketDescriptor.socketFamily = AF_INET;

	JACKIE_INET::ServerApplication* app = JACKIE_INET::ServerApplication::GetInstance();
	app->incomeDatagramEventHandler = IncomeDatagramEventHandler;
	app->Start(4, &socketDescriptor, 1);

	app->Connect("localhost", 32000);
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

#include "JackieNet/JackieBits.h"
struct vec{ float x, y, z; };
static void test_JackieStream__funcs()
{
	TIMED_FUNC(test);

	JINFO << "Test alll write and read";
	JackieBits s8;

	UInt24 uint24 = 24;
	UInt8 uint8 = 8;
	UInt16 uint16 = 16;
	UInt32 uint32 = 32;
	UInt64 uint64 = 64;
	Int8 int8 = -8;
	Int16 int16 = -16;
	Int32 int32 = -32;
	Int64 int64 = -64;

	UInt8 particialByte = 0xf0; /// 11110000
	JackieGUID guid(123);
	JackieAddress addr("localhost", 32000);
	vec vector_ = { 0.2f, -0.4f, -0.8f };
	vec vector__ = { 2.234f, -4.78f, -32.2f };

	UInt32 curr = 12;
	UInt32 min = 8;
	UInt32 max = 64;

	s8.WriteIntegerRange(curr, min, max);
	curr = 0;
	s8.ReadIntegerRange(curr, min, max);
	DCHECK(curr == 12);

	s8.Write(uint24);
	uint24 = 0;
	s8.Read(uint24);
	DCHECK(uint24.val == 24);

	s8.WriteBits(&particialByte, 7, false);
	UInt8 v = 0;
	s8.ReadBits(&v, 7, false);
	DCHECK(particialByte == v);
	DCHECK(s8.GetPayLoadBits() == 0);

	for (int i = 100; i >= 0; i--)
	{
		UInt32 looptimes = 10000;
		for (UInt32 i = 1; i <= looptimes; i++)
		{
			s8.Write(uint24);

			s8 << guid;

			s8.WriteMini(uint24);

			s8 << addr;

			s8.WriteMini(uint24);

			s8.Write(uint8);
			s8.Write(int64);
			s8.WriteMini(uint8);
			s8.WriteMini(int64);

			s8.Write(uint16);
			s8.Write(int32);
			s8.WriteMini(uint16);
			s8.WriteMini(int32);

			s8.WriteBits(&particialByte, 4, true);
			s8.Write(uint24);
			s8.WriteNormVector(vector_.x, vector_.y, vector_.z);

			s8.WriteIntegerRange(curr, min, max);

			s8.WriteVector(vector__.x, vector__.y, vector__.z);

			s8.Write(uint32);
			s8.Write(int16);
			s8.WriteMini(uint32);
			s8.WriteMini(int16);

			s8.WriteBits(&particialByte, 4, false);

			s8.Write(uint64);
			s8.Write(int8);
			s8.WriteMini(uint64);
			s8.WriteMini(int8);

			s8.WriteBits(&particialByte, 7, false);
		}
		/*for (UInt32 i = 1; i <= looptimes; i++)
		{
		uint24 = 0;
		s8.Read(uint24);
		DCHECK(uint24.val == 24);

		JackieGUID guidd;
		s8 >> guidd;
		DCHECK(guid == guidd);

		JackieAddress addrr;
		s8 >> addrr;
		DCHECK(addr == addrr);

		UInt24 mini_uint24;
		s8.ReadMini(mini_uint24);
		DCHECK(mini_uint24.val == 24);

		s8.Read(uint8);
		s8.Read(int64);
		UInt8 mini_uint8;
		s8.ReadMini(mini_uint8);
		DCHECK(mini_uint8 == uint8);
		Int64 mini_int64;
		s8.ReadMini(mini_int64);
		DCHECK(mini_int64 == int64);

		s8.Read(uint16);
		s8.Read(int32);
		UInt16 mini_uint16;
		s8.ReadMini(mini_uint16);
		DCHECK(mini_uint16 == uint16);
		Int32 mini_int32;
		s8.ReadMini(mini_int32);
		DCHECK(mini_int32 == int32);


		UInt8 v = 0;
		s8.ReadBits(&v, 4, true);
		DCHECK(v == 0);

		vec vectorr;
		s8.ReadNormVector(vectorr.x, vectorr.y, vectorr.z);
		DCHECK(fabs(vectorr.x - vector_.x) <= 0.0001f);
		DCHECK(fabs(vectorr.y - vector_.y) <= 0.0001f);
		DCHECK(fabs(vectorr.y - vector_.y) <= 0.0001f);

		UInt32 v1;
		s8.ReadIntegerRange(v1, min, max);
		DCHECK(v1 == curr);

		vec vectorrr;
		s8.ReadVector(vectorrr.x, vectorrr.y, vectorrr.z);
		DCHECK(fabs(vectorrr.x - vector__.x) <= 0.001f);
		DCHECK(fabs(vectorrr.y - vector__.y) <= 0.001f);
		DCHECK(fabs(vectorrr.y - vector__.y) <= 0.001f);

		s8.Read(uint32);
		s8.Read(int16);
		UInt32 mini_uint32;
		s8.ReadMini(mini_uint32);
		DCHECK(mini_uint32 == uint32);
		Int16 mini_int16;
		s8.ReadMini(mini_int16);
		DCHECK(mini_int16 == int16);

		v = 0;
		s8.ReadBits(&v, 4, false);
		DCHECK(particialByte == ((v >> 4) << 4));

		s8.Read(uint64);
		s8.Read(int8);
		UInt64 mini_uint64;
		s8.ReadMini(mini_uint64);
		DCHECK(mini_uint64 == uint64);
		Int8 mini_int8;
		s8.ReadMini(mini_int8);
		DCHECK(mini_int8 == int8);

		v = 0;
		s8.ReadBits(&v, 7, false);
		DCHECK(particialByte == ((v >> 1) << 1));

		DCHECK(uint8 == 8);
		DCHECK(int8 == -8);
		DCHECK(uint16 == 16);
		DCHECK(int16 == -16);
		DCHECK(uint24.val == 24);
		DCHECK(uint32 == 32);
		DCHECK(int32 == -32);
		DCHECK(uint64 == 64);
		DCHECK(int64 == -64);
		}*/

		JackieBits s9;
		s9 << s8;

		for (UInt32 i = 1; i <= looptimes; i++)
		{
			uint24 = 0;
			s9.Read(uint24);
			DCHECK(uint24.val == 24);

			JackieGUID guidd;
			s9 >> guidd;
			DCHECK(guid == guidd);

			UInt24 mini_uint24 = 0;
			s9.ReadMini(mini_uint24);
			DCHECK(mini_uint24.val == 24);

			JackieAddress addrr;
			s9 >> addrr;
			DCHECK(addr == addrr);

			mini_uint24 = 0;
			s9.ReadMini(mini_uint24);
			DCHECK(mini_uint24.val == 24);

			s9.Read(uint8);
			s9.Read(int64);
			UInt8 mini_uint8;
			s9.ReadMini(mini_uint8);
			DCHECK(mini_uint8 == uint8);
			Int64 mini_int64;
			s9.ReadMini(mini_int64);
			DCHECK(mini_int64 == int64);

			s9.Read(uint16);
			s9.Read(int32);
			UInt16 mini_uint16;
			s9.ReadMini(mini_uint16);
			DCHECK(mini_uint16 == uint16);
			Int32 mini_int32;
			s9.ReadMini(mini_int32);
			DCHECK(mini_int32 == int32);


			UInt8 v = 0;
			s9.ReadBits(&v, 4, true);
			DCHECK(v == 0);

			uint24 = 0;
			s9.Read(uint24);
			DCHECK(uint24.val == 24);

			vec vectorr;
			s9.ReadNormVector(vectorr.x, vectorr.y, vectorr.z);
			DCHECK(fabs(vectorr.x - vector_.x) <= 0.0001f);
			DCHECK(fabs(vectorr.y - vector_.y) <= 0.0001f);
			DCHECK(fabs(vectorr.y - vector_.y) <= 0.0001f);

			UInt32 v1;
			s9.ReadIntegerRange(v1, min, max);
			DCHECK(v1 == curr);

			vec vectorrr;
			s9.ReadVector(vectorrr.x, vectorrr.y, vectorrr.z);
			DCHECK(fabs(vectorrr.x - vector__.x) <= 0.001f);
			DCHECK(fabs(vectorrr.y - vector__.y) <= 0.001f);
			DCHECK(fabs(vectorrr.y - vector__.y) <= 0.001f);

			s9.Read(uint32);
			s9.Read(int16);
			UInt32 mini_uint32;
			s9.ReadMini(mini_uint32);
			DCHECK(mini_uint32 == uint32);
			Int16 mini_int16;
			s9.ReadMini(mini_int16);
			DCHECK(mini_int16 == int16);

			v = 0;
			s9.ReadBits(&v, 4, false);
			DCHECK(particialByte == ((v >> 4) << 4));

			s9.Read(uint64);
			s9.Read(int8);
			UInt64 mini_uint64;
			s9.ReadMini(mini_uint64);
			DCHECK(mini_uint64 == uint64);
			Int8 mini_int8;
			s9.ReadMini(mini_int8);
			DCHECK(mini_int8 == int8);

			v = 0;
			s9.ReadBits(&v, 7, false);
			DCHECK(particialByte == ((v >> 1) << 1));

			DCHECK(uint8 == 8);
			DCHECK(int8 == -8);
			DCHECK(uint16 == 16);
			DCHECK(int16 == -16);
			DCHECK(uint24.val == 24);
			DCHECK(uint32 == 32);
			DCHECK(int32 == -32);
			DCHECK(uint64 == 64);
			DCHECK(int64 == -64);
		}

		s8.Reset();
		s9.Reset();
	}
}

enum
{
	GlobalFunctions_h,
	superfastfunction_func,
	isDomainIPAddr_func,
	DomainNameToIP_func,
	Itoa_func,

	JACKIE_INET_Address_h,
	size_func,
	ToHashCode_func,
	Ctor_ToString_FromString_funcs,
	SetToLoopBack_func,
	IsLoopback_func,
	IsLANAddress_func,

	JACKIE_INet_GUID_h,
	JACKIE_INet_GUID_ToString_func,

	CLASS_JACKIE_INET_Address_GUID_Wrapper,
	JACKIE_INET_Address_GUID_Wrapper_ToHashCodeString_func,

	NetTime_h,

	ClassJISAllocator,

	JACKIE_INet_Socket_h,
	ClassJISBerkley,

	MemoryPool_h,

	CircularArrayQueueSingleThread,
	Test_Queue_funcs,

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

static int testcase = CircularArrayQueueSingleThread;
static int testfunc = Test_Queue_funcs;

//static int testcase = ServerApplication_H;
//static int testfunc = AllFuncs;

//static int testcase = JackieStream_H;
//static int testfunc = AllFuncs;

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
