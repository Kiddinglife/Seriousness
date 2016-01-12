#include <cstring>
#include <cstdlib>

#include "JackieString.h"
#include "OverrideMemory.h"
#include "JackieBits.h"
#include "JackieSimpleMutex.h"
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

/// HuffmanEncodingTree implementations
namespace JACKIE_INET
{
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
		DataStructures::LinkedList<HuffmanEncodingTreeNode *> huffmanEncodingTreeNodeList;

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
				if (tempPath[tempPathLength])   // Write 1's and 0's because writing a bool will write the BitStream TYPE_CHECKING validation bits if that is defined along with the actual data bit, which is not what we want
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

	// Pass an array of bytes to array and a preallocated BitStream to receive the output
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

	// Pass an array of encoded bytes to array and a preallocated BitStream to receive the output
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
}

/// JackieStringCompressor
namespace JACKIE_INET
{
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
	void StringCompressor::EncodeString(const CString &input, int maxCharsToWrite, RakNet::BitStream *output)
	{
		LPTSTR p = input;
		EncodeString(p, maxCharsToWrite*sizeof(TCHAR), output, languageID);
	}
	bool StringCompressor::DecodeString(CString &output, int maxCharsToWrite, RakNet::BitStream *input, UInt8 languageId)
	{
		LPSTR p = output.GetBuffer(maxCharsToWrite*sizeof(TCHAR));
		DecodeString(p, maxCharsToWrite*sizeof(TCHAR), input, languageID);
		output.ReleaseBuffer(0)

	}
#endif

#ifdef _STD_STRING_COMPRESSOR
	void StringCompressor::EncodeString(const std::string &input, int maxCharsToWrite, RakNet::BitStream *output, UInt8 languageId)
	{
		EncodeString(input.c_str(), maxCharsToWrite, output, languageId);
	}
	bool StringCompressor::DecodeString(std::string *output, int maxCharsToWrite, RakNet::BitStream *input, UInt8 languageId)
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
}

/// JackieString
namespace JACKIE_INET
{
	JackieString::JackieString()
	{
	}

	void JackieString::Clear()
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	JackieString::~JackieString()
	{
	}
}
