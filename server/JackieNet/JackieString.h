#ifndef __JACKIE_STRING_H__
#define __JACKIE_STRING_H__

#include <stdarg.h>
#include <cstdio>
#include "DLLExport.h"
#include "ArrayList.h"
#include "NetTypes.h" // int64_t
#include "OrderListMap.h"

#ifdef _WIN32
#include "WindowsIncludes.h"
#endif

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
	class JackieSimpleMutex;
	class JackieBits;
	class JackieString;
	class HuffmanEncodingTree;

	/// \internally used in JackieString
	struct SharedString
	{
		JackieSimpleMutex *refCountMutex;
		unsigned int refCount;
		size_t bytesUsed;
		char *bigString;
		char *c_str;
		char smallString[128 - sizeof(unsigned int) - sizeof(size_t) - sizeof(char*) * 2];
	};

	class JACKIE_EXPORT JackieStringCompressor
	{
	private:
		/// Singleton instance
		static JackieStringCompressor *instance;
		/// Pointer to the huffman encoding trees.
		DataStructures::OrderListMap<int, HuffmanEncodingTree *> huffmanEncodingTrees;
		static int referenceCount;

	public:
		JackieStringCompressor();
		~JackieStringCompressor();

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
	private:
		/// \internal
		JackieString(SharedString *_sharedString);

		/// \internal
		SharedString *sharedString;

		/// \internal
		/// List of free objects to reduce memory reallocations
		static DataStructures::ArrayList<SharedString*> freeList;

		/// \internal
		static SharedString emptyString;

	public:
		// Constructors
		JackieString();
		JackieString(char input);
		JackieString(unsigned char input);
		JackieString(const unsigned char *format, ...);
		JackieString(const char *format, ...);
		virtual ~JackieString();
		JackieString(const JackieString & rhs);
	};
}

#endif


