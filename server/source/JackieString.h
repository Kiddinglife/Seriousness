#ifndef __JACKIE_STRING_H__
#define __JACKIE_STRING_H__

#include <stdarg.h>
#include <cstdio>
#include "DLLExport.h"
#include "JackieArrayList.h"
#include "NetTypes.h" // int64_t
#include "JakieOrderArrayListMap.h"
#include "JackieLinkedList.h"
#include "JackieSimpleMutex.h"
#include "JACKIE_Atomic.h"

#ifdef _WIN32
#include "WindowsIncludes.h"
#endif

/// LinuxStrings Compatiable
#if  defined(__native_client__)
#ifndef _stricmp
int _stricmp(const char* s1, const char* s2);
#endif
int _strnicmp(const char* s1, const char* s2, size_t n);
char *_strlwr(char * str);
#define _vsnprintf vsnprintf
#else
#if (defined(__GNUC__)  || defined(__GCCXML__) || defined(__S3E__) ) && !defined(_WIN32)
#ifndef _stricmp
int _stricmp(const char* s1, const char* s2);
#endif 
int _strnicmp(const char* s1, const char* s2, size_t n);
// http://www.jenkinssoftware.com/forum/index.php?topic=5010.msg20920#msg20920
//   #ifndef _vsnprintf
#define _vsnprintf vsnprintf
// #endif
#ifndef __APPLE__
char *_strlwr(char * str); //this won't compile on OSX for some reason
#endif
#endif
#endif

namespace JACKIE_INET
{
	class JackieBits;
	class JackieString;

	///  [Internal] A single node in the Huffman Encoding Tree.
	struct HuffmanEncodingTreeNode
	{
		unsigned char value;
		unsigned weight;
		HuffmanEncodingTreeNode *left;
		HuffmanEncodingTreeNode *right;
		HuffmanEncodingTreeNode *parent;
	};

	/// This generates special cases of the huffman encoding tree using 8 bit keys with the additional condition that unused combinations of 8 bits are treated as a frequency of 1
	class JACKIE_EXPORT HuffmanEncodingTree
	{

	public:
		HuffmanEncodingTree();
		~HuffmanEncodingTree();

		/// \brief Pass an array of bytes to array and a preallocated JackieBits to receive the output.
		/// \param [in] input Array of bytes to encode
		/// \param [in] sizeInBytes size of \a input
		/// \param [out] output The bitstream to write to
		void EncodeArray(unsigned char *input, size_t sizeInBytes, JackieBits * output);

		// \brief Decodes an array encoded by EncodeArray().
		unsigned DecodeArray(JackieBits * input, BitSize sizeInBits, size_t maxCharsToWrite, unsigned char *output);
		void DecodeArray(unsigned char *input, BitSize sizeInBits, JackieBits * output);

		/// \brief Given a frequency table of 256 elements, all with a frequency of 1 or more, generate the tree.
		void GenerateFromFrequencyTable(unsigned int frequencyTable[256]);

		/// \brief Free the memory used by the tree.
		void FreeMemory(void);

	private:

		/// The root node of the tree 

		HuffmanEncodingTreeNode *root;

		/// Used to hold bit encoding for one character


		struct CharacterEncoding
		{
			unsigned char* encoding;
			unsigned short bitLength;
		};

		CharacterEncoding encodingTable[256];

		void InsertNodeIntoSortedList(HuffmanEncodingTreeNode * node, DataStructures::JackieDoubleLinkedList<HuffmanEncodingTreeNode *> *huffmanEncodingTreeNodeList) const;
	};

	/// \internally used in JackieString
	/// \brief String class
	/// \details Has the following improvements over std::string
	/// -Reference counting: Suitable to store in lists
	/// -Variadic assignment operator
	/// -Doesn't cause linker errors
	struct SharedString
	{
		//JackieSimpleMutex *refCountMutex;
		//unsigned int refCount;
		size_t bytesUsed;
		char *bigString;
		char *c_str;
		JackieAtomicLong refCount;
		char smallString[128 - sizeof(unsigned int) - sizeof(size_t) - sizeof(char*) * 2];
	};

	class JACKIE_EXPORT JackieStringCompressor
	{
	private:
		/// Singleton instance
		static JackieStringCompressor *instance;
		/// Pointer to the huffman encoding trees.
		DataStructures::JakieOrderArrayListMap<int, HuffmanEncodingTree *> huffmanEncodingTrees;
		static int referenceCount;

	public:
		JackieStringCompressor();
		virtual ~JackieStringCompressor();

		/// static function because only static functions can access static members
		/// The RakPeer constructor adds a reference to this class, so don't call this until an instance of RakPeer exists, or unless you call AddReference yourself.
		/// \return the unique instance of the JackieStringCompressor 
		static JackieStringCompressor* Instance(void);

		/// Given an array of strings, such as a chat log, generate the optimal encoding tree for it.
		/// This function is optional and if it is not called a default tree will be used instead.
		/// \param[in] input An array of bytes which should point to text.
		/// \param[in] inputLength Length of \a input
		/// \param[in] languageID An identifier for the language / string table to generate the tree for.  English is automatically created with ID 0 in the constructor.
		void GenerateTreeFromStrings(unsigned char *input, unsigned inputLength, UInt8 languageId);

		/// Writes input to output, compressed.  Takes care of the null terminator for you.
		/// \param[in] input Pointer to an ASCII string
		/// \param[in] maxCharsToWrite The max number of bytes to write of \a input.  Use 0 to mean no limit.
		/// \param[out] output The bitstream to write the compressed string to
		/// \param[in] languageID Which language to use
		void EncodeString(const char *input, int maxCharsToWrite, JackieBits *output, UInt8 languageId = 0);

		/// Writes input to output, uncompressed.  Takes care of the null terminator for you.
		/// \param[out] output A block of bytes to receive the output
		/// \param[in] maxCharsToWrite Size, in bytes, of \a output .  A NULL terminator will always be appended to the output string.  If the maxCharsToWrite is not large enough, the string will be truncated.
		/// \param[in] input The bitstream containing the compressed string
		/// \param[in] languageID Which language to use
		bool DecodeString(char *output, int maxCharsToWrite, JackieBits *input, UInt8 languageId = 0);

#ifdef _CSTRING_COMPRESSOR
		void EncodeString(const CString &input, int maxCharsToWrite, JackieBits *output, UInt8 languageId = 0);
		bool DecodeString(CString &output, int maxCharsToWrite, JackieBits *input, UInt8 languageId = 0);
#endif

#ifdef _STD_STRING_COMPRESSOR
		void EncodeString(const std::string &input, int maxCharsToWrite, JackieBits *output, UInt8 languageId = 0);
		bool DecodeString(std::string *output, int maxCharsToWrite, JackieBits *input, UInt8 languageId = 0);
#endif

		void EncodeString(const JackieString *input, int maxCharsToWrite, JackieBits *output, UInt8 languageId = 0);
		bool DecodeString(JackieString *output, int maxCharsToWrite, JackieBits *input, UInt8 languageId = 0);

		/// Used so I can allocate and deallocate this singleton at runtime
		static void AddReference(void);
		/// Used so I can allocate and deallocate this singleton at runtime
		static void RemoveReference(void);
	};

	/// \brief String class
	/// \details Has the following improvements over std::string
	/// -Reference counting: Suitable to store in lists
	/// -Variadic assignment operator
	/// -Doesn't cause linker errors
	class JACKIE_EXPORT JackieString
	{
	public:
		// Constructors
		JackieString(UInt8 threadid);
		JackieString(UInt8 threadid, char input);
		JackieString(UInt8 threadid, unsigned char input);
		JackieString(UInt8 threadid, const unsigned char *format, ...);
		JackieString(UInt8 threadid, const char *format, ...);

		virtual ~JackieString();
		JackieString(const JackieString & rhs);

		// Implicit return of const char*
		operator const char* () const { return sharedString->c_str; }
		// Lets you modify the string. Do not make the string longer - 
		// however, you can make it shorter, or change the contents.
		// Pointer is only valid in the scope of JackieString itself
		char *C_StringUnsafe(void) { Clone(); return sharedString->c_str; }
		// Same as std::string::c_str
		const char *C_String(void) const { return sharedString->c_str; }

		/// Assigment operators
		JackieString& operator = (const JackieString& rhs);
		JackieString& operator = (const char *str);
		JackieString& operator = (char *str);
		JackieString& operator = (const unsigned char *str);
		JackieString& operator = (char unsigned *str);
		JackieString& operator = (const char c);

		/// Concatenation
		JackieString& operator +=(const JackieString& rhs);
		JackieString& operator += (const char *str);
		JackieString& operator += (char *str);
		JackieString& operator += (const unsigned char *str);
		JackieString& operator += (char unsigned *str);
		JackieString& operator += (const char c);

		/// Equality
		bool operator==(const JackieString &rhs) const;
		bool operator==(const char *str) const;
		bool operator==(char *str) const;

		// Comparison
		bool operator < (const JackieString& right) const;
		bool operator <= (const JackieString& right) const;
		bool operator > (const JackieString& right) const;
		bool operator >= (const JackieString& right) const;

		/// Inequality
		bool operator!=(const JackieString &rhs) const;
		bool operator!=(const char *str) const;
		bool operator!=(char *str) const;

		/// Character index. Read-only. Do not use to change the string however.
		unsigned char operator[] (const unsigned int position) const;

#ifdef _WIN32
		// Return as Wide char
		// Deallocate with DeallocWideChar
		const WCHAR * ToWideChar(void);
		void DeallocWideChar(WCHAR * w);

		void FromWideChar(const wchar_t *source);
		static JACKIE_INET::JackieString FromWideChar_S(const wchar_t *source);
#endif

		/// Change all characters to lowercase
		const char * ToLower(void);

		/// Change all characters to uppercase
		const char * ToUpper(void);

		/// Set the value of the string
		void Set(const char *format, ...);

		/// Sets a copy of a substring of str as the new content. The substring is the portion of str 
		/// that begins at the character position pos and takes up to n characters 
		/// (it takes less than n if the end of str is reached before).
		/// \param[in] str The string to copy in
		/// \param[in] pos The position on str to start the copy
		/// \param[in] n How many chars to copy
		/// \return Returns the string, note that the current string is set to that value as well
		JackieString Assign(const char *str, size_t pos, size_t n);

		/// Returns if the string is empty. Also, C_String() would return ""
		bool IsEmpty(void) const;

		/// Returns the length of the string
		size_t GetLength(void) const;
		size_t GetLengthUTF8(void) const;

		/// Replace character(s) in starting at index, for count, with c
		void Replace(unsigned index, unsigned count, unsigned char c);

		/// Replace character at index with c
		void SetChar(unsigned index, unsigned char c);

		/// Replace character at index with string s
		void SetChar(unsigned index, JACKIE_INET::JackieString s);

		/// Make sure string is no longer than \a length
		void Truncate(unsigned int length);
		void TruncateUTF8(unsigned int length);

		// Gets the substring starting at index for count characters
		JackieString SubStr(unsigned int index, unsigned int count) const;

		/// Erase characters out of the string at index for count
		void Erase(unsigned int index, unsigned int count);

		/// Set the first instance of c with a NULL terminator
		void TerminateAtFirstCharacter(char c);
		/// Set the last instance of c with a NULL terminator
		void TerminateAtLastCharacter(char c);

		void StartAfterFirstCharacter(char c);
		void StartAfterLastCharacter(char c);

		/// Returns how many occurances there are of \a c in the string
		int GetCharacterCount(char c);

		/// Remove all instances of c
		void RemoveCharacter(char c);

		/// Create a JackieString with a value, without doing printf style parsing
		/// Equivalent to assignment operator
		static JACKIE_INET::JackieString NonVariadic(const char *str);

		/// Hash the string into an unsigned int
		static unsigned long ToInteger(const char *str);
		static unsigned long ToInteger(const JackieString &rs);

		/// \brief Read an integer out of a substring
		/// \param[in] str The string
		/// \param[in] pos The position on str where the integer starts
		/// \param[in] n How many chars to copy
		static int ReadIntFromSubstring(const char *str, size_t pos, size_t n);

		// Like strncat, but for a fixed length
		void AppendBytes(const char *bytes, unsigned int count);

		/// Compare strings (case sensitive)
		int StrCmp(const JackieString &rhs) const;

		/// Compare strings (case sensitive), up to num characters
		int StrNCmp(const JackieString &rhs, size_t num) const;

		/// Compare strings (not case sensitive)
		int StrICmp(const JackieString &rhs) const;

		/// Clear the string
		void Clear();

		/// Print the string to the screen
		void Printf(void);

		/// Print the string to a file
		void FPrintf(FILE *fp);

		/// Does the given IP address match the IP address encoded into this string, accounting for wildcards?
		bool IPAddressMatch(const char *IP);

		/// Does the string contain non-printable characters other than spaces?
		bool ContainsNonprintableExceptSpaces(void) const;

		/// Is this a valid email address?
		bool IsEmailAddress(void) const;

		/// URL Encode the string. See http://www.codeguru.com/cpp/cpp/cpp_mfc/article.php/c4029/
		JACKIE_INET::JackieString& URLEncode(void);

		/// URL decode the string
		JACKIE_INET::JackieString& URLDecode(void);

		/// https://servers.api.rackspacecloud.com/v1.0 to https://,  servers.api.rackspacecloud.com, /v1.0
		void SplitURI(JACKIE_INET::JackieString &header, JACKIE_INET::JackieString &domain, JACKIE_INET::JackieString &path);

		/// Scan for quote, double quote, and backslash and prepend with backslash
		JACKIE_INET::JackieString& SQLEscape(void);

		/// Format as a POST command that can be sent to a webserver
		/// \param[in] uri For example, masterserver2.raknet.com/testServer
		/// \param[in] contentType For example, text/plain; charset=UTF-8
		/// \param[in] body Body of the post
		/// \return Formatted string
		static JACKIE_INET::JackieString FormatForPOST(const char* uri, const char* contentType, const char* body, const char* extraHeaders = "");
		static JACKIE_INET::JackieString FormatForPUT(const char* uri, const char* contentType, const char* body, const char* extraHeaders = "");

		/// Format as a GET command that can be sent to a webserver
		/// \param[in] uri For example, masterserver2.raknet.com/testServer?__gameId=comprehensivePCGame
		/// \return Formatted string
		static JACKIE_INET::JackieString FormatForGET(const char* uri, const char* extraHeaders = "");

		/// Format as a DELETE command that can be sent to a webserver
		/// \param[in] uri For example, masterserver2.raknet.com/testServer?__gameId=comprehensivePCGame&__rowId=1
		/// \return Formatted string
		static JACKIE_INET::JackieString FormatForDELETE(const char* uri, const char* extraHeaders = "");

		/// Fix to be a file path, ending with /
		JACKIE_INET::JackieString& MakeFilePath(void);

		/// JackieString uses a freeList of old no-longer used strings
		/// Call this function to clear this memory on shutdown
		static void FreeMemory(void);
		/// \internal
		static void FreeMemoryNoMutex(void);

		/// Serialize to a bitstream, uncompressed (slightly faster)
		/// \param[out] bs Bitstream to serialize to
		void Write(JackieBits *bs) const;

		/// Static version of the Serialize function
		static void Write(const char *str, JackieBits *bs);

		/// Serialize to a bitstream, compressed (better bandwidth usage)
		/// \param[out]  bs Bitstream to serialize to
		/// \param[in] languageId languageId to pass to the StringCompressor class
		/// \param[in] writeLanguageId encode the languageId variable in the stream. If false, 0 is assumed, and DeserializeCompressed will not look for this variable in the stream (saves bandwidth)
		/// \pre StringCompressor::AddReference must have been called to instantiate the class (Happens automatically from RakPeer::Startup())
		void WriteMini(JackieBits *bs, UInt8 languageId = 0, bool writeLanguageId = false) const;

		/// Static version of the SerializeCompressed function
		static void WriteMini(const char *str, JackieBits *bs, UInt8 languageId = 0, bool writeLanguageId = false);

		/// Deserialize what was written by Serialize
		/// \param[in] bs Bitstream to serialize from
		/// \return true if the deserialization was successful
		bool Read(JackieBits *bs);

		/// Static version of the Deserialize() function
		static bool Read(char *str, JackieBits *bs);

		/// Deserialize compressed string, written by SerializeCompressed
		/// \param[in] bs Bitstream to serialize from
		/// \param[in] readLanguageId If true, looks for the variable langaugeId in the data stream. Must match what was passed to SerializeCompressed
		/// \return true if the deserialization was successful
		/// \pre StringCompressor::AddReference must have been called to instantiate the class (Happens automatically from RakPeer::Startup())
		bool ReadMini(JackieBits *bs, bool readLanguageId = false);

		/// Static version of the DeserializeCompressed() function
		static bool ReadMini(char *str, JackieBits *bs, bool readLanguageId = false);

		static const char *ToString(Int64 i);
		static const char *ToString(UInt64 i);

		/// \internal
		static size_t GetSizeToAllocate(size_t bytes)
		{
			const size_t smallStringSize = 128 - sizeof(unsigned int) - sizeof(size_t) - sizeof(char*) * 2;
			if (bytes <= smallStringSize)
				return smallStringSize;
			else
				return bytes * 2;
		}

		/// \internal
		JackieString(SharedString *_sharedString);
		/// \internal
		SharedString *sharedString;
		/// \internal
		UInt8 freeListIndex;

		//	static SimpleMutex poolMutex;
		//	static DataStructures::MemoryPool<SharedString> pool;
		/// \internal
		static SharedString emptyString;

		//static SharedString *sharedStringFreeList;
		//static unsigned int sharedStringFreeListAllocationCount;

		/// \internal
		/// List of free objects to reduce memory reallocations
		typedef DataStructures::JackieArrayList<SharedString*, 128> StringPool;
		static DataStructures::JackieArrayList<StringPool*, 32> freeList;
		//static DataStructures::JackieArrayList<SharedString*> freeList;

		static int RakStringComp(JackieString const &key, JackieString const &data);

		static void LockMutex(void);
		static void UnlockMutex(void);

	protected:
		static JACKIE_INET::JackieString FormatForPUTOrPost(const char* type, const char* uri, const char* contentType, const char* body, const char* extraHeaders);
		void Assign(const char *str);
		void Assign(const char *str, va_list ap);

		void Clone(void);
		void Free(void);
		void Allocate(size_t len);
		unsigned char ToLower(unsigned char c);
		unsigned char ToUpper(unsigned char c);
		void Realloc(SharedString *sharedString, size_t bytes);
	};
}

#endif


