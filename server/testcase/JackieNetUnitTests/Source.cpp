#include <iostream>
#define _ELPP_STRICT_ROLLOUT
//#define ELPP_DISABLE_DEBUG_LOGS
#define ELPP_THREAD_SAFE 
#define ELPP_FORCE_USE_STD_THREAD
//#define ELPP_DISABLE_INFO_LOGS
#include "980easylogging++.h"
INITIALIZE_EASYLOGGINGPP

#include "EasyLog.h"
#include "WSAStartupSingleton.h"
#include "NetTypes.h"
#include "GlobalFunctions.h"
#include "JackieIPlugin.h"

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

#include "NetTypes.h"
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
	std::cout << "JackieAddressGuidWrapper::test_ToHashCode_func() starts...\n";

	JACKIE_INET::JackieAddressGuidWrapper wrapper;
	printf_s("ToString(%s)\n", wrapper.ToString());
	printf_s("ToHashCode(%d)\n",
		JACKIE_INET::JackieAddressGuidWrapper::ToHashCode(wrapper));

	JACKIE_INET::JackieGUID gui(12);
	JACKIE_INET::JackieAddress adrr("localhost", (UInt16)123456);
	JACKIE_INET::JackieAddressGuidWrapper wrapper1(gui);
	printf_s("ToString(%s)\n", wrapper1.ToString());
	printf_s("ToHashCode(%d)\n",
		JACKIE_INET::JackieAddressGuidWrapper::ToHashCode(wrapper1));
}

#include "NetTime.h"
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
#include "JackieINetSocket.h"
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

#include "JackieMemoryPool.h"
struct TestMemoryPool
{
	int allocationId;
};
static void test_MemoryPool_funcs()
{
	std::cout << "test_MemoryPool_funcs starts...\n";
	DataStructures::JackieMemoryPool<TestMemoryPool> memoryPool;
	for (int i = 0; i < 100000; i++)
	{
		TestMemoryPool* test = memoryPool.Allocate();
		test->allocationId = i;
		//printf_s("allocationId(%d)\n", test->allocationId);
		memoryPool.Reclaim(test);
	}
}

#include "JackieArraryQueue.h"
#include "JackieSPSCQueue.h"
#include "EasyLog.h"
#include "JackieArrayList.h"
#include "JackieOrderArraryList.h"
#include "JakieOrderArrayListMap.h"
#include "JackieLinkedList.h"


static void test_Queue_funcs()
{
	JINFO << "test_Queue_funcs STARTS...";

	DataStructures::JackieSPSCQueue<int, 4 * 100000> lockfree;
	TIMED_BLOCK(LockFreeQueueTimer, "JackieSPSCQueue")
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

	DataStructures::JackieArraryQueue<int> alloc;
	DataStructures::JackieArraryQueue<int> dealloc;
	TIMED_BLOCK(RingBufferQueueTimer, "RingBufferQueueTimer")
	{
		for (int i = 0; i < 100000; i++)
		{
			alloc.PushTail(i);
			alloc.PopHead(i);
			dealloc.PushTail(i);
			dealloc.PopHead(i);
		}
	}

	DataStructures::JackieArrayList<int, 100001> list;
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

	DataStructures::JackieOrderArraryList<int, int> olist;
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

	DataStructures::JakieOrderArrayListMap<std::string, int> omap;
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

	DataStructures::JackieDoubleLinkedList<int> linkedlist;
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

	//DataStructures::ArrayCircularList<int, 1000> arrayCircularList;
	//TIMED_BLOCK(ArrayCircularListTimer, "ArrayCircularListTimer")
	//{
	//	int val;
	//	arrayCircularList.Add(2);
	//	arrayCircularList.Add(3);
	//	arrayCircularList.Add(8);

	//	arrayCircularList.Print();

	//	bool flag = arrayCircularList.Has(8);
	//	DCHECK(flag == true);
	//	flag = arrayCircularList.Has(13);
	//	DCHECK(flag == false);

	//	arrayCircularList.Remove(8);
	//	flag = arrayCircularList.Has(8);
	//	DCHECK(flag == false);

	//	val = arrayCircularList.Pop();
	//	DCHECK(val == 2);
	//	flag = arrayCircularList.Has(2);
	//	DCHECK(flag == false);

	//	arrayCircularList.Print();

	//	arrayCircularList.Add(2);
	//	arrayCircularList.Add(8);

	//	arrayCircularList.Print();

	//	flag = arrayCircularList.Has(8);
	//	DCHECK(flag == true);
	//	flag = arrayCircularList.Has(3);
	//	DCHECK(flag == true);
	//}
}

#include "JackieBits.h"
static bool IncomeDatagramEventHandler(JISRecvParams *param)
{
	JackieBits jb((UInt8*)param->data, param->bytesRead, false);
	MessageID msgid;
	jb.Read(msgid);
	JINFO << "recv from  " << param->senderINetAddress.ToString() << ", bytes " << param->bytesRead << " with msg id " << (int)msgid;

	return true;
}
#include "JackieApplication.h"
#include "MessageID.h"
#include "JackieIPlugin.h"
#include "SecurityHandShake.h"

static const unsigned char OFFLINE_MESSAGE_DATA_ID[16] =
{
	0x00, 0xFF, 0xFF, 0x00, 0xFE, 0xFE, 0xFE, 0xFE,
	0xFD, 0xFD, 0xFD, 0xFD, 0x12, 0x34, 0x56, 0x78
};
static void test_ServerApplication_funcs()
{
	JINFO << "test_ServerApplication_funcs STARTS...";

	JackieIPlugin plugin;
	JACKIE_INET::JackieApplication* server = JACKIE_INET::JackieApplication::GetInstance();
	server->SetSleepTime(5);
	server->SetIncomingConnectionsPasswd("admin", (int)strlen("admin"));
	server->SetBannedRemoteSystem("202.168.1.123", 100);
	server->SetBannedRemoteSystem("202.168.1", 10000);
	server->SetBannedRemoteSystem("202.168.1", 1000);
	server->SetBannedRemoteSystem("192.168.0.168", 0);
	server->SetPlugin(&plugin);

#if ENABLE_SECURE_HAND_SHAKE==1
	{
		cat::EasyHandshake handshake;
		char public_key[cat::EasyHandshake::PUBLIC_KEY_BYTES];
		char private_key[cat::EasyHandshake::PRIVATE_KEY_BYTES];

		// generated key pairs are not encrypted
		handshake.GenerateServerKey(public_key, private_key);
		server->EnableSecureIncomingConnections(public_key, private_key, false);

		FILE *fp = fopen("..\\publicKey.pk", "wb");
		fwrite(public_key, sizeof(public_key), 1, fp);
		fclose(fp);
	}
#endif

	/// default blobking 
	JACKIE_INET::JackieBindingSocket socketDescriptor("localhost", 38000);
	server->Start(&socketDescriptor);

	JackiePacket* packet;
	Command* c;
	while (1)
	{
		// This sleep keeps RakNet responsive
		for (packet = server->GetPacketOnce(); packet != 0; server->ReclaimPacket(packet), packet = server->GetPacketOnce())
		{
			/// user logics goes here
			c = server->AllocCommand();
			c->commandID = Command::BCS_SEND;
			server->PostComand(c);
		}
	}

	server->StopRecvThread();
	server->StopNetworkUpdateThread();
	JackieApplication::DestroyInstance(server);


}

#include "JackieBits.h"
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
	JackieAddress addr("192.168.1.107", 32000);
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

	for (int i = 1000; i >= 0; i--)
	{
		UInt32 looptimes = 100;
		for (UInt32 i = 1; i <= looptimes; i++)
		{
			s8.Write(uint24);

			s8.WriteMini(guid);

			s8.WriteMini(uint24);

			s8.WriteMini(addr);

			s8.WriteMini(uint24);

			s8.Write(uint8);
			s8.Write(int64);
			s8.WriteMini(uint8);
			s8.WriteMini<SignedInteger>(int64);

			s8.Write(uint16);
			s8.Write(int32);
			s8.WriteMini(uint16);
			s8.WriteMini<SignedInteger>(int32);

			s8.WriteBits(&particialByte, 4, true);
			s8.Write(uint24);
			s8.WriteNormVector(vector_.x, vector_.y, vector_.z);

			s8.WriteIntegerRange(curr, min, max);

			s8.WriteVector(vector__.x, vector__.y, vector__.z);

			s8.Write(uint32);
			s8.Write(int16);
			s8.WriteMini(uint32);
			s8.WriteMini<SignedInteger>(int16);

			s8.WriteBits(&particialByte, 4, false);

			s8.Write(uint64);
			s8.Write(int8);
			s8.WriteMini(uint64);
			s8.WriteMini<UnSignedInteger>(int8);

			s8.WriteBits(&particialByte, 7, false);
		}

		JackieBits s9;
		s9.Write(s8);

		for (UInt32 i = 1; i <= looptimes; i++)
		{
			uint24 = 0;
			s9.Read(uint24);
			DCHECK(uint24.val == 24);

			JackieGUID guidd;
			s9.ReadMini(guidd);
			DCHECK(guid == guidd);

			UInt24 mini_uint24 = 0;
			s9.ReadMini(mini_uint24);
			DCHECK(mini_uint24.val == 24);

			JackieAddress addrr;
			s9.ReadMini(addrr);
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
			s9.ReadMini<SignedInteger>(mini_int64);
			DCHECK(mini_int64 == int64);

			s9.Read(uint16);
			s9.Read(int32);
			UInt16 mini_uint16;
			s9.ReadMini(mini_uint16);
			DCHECK(mini_uint16 == uint16);
			Int32 mini_int32;
			s9.ReadMini<SignedInteger>(mini_int32);
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
			s9.ReadMini<SignedInteger>(mini_int16);
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
			s9.ReadMini<SignedInteger>(mini_int8);
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

#include "JackieString.h"
static void test_jackie_string()
{
	JINFO << "Test jackie_string";
	TIMED_FUNC(test);
	JackieString str(0, "hello");
	str = "In a previous job, I was asked to look at hashing functions and got into a dispute with my boss about how such functions should be designed. I had advocated the used of LFSRs or";

	JackieBits js;
	str.Write(&js);
	UInt32 vv = js.GetWrittenBytesCount();
	printf("%d\n", js.GetWrittenBytesCount());
	str.WriteMini(&js);
	printf("%d\n", js.GetWrittenBytesCount() - vv);
	JDEBUG << "compresee rate is %" << (int)(((js.GetWrittenBytesCount() - vv) / (float)vv) * 100);
	str.Write(&js);

	JackieString str2((UInt32)0);
	str2.Read(&js);
	str2.Printf();
	printf("\n");
	str2.ReadMini(&js);
	str2.Printf();
	printf("\n");
	str2.Read(&js);
	str2.Printf();
	printf("\n");

	printf("use jackie bits to read and write\n");
	js.Reset();
	js.Write(str);
	js.WriteMini(str);
	js.Write((char*)"hello man");

	JackieString str3((UInt32)0);
	js.Read(str3);
	str3.Printf();
	printf("\n");

	js.ReadMini(str3);
	str3.Printf();
	printf("\n");

	js.Read(str3);
	str3.Printf();
	printf("\n");




}

#include "JackieBytesPool.h"
static void test_JackieBytesPool()
{
	TIMED_BLOCK(JackieBytesPoolTimer, "JackieBytesPool")
	{
		for (unsigned i = 0; i < 100; i++)
		{
			unsigned char* ptr[8000];
			for (unsigned i = 0; i < 8000; i++)
			{
				ptr[i] = JackieBytesPool::GetInstance()->Allocate(i + 1, TRACE_FILE_AND_LINE_);
			}
			for (unsigned i = 0; i < 8000; i++)
			{
				JackieBytesPool::GetInstance()->Release(ptr[i], TRACE_FILE_AND_LINE_);
			}
		}
	}

	TIMED_BLOCK(NewDeleteTimer, "NewDeletePool")
	{
		for (unsigned i = 0; i < 100; i++)
		{
			unsigned char* ptr[8000];
			for (unsigned i = 0; i < 8000; i++)
			{
				ptr[i] = (unsigned char*)jackieMalloc_Ex(i + 1, TRACE_FILE_AND_LINE_);
			}
			for (unsigned i = 0; i < 8000; i++)
			{
				jackieFree_Ex(ptr[i], TRACE_FILE_AND_LINE_);
			}
		}
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

	JackieStringClass,

	JackieBytePoolClass,

	AllFuncs,

};

//static int testcase = GlobalFunctions_h;
//static int testfunc = AllFuncs;

//static int testcase = JACKIE_INET_Address_h;
//static int testfunc = AllFuncs;

//static int testcase = JACKIE_INet_GUID_h;
//static int testfunc = AllFuncs;

//static int testcase = JackieAddressGuidWrapper;
//static int testfunc = AllFuncs;

//static int testcase = NetTime_h;
//static int testfunc = AllFuncs;

//static int testcase = JACKIE_INet_Socket_h;
//static int testfunc = ClassJISBerkley;

//static int testcase = MemoryPool_h;
//static int testfunc = AllFuncs;

//static int testcase = CircularArrayQueueSingleThread;
//static int testfunc = Test_Queue_funcs;

//static int testcase = ServerApplication_H;
//static int testfunc = AllFuncs;

//static int testcase = JackieStream_H;
//static int testfunc = AllFuncs;

//static int testcase = JackieStringClass;
//static int testfunc = AllFuncs;

static int testcase = JackieBytePoolClass;
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
	case JackieStringClass:
		test_jackie_string();
		JackieString::FreeMemory();
		break;
	case JackieBytePoolClass:
		test_JackieBytesPool();
		JackieBytesPool::DestroyInstance();
		break;
	default:
		break;
	}
	return 0;
}
