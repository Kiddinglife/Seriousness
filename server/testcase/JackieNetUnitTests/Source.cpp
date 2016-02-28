#include <iostream>
#include <gtest/gtest.h>
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
#include "NetTypes.h"

using namespace JACKIE_INET;

TEST(IsDomainIPAddrTest, when_given_numberic_addr_return_false)
{
	const char* host_Num = "192.168.1.168";
	EXPECT_FALSE(isDomainIPAddr(host_Num));
}

TEST(IsDomainIPAddrTest, when_given_domain_addr_return_true)
{
	const char* host_domain = "www.baidu.com";
	EXPECT_TRUE(isDomainIPAddr(host_domain));
}

TEST(ItoaTest, when_given_positive_and_nagative_integers_return_correct_string)
{
	char result[8];

	Itoa(12, result, 10);
	EXPECT_STREQ("12", result);

	Itoa(-12, result, 10);
	EXPECT_STREQ("-12", result);

	Itoa(-12.5, result, 10);
	EXPECT_STREQ("-12", result);
}

TEST(DomainNameToIPTest, when_given_localhost_string_return_127001)
{
	char result[65] = { 0 };
	DomainNameToIP("localhost", result);
	EXPECT_STREQ("127.0.0.1", result);
	printf("localhost ip addr ('%s')\n", result);
}

TEST(DomainNameToIPTest, when_given_hostname_return_bound_ip_for_that_nic)
{
	char result[65] = { 0 };
	DomainNameToIP("DESKTOP-E2KL25B", result);
	//EXPECT_STREQ("192.168.56.1", result);
	printf("hostname ip addr ('%s')\n", result);
}

TEST(DomainNameToIPTest, when_given_numberic_addr_return_same_ip_addr)
{
	char result[65] = { 0 };
	DomainNameToIP("192.168.2.5", result);
	EXPECT_STREQ("192.168.2.5", result);
}

TEST(DomainNameToIPTest, when_given_external_domain_return_correct_ip_addr)
{
	char result[65] = { 0 };
	DomainNameToIP("www.baidu.com", result);
	printf("baidu ip addr ('%s')\n", result);
}

TEST(JackieAddressTests, test_JackieAddress_size_equals_7)
{
	EXPECT_EQ(7, JackieAddress::size());
}

TEST(JackieAddressTests, TestToHashCode)
{
	JackieAddress addr3("localhost", 32000);
	printf("hash code for addr '%s' is %ld\n'", addr3.ToString(), JackieAddress::ToHashCode(addr3));

	JackieAddress addr4("192.168.56.1", 32000);
	printf("hash code for addr '%s' is %ld\n'", addr4.ToString(), JackieAddress::ToHashCode(addr4));
}

/// usually seprate the ip addr and port number and you will ne fine
TEST(JackieAddressTests, TestCtorToStringFromString)
{
	JACKIE_INET::JackieAddress default_ctor_addr;
	const char* str1 = default_ctor_addr.ToString();

	JACKIE_INET::JackieAddress param_ctor_addr_localhost("localhost", 12345);
	const char* str2 = param_ctor_addr_localhost.ToString();
	EXPECT_STREQ("127.0.0.1|12345", str2);

	// THIS IS WRONG, so when you use domain name, you have to seprate two-params ctor
	//JACKIE_INET::JACKIE_INET_Address param_ctor_addr_domain("ZMD-SERVER:1234");

	// If you have multiple ip address bound on hostname, this will return the first one,
	// so sometimes, it will not be the one you want to use, so better way is to assign the ip address
	// manually.
	JACKIE_INET::JackieAddress param_ctor_addr_domain("DESKTOP-E2KL25B", 1234);
	const char* str3 = param_ctor_addr_domain.ToString();
	//EXPECT_STREQ("192.168.56.1|1234", str3);
}

static void test_superfastfunction_func()
{
	std::cout << "\nGlobalFunctions_h::test_superfastfunction_func() starts...\n";
	char* name = "jackie";
	std::cout << "name hash code = " << (name, strlen(name) + 1, strlen(name) + 1);
}


TEST(JackieAddressTests, SetToLoopBack_when_given_ip4_return_ip4_loopback)
{
	JACKIE_INET::JackieAddress addr("192.168.1.108", 12345);
	addr.SetToLoopBack(4);
	EXPECT_STREQ("127.0.0.1|12345", addr.ToString());
}

// this function will not work if you do not define NET_SUPPORT_IP6 MACRO
TEST(JackieAddressTests, SetToLoopBack_when_given_ip6_return_ip6_loopback)
{
	JACKIE_INET::JackieAddress addr("192.168.1.108", 12345);
	addr.SetToLoopBack(6);
	EXPECT_STREQ("192.168.1.108|12345", addr.ToString());
}

struct val
{
	val(const char* ip, USHORT port, bool expval)
	{
		addr.FromString(ip, port);
		expectedVal = expval;
	}
	JACKIE_INET::JackieAddress addr;
	bool expectedVal;
};
class  IsLoopbackParamTest : public::testing::TestWithParam <val >{};
TEST_P(IsLoopbackParamTest, handle_return_vals){ val ret = GetParam(); EXPECT_EQ(ret.expectedVal, ret.addr.IsLoopback()); }
INSTANTIATE_TEST_CASE_P(JackieAddressTests, IsLoopbackParamTest, testing::Values(val("localhost", 123456, true), val("192.168.107.1", 123456, false)));

TEST(JackieAddressTests, IsLANAddress_when_given_localhost_return_true)
{
	JACKIE_INET::JackieAddress addr("localhost", 12345);
	EXPECT_FALSE(addr.IsLANAddress()) << " localhost";
}

TEST(JackieAddressTests, IsLANAddress_when_given_non_localhost_return_false)
{
	JACKIE_INET::JackieAddress addr("192.168.1.108", 12345);
	EXPECT_TRUE(addr.IsLANAddress()) << " 192.168.1.108";
}

TEST(JackieGUIDTests, ToUInt32_)
{
	JackieGUID gui(12);
	EXPECT_STREQ("12", gui.ToString());
	EXPECT_EQ(12, JackieGUID::ToUInt32(gui));
}

TEST(JackieGUIDTests, TestToString)
{
	EXPECT_STREQ("JACKIE_INet_GUID_Null", JACKIE_NULL_GUID.ToString());
	JackieGUID gui(12);
	EXPECT_STREQ("12", gui.ToString());
}

TEST(JackieGUIDTests, TestToHashCode)
{
	JackieAddressGuidWrapper wrapper;
	EXPECT_STREQ("204.204.204.204|52428", wrapper.ToString());
	EXPECT_EQ(-395420190, JackieAddressGuidWrapper::ToHashCode(wrapper));

	JackieGUID gui(12);
	JackieAddressGuidWrapper wrapper1(gui);
	EXPECT_STREQ("12", wrapper1.ToString());
	EXPECT_EQ(12, JackieAddressGuidWrapper::ToHashCode(wrapper1));

	JackieAddress adrr("localhost", 12345);
	JackieAddressGuidWrapper wrapper2(adrr);
	EXPECT_STREQ("127.0.0.1|12345", wrapper2.ToString());
	EXPECT_EQ(JackieAddress::ToHashCode(adrr), JackieAddressGuidWrapper::ToHashCode(wrapper2));
}

TEST(NetTimeTests, test_gettimeofday)
{
	time_t rawtime;
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);
	rawtime = tv.tv_sec;

	char buffer[128];
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

#include "JackieINetSocket.h"
TEST(JISBerkleyTests, test_GetMyIPBerkley)
{
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
TEST(JackieMemoryPoolTests, test_alloca_and_release)
{
	DataStructures::JackieMemoryPool<TestMemoryPool> memoryPool;
	for (int i = 0; i < 100000; i++)
	{
		TestMemoryPool* test = memoryPool.Allocate();
		test->allocationId = i;
		//printf_s("allocationId(%d)\n", test->allocationId);
		memoryPool.Reclaim(test);
	}
}

#include "JackieSPSCQueue.h"
struct TestQueueElement
{
	int a;
	int b;
};
TEST(JackieSPSCQueueTests, when_pushTail_and_then_popHead_element_should_be_equal)
{
	DataStructures::JackieSPSCQueue<TestQueueElement, 100000> lockfree;
	TestQueueElement elepush;
	TestQueueElement elepop;
	for (int i = 0; i < 100000; i++)
	{
		elepush.a = i;
		elepush.b = i;
		lockfree.PushTail(elepush);
		lockfree.PopHead(elepop);
		EXPECT_EQ(elepush.a, elepop.a);
		EXPECT_EQ(elepush.b, elepop.b);
	}

}

TEST(JackieSPSCQueueTests, when_pushTail_first_and_then_popHead_element_should_be_equal)
{
	DataStructures::JackieSPSCQueue<TestQueueElement, 10000> lockfree;
	TestQueueElement elepush;
	TestQueueElement elepop;
	for (int i = 0; i < 10000; i++)
	{
		elepush.a = i;
		elepush.b = i;
		lockfree.PushTail(elepush);
	}

	for (int i = 0; i < 10000; i++)
	{
		lockfree.PopHead(elepop);
		EXPECT_EQ(i, elepop.a);
		EXPECT_EQ(i, elepop.b);
	}
}

struct vec { float x; float y; float z; };
TEST(JackieBitsTests, test_all_reads_and_writes)
{
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
	EXPECT_TRUE(curr == 12);

	s8.Write(uint24);
	uint24 = 0;
	s8.Read(uint24);
	EXPECT_TRUE(uint24.val == 24);


	s8.WriteBits(&particialByte, 7, false);
	UInt8 v = 0;
	s8.ReadBits(&v, 7, false);
	EXPECT_TRUE(particialByte == v);
	EXPECT_TRUE(s8.GetPayLoadBits() == 0);

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
			s8.WriteMini<UnSignedInteger>(uint16);
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
			EXPECT_TRUE(uint24.val == 24);

			JackieGUID guidd;
			s9.ReadMini(guidd);
			EXPECT_TRUE(guid == guidd);

			UInt24 mini_uint24 = 0;
			s9.ReadMini(mini_uint24);
			EXPECT_TRUE(mini_uint24.val == 24);

			JackieAddress addrr;
			s9.ReadMini(addrr);
			EXPECT_TRUE(addr == addrr);

			mini_uint24 = 0;
			s9.ReadMini(mini_uint24);
			EXPECT_TRUE(mini_uint24.val == 24);

			s9.Read(uint8);
			s9.Read(int64);
			UInt8 mini_uint8;
			s9.ReadMini(mini_uint8);
			EXPECT_TRUE(mini_uint8 == uint8);

			Int64 mini_int64;
			s9.ReadMini<SignedInteger>(mini_int64);
			EXPECT_TRUE(mini_int64 == int64);

			s9.Read(uint16);
			s9.Read(int32);
			UInt16 mini_uint16;
			s9.ReadMini(mini_uint16);
			EXPECT_TRUE(mini_uint16 == uint16);

			Int32 mini_int32;
			s9.ReadMini<SignedInteger>(mini_int32);
			EXPECT_TRUE(mini_int32 == int32);


			UInt8 v = 0;
			s9.ReadBits(&v, 4, true);
			EXPECT_TRUE(v == 0);

			uint24 = 0;
			s9.Read(uint24);
			EXPECT_TRUE(uint24.val == 24);

			vec vectorr;
			s9.ReadNormVector(vectorr.x, vectorr.y, vectorr.z);
			EXPECT_TRUE(fabs(vectorr.x - vector_.x) <= 0.0001f);
			EXPECT_TRUE(fabs(vectorr.y - vector_.y) <= 0.0001f);
			EXPECT_TRUE(fabs(vectorr.y - vector_.y) <= 0.0001f);

			UInt32 v1;
			s9.ReadIntegerRange(v1, min, max);
			EXPECT_TRUE(v1 == curr);

			vec vectorrr;
			s9.ReadVector(vectorrr.x, vectorrr.y, vectorrr.z);
			EXPECT_TRUE(fabs(vectorrr.x - vector__.x) <= 0.001f);
			EXPECT_TRUE(fabs(vectorrr.y - vector__.y) <= 0.001f);
			EXPECT_TRUE(fabs(vectorrr.y - vector__.y) <= 0.001f);

			s9.Read(uint32);
			s9.Read(int16);
			UInt32 mini_uint32;
			s9.ReadMini(mini_uint32);
			EXPECT_TRUE(mini_uint32 == uint32);

			Int16 mini_int16;
			s9.ReadMini<SignedInteger>(mini_int16);
			EXPECT_TRUE(mini_int16 == int16);

			v = 0;
			s9.ReadBits(&v, 4, false);
			EXPECT_TRUE(particialByte == ((v >> 4) << 4));

			s9.Read(uint64);
			s9.Read(int8);
			UInt64 mini_uint64;
			s9.ReadMini(mini_uint64);
			EXPECT_TRUE(mini_uint64 == uint64);

			Int8 mini_int8;
			s9.ReadMini<SignedInteger>(mini_int8);
			EXPECT_TRUE(mini_int8 == int8);

			v = 0;
			s9.ReadBits(&v, 7, false);
			EXPECT_TRUE(particialByte == ((v >> 1) << 1));

			EXPECT_TRUE(uint8 == 8);
			EXPECT_TRUE(int8 == -8);
			EXPECT_TRUE(uint16 == 16);
			EXPECT_TRUE(int16 == -16);
			EXPECT_TRUE(uint24.val == 24);
			EXPECT_TRUE(uint32 == 32);
			EXPECT_TRUE(int32 == -32);
			EXPECT_TRUE(uint64 == 64);
			EXPECT_TRUE(int64 == -64);
		}

		s8.Reset();
		s9.Reset();
	}
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

TEST(JackieStringTests, test_emptyString)
{
	EXPECT_STREQ("", JackieString::emptyString.c_str);
	EXPECT_STREQ("", JackieString::emptyString.bigString);
	EXPECT_EQ(0, JackieString::emptyString.refCount.GetValue());
}
#include "GlobalFunctions.h"
TEST(JackieStringTests, test_work_with_JackieBits)
{
	UInt32 CURR_THREAD_ID_1 = JACKIE_Thread::JackieGetCurrentThreadId();
	printf_s("before CURR_THREAD_ID_1[%d]\n", CURR_THREAD_ID_1);
	if (CURR_THREAD_ID_1 > JackieString_FREE_LIST_SIZE)
		CURR_THREAD_ID_1 = CURR_THREAD_ID_1 % JackieString_FREE_LIST_SIZE;
	printf_s("after CURR_THREAD_ID_1[%d]\n", CURR_THREAD_ID_1);

	CURR_THREAD_ID_1 = JACKIE_Thread::JackieGetCurrentThreadId();
	printf_s("before CURR_THREAD_ID_1[%d]\n", CURR_THREAD_ID_1);
	if (CURR_THREAD_ID_1 > JackieString_FREE_LIST_SIZE)
		CURR_THREAD_ID_1 = CURR_THREAD_ID_1 % JackieString_FREE_LIST_SIZE;
	printf_s("after CURR_THREAD_ID_1[%d]\n", CURR_THREAD_ID_1);

	JackieString str(CURR_THREAD_ID_1, "hello");
	str = "In a previous job, I was asked to look at hashing functions and got into a dispute with my boss about how such functions should be designed. I had advocated the used of LFSRs or";

	JackieBits js;
	str.Write(&js);
	UInt32 vv = js.GetWrittenBytesCount();
	printf("%d\n", js.GetWrittenBytesCount());

	str.WriteMini(&js);
	printf(" js.GetWrittenBytesCount() - vv '%d'\n", js.GetWrittenBytesCount() - vv);
	printf("compresee rate is '%d%'\n", (int)(((js.GetWrittenBytesCount() - vv) / (float)vv) * 100));

	str.Write(&js);

	JackieString str2(CURR_THREAD_ID_1);

	printf("str2.Read(&js);\n");
	str2.Read(&js);
	str2.Printf();
	printf("\n");

	printf("str2.ReadMini(&js);\n");
	str2.ReadMini(&js);
	str2.Printf();
	printf("\n");

	printf("str2.Read(&js);\n");
	str2.Read(&js);
	str2.Printf();
	printf("\n");

	printf("reuse jackie bits to read and write\n");
	js.Reset();
	js.Write(str);
	js.WriteMini(str);
	js.Write((char*)"hello man");

	JackieString str3(CURR_THREAD_ID_1);
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
TEST(JackieBytesPoolTests, test_pool_alloc_and_release)
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
TEST(JackieBytesPoolTests, test_c_allocator_alloc_and_release)
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
TEST(JackieBytesPoolTests, test_cplus_allocator_alloc_and_release)
{
	for (unsigned i = 0; i < 100; i++)
	{
		unsigned char* ptr[8000];
		for (unsigned i = 0; i < 8000; i++)
		{
			ptr[i] = new unsigned char[i + 1];
		}
		for (unsigned i = 0; i < 8000; i++)
		{
			delete ptr[i];
		}
	}
}

class TestFixtureClass : public testing::Test
{
public:

	static void SetUpTestCase()
	{
		std::cout << "SetUp before all test case." << std::endl;
	}

	static void TearDownTestCase()
	{
		std::cout << "TearDown after all test case." << std::endl;
	}

	void SetUp() override
	{
		std::cout << "SetUp every test case." << std::endl;
	}

	void TearDown() override
	{
		std::cout << "TearDown every test case." << std::endl;
	}

private:
	/**@brief    ²âÊÔ¶ÔÏó */
	JackieAddress     mTestFindAllFiles;
};
TEST_F(TestFixtureClass, test1){}
TEST_F(TestFixtureClass, test2){}

int main(int argc, char** argv)
{
	testing::GTEST_FLAG(break_on_failure) = true;
	testing::InitGoogleTest(&argc, argv);;
	int ret = RUN_ALL_TESTS();
	JackieString::FreeMemory();
	return ret;
}
