#ifndef OVERRIDE_MEMORY_H_
#define OVERRIDE_MEMORY_H_

#include "DLLExport.h"
#include "DefaultNetDefines.h"
#include <new>
#if defined(__FreeBSD__)
#include <stdlib.h>
#elif defined ( __APPLE__ ) || defined ( __APPLE_CC__ )
#include <malloc/malloc.h>
#include <alloca.h>
#elif defined(_WIN32)
#include <malloc.h>
#else
#include <malloc.h>
// Alloca needed on Ubuntu apparently
#include <alloca.h>
#endif


// These pointers are statically and globally defined in RakMemoryOverride.cpp
// Change them to point to your own allocators if you want.
// Use the functions for a DLL, or just reassign the variable if using source
typedef void * ( *RakMalloc ) ( size_t size );
typedef void * ( *RakRealloc ) ( void *p, size_t size );
typedef void(*RakFree) ( void *p );
extern JACKIE_EXPORT RakMalloc rakMalloc;
extern JACKIE_EXPORT RakRealloc rakRealloc;
extern JACKIE_EXPORT RakFree rakFree;
void JACKIE_EXPORT SetMalloc(RakMalloc userFunction);
void JACKIE_EXPORT SetRealloc(RakRealloc userFunction);
void JACKIE_EXPORT SetFree(RakFree userFunction);
extern JACKIE_EXPORT RakMalloc GetMalloc();
extern JACKIE_EXPORT RakRealloc GetRealloc();
extern JACKIE_EXPORT RakFree GetFree();

typedef  void * ( *RakMalloc_Ex ) ( size_t size, const char *file, unsigned int line );
typedef  void * ( *RakRealloc_Ex ) ( void *p, size_t size, const char *file, unsigned int line );
typedef  void(*RakFree_Ex) ( void *p, const char *file, unsigned int line );
extern JACKIE_EXPORT RakMalloc_Ex rakMalloc_Ex;
extern JACKIE_EXPORT RakRealloc_Ex rakRealloc_Ex;
extern JACKIE_EXPORT RakFree_Ex rakFree_Ex;
void JACKIE_EXPORT SetMalloc_Ex(RakMalloc_Ex userFunction);
void JACKIE_EXPORT SetRealloc_Ex(RakRealloc_Ex userFunction);
void JACKIE_EXPORT SetFree_Ex(RakFree_Ex userFunction);
extern JACKIE_EXPORT RakMalloc_Ex GetMalloc_Ex();
extern JACKIE_EXPORT RakRealloc_Ex GetRealloc_Ex();
extern JACKIE_EXPORT RakFree_Ex GetFree_Ex();

typedef void(*NotifyOutOfMemory) ( const char *file, const long line );
typedef void* ( *DlMallocMMap ) ( size_t size );
typedef void* ( *DlMallocDirectMMap ) ( size_t size );
typedef int(*DlMallocMUnmap) ( void* ptr, size_t size );
extern JACKIE_EXPORT NotifyOutOfMemory notifyOutOfMemory;
extern JACKIE_EXPORT DlMallocMMap dlMallocMMap;
extern JACKIE_EXPORT DlMallocDirectMMap dlMallocDirectMMap;
extern JACKIE_EXPORT DlMallocMUnmap dlMallocMUnmap;
void JACKIE_EXPORT SetNotifyOutOfMemory(NotifyOutOfMemory userFunction);
void JACKIE_EXPORT SetDLMallocMMap(DlMallocMMap userFunction);
void JACKIE_EXPORT SetDLMallocDirectMMap(DlMallocDirectMMap userFunction);
void JACKIE_EXPORT SetDLMallocMUnmap(DlMallocMUnmap userFunction);
extern JACKIE_EXPORT DlMallocMMap GetDLMallocMMap();
extern JACKIE_EXPORT DlMallocDirectMMap GetDLMallocDirectMMap();
extern JACKIE_EXPORT DlMallocMUnmap GetDLMallocMUnmap();

//extern JACKIE_EXPORT void * ( *rakMalloc ) ( size_t size );
//extern JACKIE_EXPORT void * ( *rakRealloc ) ( void *p, size_t size );
//extern JACKIE_EXPORT void(*rakFree) ( void *p );
//
//extern JACKIE_EXPORT void * ( *rakMalloc_Ex ) ( size_t size, const char *file, unsigned int line );
//extern JACKIE_EXPORT void * ( *rakRealloc_Ex ) ( void *p, size_t size, const char *file, unsigned int line );
//extern JACKIE_EXPORT void(*rakFree_Ex) ( void *p, const char *file, unsigned int line );

//extern JACKIE_EXPORT void(*notifyOutOfMemory) ( const char *file, const long line );
//extern JACKIE_EXPORT void * ( *dlMallocMMap ) ( size_t size );
//extern JACKIE_EXPORT void * ( *dlMallocDirectMMap ) ( size_t size );
//extern JACKIsE_EXPORT int(*dlMallocMUnmap) ( void* ptr, size_t size );

//// Change to a user defined allocation function
//inline void JACKIE_EXPORT SetMalloc(void* ( *userFunction )( size_t size ));
//inline void JACKIE_EXPORT SetRealloc(void* ( *userFunction )( void *p, size_t size ));
//inline void JACKIE_EXPORT SetFree(void(*userFunction)( void *p ));
//inline void JACKIE_EXPORT SetMalloc_Ex(void* ( *userFunction )( size_t size, const char *file, unsigned int line ));
//inline void JACKIE_EXPORT SetRealloc_Ex(void* ( *userFunction )( void *p, size_t size, const char *file, unsigned int line ));
//inline void JACKIE_EXPORT SetFree_Ex(void(*userFunction)( void *p, const char *file, unsigned int line ));
//
//// Change to a user defined out of memory function
//void JACKIE_EXPORT SetNotifyOutOfMemory(void(*userFunction)( const char *file, const long line ));
//void JACKIE_EXPORT SetDLMallocMMap(void* ( *userFunction )( size_t size ));
//void JACKIE_EXPORT SetDLMallocDirectMMap(void* ( *userFunction )( size_t size ));
//void JACKIE_EXPORT SetDLMallocMUnmap(int(*userFunction)( void* ptr, size_t size ));
//extern JACKIE_EXPORT void * ( *GetMalloc() ) ( size_t size );
//extern JACKIE_EXPORT void * ( *GetRealloc() ) ( void *p, size_t size );
//extern JACKIE_EXPORT void(*GetFree()) ( void *p );
//
//extern JACKIE_EXPORT void * ( *GetMalloc_Ex() ) ( size_t size, const char *file, unsigned int line );
//extern JACKIE_EXPORT void * ( *GetRealloc_Ex() ) ( void *p, size_t size,
//	const char *file, unsigned int line );
//extern JACKIE_EXPORT void(*GetFree_Ex()) ( void *p, const char *file, unsigned int line );
//
//extern JACKIE_EXPORT void *( *GetDLMallocMMap() )( size_t size );
//extern JACKIE_EXPORT void *( *GetDLMallocDirectMMap() )( size_t size );
//extern JACKIE_EXPORT int(*GetDLMallocMUnmap())( void* ptr, size_t size );

namespace JACKIE_INET
{
	/// new functions with different number of ctor params, up to 4
	template <class Type>
	JACKIE_EXPORT Type* OP_NEW(const char *file, unsigned int line)
	{
#if USE_MEM_OVERRIDE==1
		char *buffer = (char *) ( GetMalloc_Ex() )( sizeof(Type), file, line );
		Type *t = new (buffer) Type;
		return t;
#else
		(void) file;
		(void) line;
		return new Type;
#endif
	}
	template <class Type, class P1>
	JACKIE_EXPORT Type* OP_NEW_1(const char *file, unsigned int line,
		const P1 &p1)
	{
#if USE_MEM_OVERRIDE==1
		char *buffer = (char *) ( GetMalloc_Ex() )( sizeof(Type), file, line );
		Type *t = new (buffer) Type(p1);
		return t;
#else
		(void) file;
		(void) line;
		return new Type(p1);
#endif
	}
	template <class Type, class P1, class P2>
	JACKIE_EXPORT Type* OP_NEW_2(const char *file, unsigned int line,
		const P1 &p1, const P2 &p2)
	{
#if USE_MEM_OVERRIDE==1
		char *buffer = (char *) ( GetMalloc_Ex() )( sizeof(Type), file, line );
		Type *t = new (buffer) Type(p1, p2);
		return t;
#else
		(void) file;
		(void) line;
		return new Type(p1, p2);
#endif
	}
	template <class Type, class P1, class P2, class P3>
	JACKIE_EXPORT Type* OP_NEW_3(const char *file, unsigned int line,
		const P1 &p1, const P2 &p2, const P3 &p3)
	{
#if USE_MEM_OVERRIDE==1
		char *buffer = (char *) ( GetMalloc_Ex() )( sizeof(Type), file, line );
		Type *t = new (buffer) Type(p1, p2, p3);
		return t;
#else
		(void) file;
		(void) line;
		return new Type(p1, p2, p3);
#endif
	}
	template <class Type, class P1, class P2, class P3, class P4>
	JACKIE_EXPORT Type* OP_NEW_4(const char *file, unsigned int line,
		const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4)
	{
#if USE_MEM_OVERRIDE==1
		char *buffer = (char *) ( GetMalloc_Ex() )( sizeof(Type), file, line );
		Type *t = new (buffer) Type(p1, p2, p3, p4);
		return t;
#else
		(void) file;
		(void) line;
		return new Type(p1, p2, p3, p4);
#endif
	}


	template <class Type>
	JACKIE_EXPORT Type* OP_NEW_ARRAY(const int count, const char *file, unsigned int line)
	{
		if( count == 0 )
			return 0;

#if USE_MEM_OVERRIDE==1
		//		Type *t;
		char *buffer = (char *) ( GetMalloc_Ex() )( sizeof(int) + sizeof(Type)*count, file, line );
		( (int*) buffer )[0] = count;
		for( int i = 0; i<count; i++ )
		{
			//t = 
			new( buffer + sizeof(int) + i*sizeof(Type) ) Type;
		}
		return (Type *) ( buffer + sizeof(int) );
#else
		(void) file;
		(void) line;
		return new Type[count];
#endif

	}

	template <class Type>
	JACKIE_EXPORT void OP_DELETE(Type *buff, const char *file, unsigned int line)
	{
#if USE_MEM_OVERRIDE==1
		if( buff == 0 ) return;
		buff->~Type();
		( GetFree_Ex() )( (char*) buff, file, line );
#else
		(void) file;
		(void) line;
		delete buff;
#endif

	}

	template <class Type>
	JACKIE_EXPORT void OP_DELETE_ARRAY(Type *buff, const char *file, unsigned int line)
	{
#if USE_MEM_OVERRIDE==1
		if( buff == 0 )
			return;

		int count = ( (int*) ( (char*) buff - sizeof(int) ) )[0];
		Type *t;
		for( int i = 0; i<count; i++ )
		{
			t = buff + i;
			t->~Type();
		}
		( GetFree_Ex() )( (char*) buff - sizeof(int), file, line );
#else
		(void) file;
		(void) line;
		delete[ ] buff;
#endif

	}

	void JACKIE_EXPORT * _RakMalloc(size_t size);
	void JACKIE_EXPORT * _RakRealloc(void *p, size_t size);
	void JACKIE_EXPORT _RakFree(void *p);
	void JACKIE_EXPORT * _RakMalloc_Ex(size_t size, const char *file, unsigned int line);
	void JACKIE_EXPORT * _RakRealloc_Ex(void *p, size_t size, const char *file, unsigned int line);
	void JACKIE_EXPORT _RakFree_Ex(void *p, const char *file, unsigned int line);
	void JACKIE_EXPORT * _DLMallocMMap(size_t size);
	void JACKIE_EXPORT * _DLMallocDirectMMap(size_t size);
	int JACKIE_EXPORT _DLMallocMUnmap(void *p, size_t size);

}

// Call to make JACKIE_INET allocate a large block of memory, 
// and do all subsequent allocations in that memory block
// Initial and reallocations will be done through whatever function is pointed to by 
// yourMMapFunction, and yourDirectMMapFunction (default is malloc)
// Allocations will be freed through whatever function is pointed to by yourMUnmapFunction
// (default free)
void UseRaknetFixedHeap(size_t initialCapacity,
	void * ( *yourMMapFunction ) ( size_t size ) = JACKIE_INET::_DLMallocMMap,
	void * ( *yourDirectMMapFunction ) ( size_t size ) = JACKIE_INET::_DLMallocDirectMMap,
	int(*yourMUnmapFunction) ( void *p, size_t size ) = JACKIE_INET::_DLMallocMUnmap);

// Free memory allocated from UseRaknetFixedHeap
void FreeRakNetFixedHeap(void);

#endif