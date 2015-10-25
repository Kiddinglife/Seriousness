#ifndef MEMORY_POOL_H_
#define MEMORY_POOL_H_

#include <cassert>
#ifndef __APPLE__
// Use stdlib and not malloc for compatibility
#include <stdlib.h>
#endif
#include "DLLExport.h"
#include "OverrideMemory.h"

#define DS_MEMORY_POOL_MAX_FREE_PAGES 4

namespace DataStructures
{
	//////////////////////////////////////////////////////////////////////////
	/// Very fast memory pool for allocating and deallocating structures that don't have 
	/// constructors or destructors.Contains a list of pages, each of which has an array of 
	/// the user structures
	//////////////////////////////////////////////////////////////////////////
	template <class MemoryBlockType>
	class JACKIE_EXPORT MemoryPool
	{
		public:

		struct MemoryWithPage
		{
			MemoryBlockType userMemory;
			Page *parentPage;
		};
		struct Page
		{
			MemoryWithPage** availableStack;
			int availableStackSize;
			MemoryWithPage* block;
			Page *next, *prev;
		};

		//////////////////////////////////////////////////////////////////////////
		// availablePages contains pages which have room to give the user new blocks.  
		/// We return these blocks from the head of the list
		/// unavailablePages are pages which are totally full, and from which we do not
		/// return new blocks.
		// Pages move from the head of unavailablePages to the tail of availablePages,
		/// and from the head of availablePages to the tail of unavailablePages
		//////////////////////////////////////////////////////////////////////////
		Page *availablePages, *unavailablePages;
		int availablePagesSize, unavailablePagesSize;
		int memoryPoolPageSize;

		MemoryPool()
		{
#if _DISABLE_MEMORY_POOL == 0
			availablePagesSize = unavailablePagesSize = 0;
			memoryPoolPageSize = 16384;
#endif
		}
		~MemoryPool()
		{
#if _DISABLE_MEMORY_POOL != 0
			return;
#else
			Page *currentPage, *freedPage;

			if( availablePagesSize > 0 )
			{
				currentPage = availablePages;
				while( true )
				{
					rakFree_Ex(currentPage->availableStack, TRACE_FILE_AND_LINE_);
					rakFree_Ex(currentPage->block, TRACE_FILE_AND_LINE_);
					freedPage = currentPage;
					rakFree_Ex(freedPage, TRACE_FILE_AND_LINE_);
					if( ( currentPage = currentPage->next ) == availablePages )
					{
						availablePagesSize = 0;
						break;
					}
				}
			}

			if( unavailablePagesSize > 0 )
			{
				currentPage = unavailablePages;
				while( true )
				{
					rakFree_Ex(currentPage->availableStack, TRACE_FILE_AND_LINE_);
					rakFree_Ex(currentPage->block, TRACE_FILE_AND_LINE_);
					freedPage = currentPage;
					rakFree_Ex(freedPage, TRACE_FILE_AND_LINE_);
					if( ( currentPage = currentPage->next ) == availablePages )
					{
						unavailablePagesSize = 0;
						break;
					}
				}
			}
		}
#endif
	};
}
#endif