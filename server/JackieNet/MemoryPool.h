#ifndef MEMORY_POOL_H_
#define MEMORY_POOL_H_

#include <cassert>
#ifndef __APPLE__
// Use stdlib and not malloc for compatibility
#include <stdlib.h>
#endif
#include "DLLExport.h"
#include "OverrideMemory.h"

namespace DataStructures
{
	const int DS_MEMORY_POOL_MAX_FREE_PAGES = 4;
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
		// availablePage contains pages which have room to give the user new blocks.  
		/// We return these blocks from the head of the list
		/// unavailablePage are pages which are totally full, and from which we do not
		/// return new blocks.
		// Pages move from the head of unavailablePage to the tail of availablePage,
		/// and from the head of availablePage to the tail of unavailablePage
		//////////////////////////////////////////////////////////////////////////
		Page *availablePage;
		Page *unavailablePage;
		UInt32 availablePagesSize;
		UInt32 unavailablePagesSize;
		UInt32 memoryPoolPageSize;
		UInt32 blocksCountPerPage;

		MemoryPool()
		{
#if _DISABLE_MEMORY_POOL == 0
			availablePagesSize = unavailablePagesSize = 0;
			memoryPoolPageSize = 16384;
			blocksCountPerPage = memoryPoolPageSize / sizeof(MemoryWithPage);
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
				currentPage = availablePage;
				while( true )
				{
					rakFree_Ex(currentPage->availableStack, TRACE_FILE_AND_LINE_);
					rakFree_Ex(currentPage->block, TRACE_FILE_AND_LINE_);
					freedPage = currentPage;
					rakFree_Ex(freedPage, TRACE_FILE_AND_LINE_);
					if( ( currentPage = currentPage->next ) == availablePage )
					{
						availablePagesSize = 0;
						break;
					}
				}
			}

			if( unavailablePagesSize > 0 )
			{
				currentPage = unavailablePage;
				while( true )
				{
					rakFree_Ex(currentPage->availableStack, TRACE_FILE_AND_LINE_);
					rakFree_Ex(currentPage->block, TRACE_FILE_AND_LINE_);
					freedPage = currentPage;
					rakFree_Ex(freedPage, TRACE_FILE_AND_LINE_);
					if( ( currentPage = currentPage->next ) == availablePage )
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
			page->next = availablePage;
			page->prev = pre;
			return true;
		}

		MemoryBlockType* Allocate(void)
		{
#if _DISABLE_MEMORY_POOL != 0
			return(MemoryBlockType*) rakMalloc_Ex(sizeof(MemoryBlockType), TRACE_FILE_AND_LINE_);
#endif
			if( availablePagesSize > 0 )
			{
				Page *currentPage = availablePage;
				MemoryWithPage *retValue = currentPage->availableStack[--( currentPage->availableStackSize )];
				if( currentPage->availableStackSize == 0 )
				{
					--availablePagesSize;
					availablePage = currentPage->next;
					assert(availablePagesSize == 0 || availablePage->availableStackSize > 0);
					currentPage->next->prev = currentPage->prev;
					currentPage->prev->next = currentPage->next;
					if( unavailablePagesSize++ == 0 )
					{
						unavailablePage = currentPage;
						currentPage->next = currentPage;
						currentPage->prev = currentPage;
					} else
					{
						currentPage->next = unavailablePage;
						currentPage->prev = unavailablePage->prev;
						unavailablePage->prev->next = currentPage;
						unavailablePage->prev = currentPage;
					}
				}
				assert(availablePagesSize == 0 || availablePage->availableStackSize > 0);
				return retValue;
			}
		}
		void MemoryPool<MemoryBlockType>::Release(MemoryBlockType *m)
		{
#if _DISABLE_MEMORY_POOL != 0
			rakFree_Ex(m, TRACE_FILE_AND_LINE_);
			return;
#endif
			/// find the page where this block is in and return it
			MemoryWithPage *memoryWithPage = (MemoryWithPage*) m;
			Page *currentPage = memoryWithPage->parentPage;

			if( currentPage->availableStackSize == 0 )
			{
				// reclaim m to currentPage
				currentPage->availableStack[currentPage->availableStackSize++] = memoryWithPage;

				// remove currentPage from unavailable page list
				currentPage->next->prev = currentPage->prev;
				currentPage->prev->next = currentPage->next;
				unavailablePagesSize--;

				// update the unavailablePage  
				if (unavailablePagesSize > 0 && currentPage == unavailablePage)
				{
					unavailablePage = unavailablePage->next;
				}

				// then insert currentPage to the head of available page list
				if( availablePagesSize++ == 0 )
				{
					availablePage = currentPage;
					currentPage->next = currentPage;
					currentPage->prev = currentPage;
				} else
				{
					currentPage->next = availablePage;
					currentPage->prev = availablePage->prev;
					availablePage->prev->next = currentPage;
					availablePage->prev = currentPage;
				}

			} else
			{
				// reclaim m to currentPage
				currentPage->availableStack[currentPage->availableStackSize++] = memoryWithPage;

				// all objects in currentPage are reclaimed
				if( currentPage->availableStackSize == blocksCountPerPage &&
					availablePagesSize >= DS_MEMORY_POOL_MAX_FREE_PAGES )
				{
					/// After a certain point, just deallocate empty pages
					/// rather than keep them around
					if( currentPage == availablePage )
					{
						availablePage = currentPage->next;
						assert(availablePage->availableStackSize > 0);
					}
					currentPage->prev->next = currentPage->next;
					currentPage->next->prev = currentPage->prev;
					availablePagesSize--;
					rakFree_Ex(currentPage->availableStack, TRACE_FILE_AND_LINE_);
					rakFree_Ex(currentPage->block, TRACE_FILE_AND_LINE_);
					rakFree_Ex(currentPage, TRACE_FILE_AND_LINE_);
				}
			}
		}
	};
}
#endif