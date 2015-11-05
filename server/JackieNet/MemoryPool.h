#ifndef MEMORY_POOL_H_
#define MEMORY_POOL_H_

#include <cassert>
#ifndef __APPLE__
// Use stdlib and not malloc for compatibility
#include <stdlib.h>
#endif
#include "DLLExport.h"
#include "OverrideMemory.h"
#include "BasicTypes.h"

///====================================================
/// Very fast memory pool for allocating and deallocating structures that don't have 
/// constructors or destructors (in other words, there no pointer type of 
/// variables that nneed to be deleted). 
///=======================================================
namespace DataStructures
{
	template <typename Type,
		UInt32 CELLS_SIZE_PER_PAGE = 256,
		/* if all cells in an usable page are reclaimed,
		 * MAX_FREE_PAGES determins whether to free this page or not */
		 UInt32 MAX_FREE_PAGES = 4>
	class JACKIE_EXPORT MemoryPool
	{
		public:
		struct Cell;
		struct Page
		{
			Cell** freeCells; /// array that stores all the @Cell pointers
			int freeCellsSize; /// the size of un-allocated cells
			Cell* cell; /// array that stores @Cell itself
			Page *next;
			Page *prev;
		};
		struct Cell
		{
			Type blockType;
			Page *parentPage;
		};

		private:
		/// @Brief Contains pages which have free cell to allocate
		Page *usablePage;
		/// @Brief contains pages which have no free cell to allocate
		Page *unUsablePage;
		UInt32 mUsablePagesSize;
		UInt32 mUnUsablePagesSize;
		UInt32 mPerPageSizeByte;
		UInt32 mCellSizePerPage;


		///========================================
		/// @Function InitPage 
		/// @Brief
		/// @Access  private  
		/// @Param [in] [Page * page]  
		/// @Param [in] [Page * prev]  
		/// @Returns [bool]
		/// @Remarks
		/// @Notice
		/// @Author mengdi[Jackie]
		///========================================
		bool InitPage(Page *page, Page *prev)
		{
			UInt32 i = 0;

			///  allocate @Cell Array
			if( ( page->cell = (Cell*) jackieMalloc_Ex(mPerPageSizeByte, TRACE_FILE_AND_LINE_) )
				== 0 ) return false;

			/// allocate @Cell Pointers Array
			if( ( page->freeCells = (Cell**) jackieMalloc_Ex(sizeof(Cell*)*mCellSizePerPage, TRACE_FILE_AND_LINE_) ) == 0 )
			{
				jackieFree_Ex(page->cell, TRACE_FILE_AND_LINE_);
				return false;
			}

			/// @freeCells stores all the pointers to the @Cell
			Cell *currCell = page->cell;
			Cell **currFreeCells = page->freeCells;
			while( i < mCellSizePerPage )
			{
				currCell->parentPage = page;
				currFreeCells[i] = currCell++;
				i++;
			}
			page->freeCellsSize = mCellSizePerPage;
			page->next = usablePage;
			page->prev = prev;
			return true;
		}

		public:
		MemoryPool()
		{
#if _DISABLE_MEMORY_POOL == 0
			mUsablePagesSize = mUnUsablePagesSize = 0;
			mPerPageSizeByte = CELLS_SIZE_PER_PAGE* sizeof(Cell);
			mCellSizePerPage = CELLS_SIZE_PER_PAGE;
			usablePage = unUsablePage = 0;
#endif
		}
		~MemoryPool() { Clear(); }

		Type* Allocate(void)
		{
#if _DISABLE_MEMORY_POOL != 0
			return(Type*) jackieMalloc_Ex(sizeof(Type), TRACE_FILE_AND_LINE_);
#endif

			if( mUsablePagesSize > 0 )
			{
				Page *currentPage = usablePage;
				Type *retValue = (Type*) currentPage->freeCells[--( currentPage->freeCellsSize )];
				if( currentPage->freeCellsSize == 0 )
				{
					--mUsablePagesSize;
					usablePage = currentPage->next;
					assert(mUsablePagesSize == 0 || usablePage->freeCellsSize > 0);
					currentPage->next->prev = currentPage->prev;
					currentPage->prev->next = currentPage->next;
					if( mUnUsablePagesSize++ == 0 )
					{
						unUsablePage = currentPage;
						currentPage->next = currentPage;
						currentPage->prev = currentPage;
					} else
					{
						currentPage->next = unUsablePage;
						currentPage->prev = unUsablePage->prev;
						unUsablePage->prev->next = currentPage;
						unUsablePage->prev = currentPage;
					}
				}
				assert(mUsablePagesSize == 0 || usablePage->freeCellsSize > 0);
				return retValue;
			}

			if( ( usablePage = (Page *) jackieMalloc_Ex(sizeof(Page), TRACE_FILE_AND_LINE_) ) == 0 ) return 0;
			mUsablePagesSize = 1;
			if( !InitPage(usablePage, usablePage) ) return 0;
			assert(usablePage->freeCellsSize > 1);
			return (Type *) usablePage->freeCells[--usablePage->freeCellsSize];
		}
		void Reclaim(Type *m)
		{
#if _DISABLE_MEMORY_POOL != 0
			jackieFree_Ex(m, TRACE_FILE_AND_LINE_);
			return;
#endif
			/// find the page where @m is in and return it
			Cell *currCell = (Cell*) m;
			Page *currPage = currCell->parentPage;

			/// this is an unavaiable page
			if( currPage->freeCellsSize == 0 )
			{
				/// firstly reclaim @currCell to currentPage
				currPage->freeCells[currPage->freeCellsSize++] = currCell;

				// then remove @currentPage from unavailable page list
				currPage->next->prev = currPage->prev;
				currPage->prev->next = currPage->next;
				mUnUsablePagesSize--;

				/// then update the @unavailablePage  
				if( mUnUsablePagesSize > 0 && currPage == unUsablePage )
				{
					unUsablePage = unUsablePage->next;
				}

				/// then insert currentPage to the head of available page list
				if( mUsablePagesSize++ == 0 )
				{
					usablePage = currPage;
					currPage->next = currPage;
					currPage->prev = currPage;
				} else
				{
					currPage->next = usablePage;
					currPage->prev = usablePage->prev;
					usablePage->prev->next = currPage;
					usablePage->prev = currPage;
				}

			} else 	/// this is an avaiable page
			{
				// first reclaim @m to currentPage
				currPage->freeCells[currPage->freeCellsSize++] = currCell;

				// all cells in @currentPage are reclaimed and becomes empty
				if( currPage->freeCellsSize == mCellSizePerPage &&
					mUsablePagesSize > MAX_FREE_PAGES )
				{
					/// After a certain point, just deallocate empty pages
					/// rather than keep them around
					if( currPage == usablePage )
					{
						usablePage = currPage->next;
						assert(usablePage->freeCellsSize > 0);
					}
					currPage->prev->next = currPage->next;
					currPage->next->prev = currPage->prev;
					mUsablePagesSize--;
					jackieFree_Ex(currPage->freeCells, TRACE_FILE_AND_LINE_);
					jackieFree_Ex(currPage->cell, TRACE_FILE_AND_LINE_);
					jackieFree_Ex(currPage, TRACE_FILE_AND_LINE_);
				}
			}
		}
		void Clear(void)
		{
#if _DISABLE_MEMORY_POOL != 0
			return;
#endif
			Page *currentPage, *freedPage;

			if( mUsablePagesSize > 0 )
			{
				currentPage = usablePage;
				while( true )
				{
					jackieFree_Ex(currentPage->freeCells, TRACE_FILE_AND_LINE_);
					jackieFree_Ex(currentPage->cell, TRACE_FILE_AND_LINE_);
					freedPage = currentPage;
					currentPage = currentPage->next;
					if( currentPage == usablePage )
					{
						jackieFree_Ex(freedPage, TRACE_FILE_AND_LINE_);
						mUsablePagesSize = 0;
						break;
					}
					jackieFree_Ex(freedPage, TRACE_FILE_AND_LINE_);
				}
			}

			if( mUnUsablePagesSize > 0 )
			{
				currentPage = unUsablePage;
				while( true )
				{
					jackieFree_Ex(currentPage->freeCells, TRACE_FILE_AND_LINE_);
					jackieFree_Ex(currentPage->cell, TRACE_FILE_AND_LINE_);
					freedPage = currentPage;
					currentPage = currentPage->next;
					if( currentPage == unUsablePage )
					{
						jackieFree_Ex(freedPage, TRACE_FILE_AND_LINE_);
						mUnUsablePagesSize = 0;
						break;
					}
					jackieFree_Ex(freedPage, TRACE_FILE_AND_LINE_);
				}
			}
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