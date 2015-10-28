#ifndef MEMORY_POOL_H_
#define MEMORY_POOL_H_

#include <cassert>
#ifndef __APPLE__
// Use stdlib and not malloc for compatibility
#include <stdlib.h>
#endif
#include "DLLExport.h"
#include "OverrideMemory.h"

//////////////////////////////////////////////////////////////////////////
/// Very fast memory pool for allocating and deallocating structures that don't have 
/// constructors or destructors (in other words, there no pointer type of 
/// variables).Contains a list of pages, each of which has an array of 
/// the user structures
//////////////////////////////////////////////////////////////////////////
namespace DataStructures
{
	template <typename MemoryBlockType,
		UInt32 BLOCKS_COUNT_PER_PAGE = 256,
		UInt32 DS_MEMORY_POOL_MAX_FREE_PAGES = 4>
	class JACKIE_EXPORT MemoryPool
	{
		public:
		struct MemoryWithPage;
		struct Page
		{
			MemoryWithPage** availableStack;
			int availableStackSize;
			MemoryWithPage* block;
			Page *next;
			Page *prev;
		};

		struct MemoryWithPage
		{
			MemoryBlockType userMemory;
			Page *parentPage;
		};

		private:
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

		bool InitPage(Page *page, Page *prev)
		{
			UInt32 i = 0;

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
			page->prev = prev;
			return true;
		}

		public:
		MemoryPool()
		{
#if _DISABLE_MEMORY_POOL == 0
			availablePagesSize = unavailablePagesSize = 0;
			memoryPoolPageSize = BLOCKS_COUNT_PER_PAGE* sizeof(MemoryWithPage);
			blocksCountPerPage = BLOCKS_COUNT_PER_PAGE;
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
					currentPage = currentPage->next;
					if( currentPage == availablePage )
					{
						rakFree_Ex(freedPage, TRACE_FILE_AND_LINE_);
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
					currentPage = currentPage->next;
					if( currentPage == availablePage )
					{
						rakFree_Ex(freedPage, TRACE_FILE_AND_LINE_);
						unavailablePagesSize = 0;
						break;
					}
				}
			}
		}
		MemoryBlockType* Allocate(void)
		{
#if _DISABLE_MEMORY_POOL != 0
			return(MemoryBlockType*) rakMalloc_Ex(sizeof(MemoryBlockType), TRACE_FILE_AND_LINE_);
#endif
			if( availablePagesSize > 0 )
			{
				Page *currentPage = availablePage;
				MemoryBlockType *retValue = (MemoryBlockType*) currentPage->availableStack[--( currentPage->availableStackSize )];
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
			if( ( availablePage = (Page *) rakMalloc_Ex(sizeof(Page), TRACE_FILE_AND_LINE_) ) == 0 ) return 0;
			availablePagesSize = 1;
			if( !InitPage(availablePage, availablePage) ) return 0;
			assert(availablePage->availableStackSize > 1);
			return (MemoryBlockType *) availablePage->availableStack[--availablePage->availableStackSize];
		}
		void Reclaim(MemoryBlockType *m)
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
				if( unavailablePagesSize > 0 && currentPage == unavailablePage )
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
		void Clear(const char *file, unsigned int line)
		{
#if  _DISABLE_MEMORY_POOL != 0
			return;
#endif
			Page *cur, *freed;

			if( availablePagesSize > 0 )
			{
				cur = availablePage;
				while( true )
				{
					rakFree_Ex(cur->availableStack, file, line);
					rakFree_Ex(cur->block, file, line);
					freed = cur;
					cur = cur->next;
					if( cur == availablePage )
					{
						rakFree_Ex(freed, file, line);
						break;
					}
					rakFree_Ex(freed, file, line);
				}
			}

			if( unavailablePagesSize > 0 )
			{
				cur = unavailablePage;
				while( 1 )
				{
					rakFree_Ex(cur->availableStack, file, line);
					rakFree_Ex(cur->block, file, line);
					freed = cur;
					cur = cur->next;
					if( cur == unavailablePage )
					{
						rakFree_Ex(freed, file, line);
						break;
					}
					rakFree_Ex(freed, file, line);
				}
			}
			availablePagesSize = 0;
			unavailablePagesSize = 0;
		}
	};
}
#endif
/*
#include "DS_MemoryPool.h"
#include "DS_List.h"

struct TestMemoryPool
{
int allocationId;
};

int main(void)
{
DataStructures::MemoryPool<TestMemoryPool> memoryPool;
DataStructures::List<TestMemoryPool*> returnList;

for (int i=0; i < 100000; i++)
returnList.Push(memoryPool.Allocate(_FILE_AND_LINE_), _FILE_AND_LINE_);
for (int i=0; i < returnList.Size(); i+=2)
{
memoryPool.Release(returnList[i], _FILE_AND_LINE_);
returnList.RemoveAtIndexFast(i);
}
for (int i=0; i < 100000; i++)
returnList.Push(memoryPool.Allocate(_FILE_AND_LINE_), _FILE_AND_LINE_);
while (returnList.Size())
{
memoryPool.Release(returnList[returnList.Size()-1], _FILE_AND_LINE_);
returnList.RemoveAtIndex(returnList.Size()-1);
}
for (int i=0; i < 100000; i++)
returnList.Push(memoryPool.Allocate(_FILE_AND_LINE_), _FILE_AND_LINE_);
while (returnList.Size())
{
memoryPool.Release(returnList[returnList.Size()-1], _FILE_AND_LINE_);
returnList.RemoveAtIndex(returnList.Size()-1);
}
for (int i=0; i < 100000; i++)
returnList.Push(memoryPool.Allocate(_FILE_AND_LINE_), _FILE_AND_LINE_);
for (int i=100000-1; i <= 0; i-=2)
{
memoryPool.Release(returnList[i], _FILE_AND_LINE_);
returnList.RemoveAtIndexFast(i);
}
for (int i=0; i < 100000; i++)
returnList.Push(memoryPool.Allocate(_FILE_AND_LINE_), _FILE_AND_LINE_);
while (returnList.Size())
{
memoryPool.Release(returnList[returnList.Size()-1], _FILE_AND_LINE_);
returnList.RemoveAtIndex(returnList.Size()-1);
}

return 0;
}
*/