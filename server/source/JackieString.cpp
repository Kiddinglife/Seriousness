#include <cstring>
#include <cstdlib>

#include "JackieString.h"
#include "OverrideMemory.h"
#include "JackieBits.h"
#include "GlobalFunctions.h"
#include "JackieArraryQueue.h"
#include "JakieOrderArrayListMap.h"

#if (defined(__GNUC__) || defined(__ARMCC_VERSION) || defined(__GCCXML__) || defined(__S3E__) ) && !defined(_WIN32)
#include <string.h>
#ifndef _stricmp
int _stricmp(const char* s1, const char* s2)
{
	return strcasecmp(s1, s2);
}
#endif
int _strnicmp(const char* s1, const char* s2, size_t n)
{
	return strncasecmp(s1, s2, n);
}
#ifndef _vsnprintf
#define _vsnprintf vsnprintf
#endif
#ifndef __APPLE__
char *_strlwr(char * str)
{
	if (str == 0)
		return 0;
	for (int i = 0; str[i]; i++)
	{
		if (str[i] >= 'A' && str[i] <= 'Z')
			str[i] += 'a' - 'A';
	}
	return str;
}
#endif
#endif

namespace JACKIE_INET
{

	/// HuffmanEncodingTree implementations
	HuffmanEncodingTree::HuffmanEncodingTree(){ root = 0; }
	HuffmanEncodingTree::~HuffmanEncodingTree()
	{
		FreeMemory();
	}
	void HuffmanEncodingTree::FreeMemory(void)
	{
		if (root == 0)
			return;

		// Use an in-order traversal to delete the tree
		DataStructures::JackieArraryQueue<HuffmanEncodingTreeNode *> nodeQueue;
		HuffmanEncodingTreeNode *node;
		nodeQueue.PushTail(root);

		while (nodeQueue.Size() > 0)
		{
			nodeQueue.PopHead(node);

			if (node->left)
				nodeQueue.PushTail(node->left);

			if (node->right)
				nodeQueue.PushTail(node->right);

			JACKIE_INET::OP_DELETE(node, TRACE_FILE_AND_LINE_);
		}

		// Delete the encoding table
		for (int i = 0; i < 256; i++)
			jackieFree_Ex(encodingTable[i].encoding, TRACE_FILE_AND_LINE_);

		root = 0;
	}
	// Given a frequency table of 256 elements, all with a frequency of 1 or more, generate the tree
	void HuffmanEncodingTree::GenerateFromFrequencyTable(unsigned int frequencyTable[256])
	{
		int counter;
		HuffmanEncodingTreeNode * node;
		HuffmanEncodingTreeNode *leafList[256]; // Keep a copy of the pointers to all the leaves so we can generate the encryption table bottom-up, which is easier
		// 1.  Make 256 trees each with a weight equal to the frequency of the corresponding character
		DataStructures::JackieDoubleLinkedList<HuffmanEncodingTreeNode *> huffmanEncodingTreeNodeList;

		FreeMemory();

		for (counter = 0; counter < 256; counter++)
		{
			node = JACKIE_INET::OP_NEW<HuffmanEncodingTreeNode>(TRACE_FILE_AND_LINE_);
			node->left = 0;
			node->right = 0;
			node->value = (unsigned char)counter;
			node->weight = frequencyTable[counter];

			if (node->weight == 0)
				node->weight = 1; // 0 weights are illegal

			leafList[counter] = node; // Used later to generate the encryption table

			InsertNodeIntoSortedList(node, &huffmanEncodingTreeNodeList); // Insert and maintain sort order.
		}


		// 2.  While there is more than one tree, take the two smallest trees and merge them so that the two trees are the left and right
		// children of a new node, where the new node has the weight the sum of the weight of the left and right child nodes.
#ifdef _MSC_VER
#pragma warning( disable : 4127 ) // warning C4127: conditional expression is constant
#endif
		while (1)
		{
			huffmanEncodingTreeNodeList.Beginning();
			HuffmanEncodingTreeNode *lesser, *greater;
			lesser = huffmanEncodingTreeNodeList.Pop();
			greater = huffmanEncodingTreeNodeList.Pop();
			node = JACKIE_INET::OP_NEW<HuffmanEncodingTreeNode>(TRACE_FILE_AND_LINE_);
			node->left = lesser;
			node->right = greater;
			node->weight = lesser->weight + greater->weight;
			lesser->parent = node;  // This is done to make generating the encryption table easier
			greater->parent = node;  // This is done to make generating the encryption table easier

			if (huffmanEncodingTreeNodeList.Size() == 0)
			{
				// 3. Assign the one remaining node in the list to the root node.
				root = node;
				root->parent = 0;
				break;
			}

			// Put the new node back into the list at the correct spot to maintain the sort.  Linear search time
			InsertNodeIntoSortedList(node, &huffmanEncodingTreeNodeList);
		}

		bool tempPath[256]; // Maximum path length is 256
		unsigned short tempPathLength;
		HuffmanEncodingTreeNode *currentNode;
		JackieBits bitStream;

		// Generate the encryption table. From before, we have an array of pointers to all the leaves which contain pointers to their parents.
		// This can be done more efficiently but this isn't bad and it's way easier to program and debug

		for (counter = 0; counter < 256; counter++)
		{
			// Already done at the end of the loop and before it!
			tempPathLength = 0;

			// Set the current node at the leaf
			currentNode = leafList[counter];

			do
			{
				if (currentNode->parent->left == currentNode)   // We're storing the paths in reverse order.since we are going from the leaf to the root
					tempPath[tempPathLength++] = false;
				else
					tempPath[tempPathLength++] = true;

				currentNode = currentNode->parent;
			}

			while (currentNode != root);

			// Write to the bitstream in the reverse order that we stored the path, which gives us the correct order from the root to the leaf
			while (tempPathLength-- > 0)
			{
				if (tempPath[tempPathLength])   // Write 1's and 0's because writing a bool will write the JackieBits TYPE_CHECKING validation bits if that is defined along with the actual data bit, which is not what we want
					bitStream.WriteBitOne();
				else
					bitStream.WriteBitZero();
			}

			// Read data from the bitstream, which is written to the encoding table in bits and bitlength. Note this function allocates the encodingTable[counter].encoding pointer
			encodingTable[counter].bitLength = (unsigned char)bitStream.Copy(encodingTable[counter].encoding);

			// Reset the bitstream for the next iteration
			bitStream.Reset();
		}
	}

	// Insertion sort.  Slow but easy to write in this case
	void HuffmanEncodingTree::InsertNodeIntoSortedList(HuffmanEncodingTreeNode * node, DataStructures::JackieDoubleLinkedList<HuffmanEncodingTreeNode *> *huffmanEncodingTreeNodeList) const
	{
		if (huffmanEncodingTreeNodeList->Size() == 0)
		{
			huffmanEncodingTreeNodeList->Insert(node);
			return;
		}

		huffmanEncodingTreeNodeList->Beginning();

		unsigned counter = 0;
#ifdef _MSC_VER
#pragma warning( disable : 4127 ) // warning C4127: conditional expression is constant
#endif
		while (1)
		{
			if (huffmanEncodingTreeNodeList->Peek()->weight < node->weight)
				++(*huffmanEncodingTreeNodeList);
			else
			{
				huffmanEncodingTreeNodeList->Insert(node);
				break;
			}

			// Didn't find a spot in the middle - add to the end
			if (++counter == huffmanEncodingTreeNodeList->Size())
			{
				huffmanEncodingTreeNodeList->End();
				huffmanEncodingTreeNodeList->Add(node)
					; // Add to the end
				break;
			}
		}
	}

	// Pass an array of bytes to array and a preallocated JackieBits to receive the output
	void HuffmanEncodingTree::EncodeArray(unsigned char *input, size_t sizeInBytes, JackieBits * output)
	{
		unsigned counter;

		// For each input byte, Write out the corresponding series of 1's and 0's that give the encoded representation
		for (counter = 0; counter < sizeInBytes; counter++)
		{
			output->WriteBits(encodingTable[input[counter]].encoding, encodingTable[input[counter]].bitLength, false); // Data is left aligned
		}

		// Byte align the output so the unassigned remaining bits don't equate to some actual value
		if (output->GetWrittenBitsCount() % 8 != 0)
		{
			// Find an input that is longer than the remaining bits.  Write out part of it to pad the output to be byte aligned.
			unsigned char remainingBits = (unsigned char)(8 - (output->GetWrittenBitsCount() % 8));

			for (counter = 0; counter < 256; counter++)
				if (encodingTable[counter].bitLength > remainingBits)
				{
					output->WriteBits(encodingTable[counter].encoding, remainingBits, false); // Data is left aligned
					break;
				}

#ifdef _DEBUG
			assert(counter != 256);  // Given 256 elements, we should always be able to find an input that would be >= 7 bits
#endif

		}
	}

	unsigned HuffmanEncodingTree::DecodeArray(JackieBits * input, BitSize sizeInBits, size_t maxCharsToWrite, unsigned char *output)
	{
		HuffmanEncodingTreeNode * currentNode;

		unsigned outputWriteIndex;
		outputWriteIndex = 0;
		currentNode = root;

		// For each bit, go left if it is a 0 and right if it is a 1.  When we reach a leaf, that gives us the desired value and we restart from the root

		for (unsigned counter = 0; counter < sizeInBits; counter++)
		{
			if (input->ReadBit() == false)   // left!
				currentNode = currentNode->left;
			else
				currentNode = currentNode->right;

			if (currentNode->left == 0 && currentNode->right == 0)   // Leaf
			{

				if (outputWriteIndex < maxCharsToWrite)
					output[outputWriteIndex] = currentNode->value;

				outputWriteIndex++;

				currentNode = root;
			}
		}

		return outputWriteIndex;
	}

	// Pass an array of encoded bytes to array and a preallocated JackieBits to receive the output
	void HuffmanEncodingTree::DecodeArray(unsigned char *input, BitSize sizeInBits, JackieBits * output)
	{
		HuffmanEncodingTreeNode * currentNode;

		if (sizeInBits <= 0)
			return;

		JackieBits bitStream(input, BITS_TO_BYTES(sizeInBits), false);

		currentNode = root;

		// For each bit, go left if it is a 0 and right if it is a 1.  When we reach a leaf, that gives us the desired value and we restart from the root
		for (unsigned counter = 0; counter < sizeInBits; counter++)
		{
			if (bitStream.ReadBit() == false)   // left!
				currentNode = currentNode->left;
			else
				currentNode = currentNode->right;

			if (currentNode->left == 0 && currentNode->right == 0)   // Leaf
			{
				// Use WriteBits instead of Write(char) because we want to avoid TYPE_CHECKING
				output->WriteBits(&(currentNode->value), 8, true);
				currentNode = root;
			}
		}
	}


	/// JackieStringCompressor
	static unsigned int englishCharacterFrequencies[256] =
	{
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		722,
		0,
		0,
		2,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		11084,
		58,
		63,
		1,
		0,
		31,
		0,
		317,
		64,
		64,
		44,
		0,
		695,
		62,
		980,
		266,
		69,
		67,
		56,
		7,
		73,
		3,
		14,
		2,
		69,
		1,
		167,
		9,
		1,
		2,
		25,
		94,
		0,
		195,
		139,
		34,
		96,
		48,
		103,
		56,
		125,
		653,
		21,
		5,
		23,
		64,
		85,
		44,
		34,
		7,
		92,
		76,
		147,
		12,
		14,
		57,
		15,
		39,
		15,
		1,
		1,
		1,
		2,
		3,
		0,
		3611,
		845,
		1077,
		1884,
		5870,
		841,
		1057,
		2501,
		3212,
		164,
		531,
		2019,
		1330,
		3056,
		4037,
		848,
		47,
		2586,
		2919,
		4771,
		1707,
		535,
		1106,
		152,
		1243,
		100,
		0,
		2,
		0,
		10,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	};

	JackieStringCompressor* JackieStringCompressor::instance = 0;
	int JackieStringCompressor::referenceCount = 0;

	void JackieStringCompressor::RemoveReference(void)
	{
		assert(referenceCount > 0);

		if (referenceCount > 0)
		{
			if (--referenceCount == 0)
			{
				JACKIE_INET::OP_DELETE(instance, TRACE_FILE_AND_LINE_);
				instance = 0;
			}
		}
	}

	JackieStringCompressor* JackieStringCompressor::Instance(void)
	{
		if (instance == 0)
			AddReference();
		return instance;
	}

	JackieStringCompressor::JackieStringCompressor()
	{
		DataStructures::JakieOrderArrayListMap<int, HuffmanEncodingTree *>::IMPLEMENT_DEFAULT_COMPARISON();

		// Make a default tree immediately, since this is used for RPC possibly from multiple threads at the same time
		HuffmanEncodingTree *huffmanEncodingTree = JACKIE_INET::OP_NEW<HuffmanEncodingTree>(TRACE_FILE_AND_LINE_);
		huffmanEncodingTree->GenerateFromFrequencyTable(englishCharacterFrequencies);
		huffmanEncodingTrees.Set(0, huffmanEncodingTree);
	}
	JackieStringCompressor::~JackieStringCompressor()
	{
		for (unsigned i = 0; i < huffmanEncodingTrees.Size(); i++)
			JACKIE_INET::OP_DELETE(huffmanEncodingTrees[i], TRACE_FILE_AND_LINE_);
	}
	void JackieStringCompressor::EncodeString(const char *input, int maxCharsToWrite, JackieBits *output, UInt8 languageId)
	{
		HuffmanEncodingTree *huffmanEncodingTree;
		if (huffmanEncodingTrees.Has(languageId) == false)
			return;
		huffmanEncodingTree = huffmanEncodingTrees.Get(languageId);

		if (input == 0)
		{
			output->WriteMini((UInt32)0);
			return;
		}

		JackieBits encodedBitStream;

		UInt32 stringBitLength;

		int charsToWrite;

		if (maxCharsToWrite <= 0 || (int)strlen(input) < maxCharsToWrite)
			charsToWrite = (int)strlen(input);
		else
			charsToWrite = maxCharsToWrite - 1;

		huffmanEncodingTree->EncodeArray((unsigned char*)input, charsToWrite, &encodedBitStream);

		stringBitLength = (UInt32)encodedBitStream.GetWrittenBitsCount();

		output->WriteMini(stringBitLength);

		output->WriteBits(encodedBitStream.Data(), stringBitLength);
	}
	bool JackieStringCompressor::DecodeString(char *output, int maxCharsToWrite, JackieBits *input, UInt8 languageId)
	{
		HuffmanEncodingTree *huffmanEncodingTree;
		if (huffmanEncodingTrees.Has(languageId) == false)
			return false;
		if (maxCharsToWrite <= 0)
			return false;
		huffmanEncodingTree = huffmanEncodingTrees.Get(languageId);

		UInt32 stringBitLength = 0;
		int bytesInStream;
		output[0] = 0;

		input->ReadMini(stringBitLength);
		if (!stringBitLength) return false;

		if ((unsigned)input->GetPayLoadBits() < stringBitLength) return false;

		bytesInStream = huffmanEncodingTree->DecodeArray(input, stringBitLength, maxCharsToWrite, (unsigned char*)output);

		if (bytesInStream < maxCharsToWrite)
			output[bytesInStream] = 0;
		else
			output[maxCharsToWrite - 1] = 0;

		return true;
	}

	void JackieStringCompressor::AddReference(void)
	{
		if (++referenceCount == 1)
		{
			instance = JACKIE_INET::OP_NEW<JackieStringCompressor>(TRACE_FILE_AND_LINE_);
		}
	}

	void JackieStringCompressor::EncodeString(const JackieString *input, int maxCharsToWrite, JackieBits *output, UInt8 languageId)
	{
		EncodeString(input->C_String(), maxCharsToWrite, output, languageId);
	}
	bool JackieStringCompressor::DecodeString(JackieString *output, int maxCharsToWrite, JackieBits *input, UInt8 languageId)
	{
		if (maxCharsToWrite <= 0)
		{
			output->Clear();
			return true;
		}

		char *destinationBlock;
		bool out;

#if USE_STACK_ALLOCA==1
		if (maxCharsToWrite < MAX_ALLOC_STACK_COUNT)
		{
			destinationBlock = (char*)alloca(maxCharsToWrite);
			out = DecodeString(destinationBlock, maxCharsToWrite, input, languageId);
			*output = destinationBlock;
		}
		else
#endif
		{
			destinationBlock = (char*)jackieMalloc_Ex(maxCharsToWrite, TRACE_FILE_AND_LINE_);
			out = DecodeString(destinationBlock, maxCharsToWrite, input, languageId);
			*output = destinationBlock;
			jackieFree_Ex(destinationBlock, TRACE_FILE_AND_LINE_);
		}

		return out;
	}


#ifdef _CSTRING_COMPRESSOR
	void StringCompressor::EncodeString(const CString &input, int maxCharsToWrite, RakNet::JackieBits *output)
	{
		LPTSTR p = input;
		EncodeString(p, maxCharsToWrite*sizeof(TCHAR), output, languageID);
	}
	bool StringCompressor::DecodeString(CString &output, int maxCharsToWrite, RakNet::JackieBits *input, UInt8 languageId)
	{
		LPSTR p = output.GetBuffer(maxCharsToWrite*sizeof(TCHAR));
		DecodeString(p, maxCharsToWrite*sizeof(TCHAR), input, languageID);
		output.ReleaseBuffer(0)

	}
#endif

#ifdef _STD_STRING_COMPRESSOR
	void StringCompressor::EncodeString(const std::string &input, int maxCharsToWrite, RakNet::JackieBits *output, UInt8 languageId)
	{
		EncodeString(input.c_str(), maxCharsToWrite, output, languageId);
	}
	bool StringCompressor::DecodeString(std::string *output, int maxCharsToWrite, RakNet::JackieBits *input, UInt8 languageId)
	{
		if (maxCharsToWrite <= 0)
		{
			output->clear();
			return true;
		}

		char *destinationBlock;
		bool out;

#if USE_ALLOCA==1
		if (maxCharsToWrite < MAX_ALLOC_STACK_COUNT)
		{
			destinationBlock = (char*)alloca(maxCharsToWrite);
			out = DecodeString(destinationBlock, maxCharsToWrite, input, languageId);
			*output = destinationBlock;
		}
		else
#endif
		{
			destinationBlock = (char*)rakMalloc_Ex(maxCharsToWrite, _FILE_AND_LINE_);
			out = DecodeString(destinationBlock, maxCharsToWrite, input, languageId);
			*output = destinationBlock;
			rakFree_Ex(destinationBlock, _FILE_AND_LINE_);
		}

		return out;
	}
#endif


	/// JackieString
	SharedString JackieString::emptyString = { 0, (char*) "", (char*) "" };
	DataStructures::JackieArrayList<JackieString::StringPool*, 32> JackieString::freeList;

	JackieString::JackieString(UInt32 freeListIndex_)
	{
		freeListIndex = freeListIndex_;
		sharedString = &emptyString;
	}
	JackieString::JackieString(UInt32 freeListIndex_, char input)
	{
		freeListIndex = freeListIndex_;
		char str[2];
		str[0] = input;
		str[1] = 0;
		Assign(str);
	}
	JackieString::JackieString(UInt32 freeListIndex_, unsigned char input)
	{
		freeListIndex = freeListIndex_;
		char str[2];
		str[0] = (char)input;
		str[1] = 0;
		Assign(str);
	}
	JackieString::JackieString(UInt32 freeListIndex_, const unsigned char *format, ...)
	{
		freeListIndex = freeListIndex_;
		va_list ap;
		va_start(ap, format);
		Assign((const char*)format, ap);
		va_end(ap);
	}
	JackieString::JackieString(UInt32 freeListIndex_, const char *format, ...)
	{
		freeListIndex = freeListIndex_;
		va_list ap;
		va_start(ap, format);
		Assign(format, ap);
		va_end(ap);
	}
	JackieString::JackieString(const JackieString & rhs)
	{
		if (rhs.sharedString == &emptyString)
		{
			sharedString = &emptyString;
			return;
		}

		if (rhs.sharedString->refCount.GetValue() == 0)
		{
			sharedString = &emptyString;
		}
		else
		{
			rhs.sharedString->refCount.Increment();
			sharedString = rhs.sharedString;
			freeListIndex = rhs.freeListIndex;
		}
	}
	JackieString::~JackieString()
	{
		Free();
	}

	static JackieSimpleMutex& GetPoolMutex(void)
	{
		static JackieSimpleMutex poolMutex;
		return poolMutex;
	}
	void JackieString::LockMutex(void)
	{
		GetPoolMutex().Lock();
	}
	void JackieString::UnlockMutex(void)
	{
		GetPoolMutex().Unlock();
	}
	void JackieString::FreeMemory(void)
	{
		LockMutex();
		FreeMemoryNoMutex();
		UnlockMutex();
	}
	void JackieString::FreeMemoryNoMutex(void)
	{
		StringPool* strPool;
		SharedString *ss;
		for (unsigned int i = 0; i < freeList.Size(); i++)
		{
			strPool = freeList[i];
			assert(strPool->Size() == strPool->AllocationSize()
				&& "some shared string is still being used somewhere, you have memeoryleak. man, try to call freememory() at the end of program");
			for (unsigned ii = 0; ii < strPool->Size(); ii++)
			{
				ss = (*strPool)[ii];
				JACKIE_INET::OP_DELETE(ss, TRACE_FILE_AND_LINE_);
			}
			strPool->Clear();
			JACKIE_INET::OP_DELETE(strPool, TRACE_FILE_AND_LINE_);
		}
		freeList.Clear();
	}


	void JackieString::Clone(void)
	{
		assert(sharedString != &emptyString);
		if (sharedString == &emptyString)
		{
			return;
		}

		// Empty or solo then no point to cloning
		if (sharedString->refCount.GetValue() == 1)
		{
			return;
		}
		sharedString->refCount.Decrement();
		Assign(sharedString->c_str);
	}
	void JackieString::Clear()
	{
		Free();
	}
	void JackieString::Free()
	{
		if (sharedString == &emptyString) return;
		sharedString->refCount.Decrement();
		if (sharedString->refCount.GetValue() == 0)
		{
			static const size_t smallStringSize = 128 - sizeof(unsigned int) - sizeof(size_t) - sizeof(char*) * 2;
			if (sharedString->bytesUsed > smallStringSize)
				jackieFree_Ex(sharedString->bigString, TRACE_FILE_AND_LINE_);
			freeList[freeListIndex]->InsertAtLast(sharedString);
		}
		sharedString = &emptyString;
	}
	void JackieString::Allocate(size_t len)
	{
		if (freeList.Size() == 0)
		{
			LockMutex();
			if (freeList.Size() == 0)
			{
				StringPool* strPool;
				SharedString *ss;

				for (unsigned ii = 0; ii < 32; ii++)
				{
					strPool = JACKIE_INET::OP_NEW<StringPool>(TRACE_FILE_AND_LINE_);
					freeList.InsertAtLast(strPool);
					for (unsigned i = 0; i < 128; i++)
					{
						ss = JACKIE_INET::OP_NEW<SharedString>(TRACE_FILE_AND_LINE_);
						strPool->InsertAtLast(ss);
					}
				}
			}
			UnlockMutex();
		}

		sharedString = (*freeList[freeListIndex])[freeList[freeListIndex]->Size() - 1];
		freeList[freeListIndex]->RemoveAtIndexFast(freeList[freeListIndex]->Size() - 1);
		sharedString->refCount.Increment();

		const size_t smallStringSize = 128 - sizeof(unsigned int) - sizeof(size_t) - sizeof(char*) * 2;
		if (len <= smallStringSize)
		{
			sharedString->bytesUsed = smallStringSize;
			sharedString->c_str = sharedString->smallString;
		}
		else
		{
			sharedString->bytesUsed = len << 1;
			sharedString->bigString = (char*)jackieMalloc_Ex(sharedString->bytesUsed, TRACE_FILE_AND_LINE_);
			sharedString->c_str = sharedString->bigString;
		}
	}


	void JackieString::Assign(const char *str)
	{
		if (str == 0 || str[0] == 0)
		{
			sharedString = &emptyString;
			return;
		}
		size_t len = strlen(str) + 1;
		Allocate(len);
		memcpy(sharedString->c_str, str, len);
	}
	JackieString JackieString::Assign(const char *str, size_t pos, size_t n)
	{
		size_t incomingLen = strlen(str);

		Clone();

		if (str == 0 || str[0] == 0 || pos >= incomingLen)
		{
			sharedString = &emptyString;
			return (*this);
		}

		if (pos + n >= incomingLen)
		{
			n = incomingLen - pos;

		}
		const char * tmpStr = &(str[pos]);

		size_t len = n + 1;
		Allocate(len);
		memcpy(sharedString->c_str, tmpStr, len);
		sharedString->c_str[n] = 0;

		return (*this);
	}
	void JackieString::Assign(const char *str, va_list ap)
	{
		if (str == 0 || str[0] == 0)
		{
			sharedString = &emptyString;
			return;
		}

		char stackBuff[512];
		if (_vsnprintf(stackBuff, 512, str, ap) != -1
#ifndef _WIN32
			// Here Windows will return -1 if the string is too long; Linux just truncates the string.
			&& strlen(stackBuff) < 511
#endif
			)
			{
				Assign(stackBuff);
				return;
			}
		char *buff = 0, *newBuff;
		size_t buffSize = 8096;
		while (1)
		{
			newBuff = (char*)jackieRealloc_Ex(buff, buffSize, TRACE_FILE_AND_LINE_);
			if (newBuff == 0)
			{
				notifyOutOfMemory(TRACE_FILE_AND_LINE_);
				if (buff != 0)
				{
					Assign(buff);
					jackieFree_Ex(buff, TRACE_FILE_AND_LINE_);
				}
				else
				{
					Assign(stackBuff);
				}
				return;
			}
			buff = newBuff;
			if (_vsnprintf(buff, buffSize, str, ap) != -1)
			{
				Assign(buff);
				jackieFree_Ex(buff, TRACE_FILE_AND_LINE_);
				return;
			}
			buffSize *= 2;
		}
	}


	void JackieString::Write(JackieBits *bs) const
	{
		JackieString::Write(sharedString->c_str, bs);
	}
	void JackieString::Write(const char *str, JackieBits *bs)
	{
		unsigned short l = (unsigned short)strlen(str);
		bs->Write(l);
		bs->WriteAlignedBytes((const unsigned char*)str, (const unsigned int)l);
	}

	void JackieString::WriteMini(JackieBits *bs, UInt8 languageId,
		bool writeLanguageId) const
	{
		JackieString::WriteMini(C_String(), bs, languageId, writeLanguageId);
	}
	void JackieString::WriteMini(const char *str, JackieBits *bs, UInt8 languageId, bool writeLanguageId)
	{
		if (writeLanguageId)
			bs->WriteMini(languageId);
		JackieStringCompressor::Instance()->EncodeString(str, 0xFFFF, bs, languageId);
	}

	bool JackieString::Read(JackieBits *bs)
	{
		Clear();
		unsigned short l = 0;
		bs->Read(l);
		if (l > 0)
		{
			Allocate(((unsigned int)l) + 1);
			bs->ReadAlignedBytes((unsigned char*)sharedString->c_str, l);
			if (l > 0)
				sharedString->c_str[l] = 0;
			else
				Clear();
		}
		else
			bs->AlignReadPosBitsByteBoundary();

		return l > 0;
	}
	bool JackieString::Read(char *str, JackieBits *bs)
	{
		unsigned short l;
		bs->Read(l);
		if (l > 0) bs->ReadAlignedBytes((unsigned char*)str, l);

		if (l == 0) str[0] = 0;
		str[l] = 0;

		return l > 0;
	}
	bool JackieString::ReadMini(JackieBits *bs, bool readLanguageId)
	{
		UInt8 languageId;
		if (readLanguageId)
			bs->ReadMini(languageId);
		else
			languageId = 0;
		return JackieStringCompressor::Instance()->DecodeString(this, 0xFFFF, bs, languageId);
	}
	bool JackieString::ReadMini(char *str, JackieBits *bs, bool readLanguageId)
	{
		UInt8 languageId;
		if (readLanguageId)
			bs->ReadMini(languageId);
		else
			languageId = 0;
		return JackieStringCompressor::Instance()->DecodeString(str, 0xFFFF, bs, languageId);
	}

	/// @Caution you must seup correct freeListIndex before valling this function
	JackieString& JackieString::operator= (const char *str)
	{
		Free();
		Assign(str);
		return *this;
	}
	JackieString& JackieString::operator=(char *str)
	{
		return operator = ((const char*)str);
	}
	JackieString& JackieString::operator = (const unsigned char *str)
	{
		return operator = ((const char*)str);
	}
	JackieString& JackieString::operator = (char unsigned *str)
	{
		return operator = ((const char*)str);
	}
	JackieString& JackieString::operator = (const char c)
	{
		char buff[2];
		buff[0] = c;
		buff[1] = 0;
		return operator = ((const char*)buff);
	}
	JackieString& JackieString::operator = (const JackieString& rhs)
	{
		Free();
		if (rhs.sharedString == &emptyString)
			return *this;

		if (rhs.sharedString->refCount.GetValue() == 0)
		{
			sharedString = &emptyString;
		}
		else
		{
			sharedString = rhs.sharedString;
			sharedString->refCount.Increment();
			freeListIndex = rhs.freeListIndex;
		}
		return *this;
	}

	const char *JackieString::ToString(Int64 i)
	{
		static int index = 0;
		static char buff[64][64];
#if defined(_WIN32)
		sprintf(buff[index], "%I64d", i);
#else
		sprintf(buff[index], "%lld", (long long unsigned int) i);
#endif
		int lastIndex = index;
		if (++index == 64)
			index = 0;
		return buff[lastIndex];
	}
	const char *JackieString::ToString(UInt64 i)
	{
		static int index = 0;
		static char buff[64][64];
#if defined(_WIN32)
		sprintf(buff[index], "%I64u", i);
#else
		sprintf(buff[index], "%llu", (long long unsigned int) i);
#endif
		int lastIndex = index;
		if (++index == 64)
			index = 0;
		return buff[lastIndex];
	}

	void JackieString::Printf(void)
	{
		printf("%s", sharedString->c_str);
	}
	void JackieString::FPrintf(FILE *fp)
	{
		fprintf(fp, "%s", sharedString->c_str);
	}
}
