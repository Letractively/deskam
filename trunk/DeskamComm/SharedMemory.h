#pragma once

#include <Windows.h>

class SharedMemory
{
public:	
	SharedMemory();
	SharedMemory(const wchar_t *_name, size_t _sz, bool _open);
	virtual ~SharedMemory();

	bool open(const wchar_t *_name, size_t _sz, bool _open);
	void close();

	size_t read(void *_ptr, size_t _pos, size_t _sz);
	size_t write(void *_ptr, size_t _pos, size_t _sz);
	size_t fill(int val, size_t _off, size_t _sz);
	void *ptr(size_t _off=0);

	size_t size() const { return mSize; }
	bool good() const { return mPtr != NULL; }
	
	virtual bool operator!() const { return !good(); }
	virtual bool operator*() const { return good(); }

protected:
	size_t mSize;
	HANDLE mFile;
	void *mPtr;
};
