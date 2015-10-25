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
		UInt32 availablePagesSize, unavailablePagesSize;
		UInt32 memoryPoolPageSize;

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
#endif
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

		void SetPageSize(UInt32 size) { memoryPoolPageSize = size; }
		bool InitPage(Page *page, Page *prev)
		{
			int i = 0;
			const int blocksCountPerPage = memoryPoolPageSize / sizeof(MemoryWithPage);

			if( ( page->block = (MemoryWithPage*) rakMalloc_Ex(memoryPoolPageSize, TRACE_FILE_AND_LINE_) )
				== 0 ) return false;

			if( ( page->availableStack = (MemoryWithPage**) rakMalloc_Ex(sizeof(MemoryWithPage*)*blocksCountPerPage, TRACE_FILE_AND_LINE_) ) == 0 )
			{
				rakFree_Ex(page->block, TRACE_FILE_AND_LINE_);
				return false;
			}

			MemoryWithPage *currentBlock = page->block;
			MemoryWithPage **currentStack = page->availableStack;
			while( i < blocksCountPerPage )
			{
				currentBlock->parentPage = page;
				currentStack[i] = currentBlock++;
				i++;
			}
			page->availableStackSize = blocksCountPerPage;
			page->next = availablePages;
			page->prev = pre;
			return true;
		}

		MemoryBlockType* Allocate(void)
		{
#if _DISABLE_MEMORY_POOL != 0
			return(MemoryBlockType*) rakMalloc_Ex(sizeof(MemoryBlockType), TRACE_FILE_AND_LINE_);
#endif

		}
	};
}
#endif