#include "SharedMemory.h"
#include <exception>

SharedMemory::SharedMemory()
{
	mPtr = NULL;
	mFile = 0;
	mSize = 0;
}

SharedMemory::SharedMemory(const wchar_t *_nm, size_t _sz, bool _open)
{
	mPtr = NULL;
	mFile = 0;
	mSize = 0;
	open(_nm, _sz, _open);
}

SharedMemory::~SharedMemory()
{
	close();
}

bool SharedMemory::open(const wchar_t *_nm, size_t _sz, bool _open)
{
	if(good())
		close();

	mSize = 0;
	mPtr = NULL;

	if(_open)
	{
		mFile = OpenFileMappingW(FILE_MAP_WRITE,
			FALSE,
			_nm);
	}
	else
	{
		mFile = CreateFileMappingW(INVALID_HANDLE_VALUE,
			NULL,
			PAGE_READWRITE,
			0,
			_sz,
			_nm);
	}

	if(!mFile)
	{
		DWORD err = GetLastError();
		return false;
	}

	mPtr = MapViewOfFile(mFile, FILE_MAP_WRITE, 0, 0, _sz);
	if(!mPtr)
	{
		CloseHandle(mFile);
		mFile = 0;
		return false;
	}

	mSize = _sz;

	return true;
}

void SharedMemory::close()
{
	if(!good())
		return;

	mSize = 0;

	if(mPtr)
	{
		UnmapViewOfFile(mPtr);
		mPtr = NULL;
	}

	if(mFile)
	{
		CloseHandle(mFile);
		mFile = 0;
	}
}

size_t SharedMemory::read(void *_ptr, size_t _off, size_t _sz)
{
	if(!mPtr)
		return NULL;

	size_t sz = min(_sz, mSize-_off);
	void *ptr = ((char*)mPtr) + _off;
	if(!memcpy(_ptr, ptr, sz))
		return 0;

	return sz;
}

size_t SharedMemory::write(void *_ptr, size_t _off, size_t _sz)
{
	if(!mPtr)
		return NULL;

	size_t sz = min(_sz, mSize-_off);
	void *ptr = ((char*)mPtr) + _off;
	if(!memcpy(ptr, _ptr, sz))
		return 0;

	return sz;
}

size_t SharedMemory::fill(int val, size_t _off, size_t _sz)
{
	if(!mPtr)
		return NULL;

	size_t sz = min(_sz, mSize-_off);
	void *ptr = ((char*)mPtr) + _off;
	if(!memset(ptr, val, sz))
		return 0;

	return sz;
}

void *SharedMemory::ptr(size_t _off)
{
	if(!mPtr)
		return NULL;

	return ((char*)mPtr + _off);
}
